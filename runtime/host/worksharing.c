/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Dept. of Computer Science & Engineering, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ort.c -- OMPi RunTime library, workshare regions support */

/*
 * 2010/11/20:
 *   fixed FOR bugs for the case of 1-thread teams
 *   fixed ORDERED bug
 * Nov. 2010:
 *   new FOR scheduling
 * Version 1.0.1j:
 *   first time around, out of ort.c code.
 */

#include "ort_prive.h"
#include <stdlib.h>
#include <stdarg.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * WORKSHARE REGIONS SUPPORT (SINGLE/SECTIONS/FOR)                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * The inner workings of what follows is documented seperately in
 * the OMPi docs/ directory.
 */

/* The array of counters is used cyclically */
#define REGION(idx)  active[(idx) % MAXACTIVEREGIONS]
#define CANCEL_ACTIVATED 1
#define ENV_CANCEL_ACTIVATED 1


/* Called when a new team is created, so as to initialize
 * the workshare regions support (in the parent of the team).
 * For speed, we only initialize the 1st nowait region;
 * the next region is initialized by the first thread to enter the
 * previous one.
 * It is completely useless if the team has 1 thread.
 * There is a special case for a parallel for construct (ispfor flag).
 */
void init_workshare_regions(ort_eecb_t *me)
{
	ort_workshare_t *ws = &me->workshare;

	if (!ws->blocking.inited)
	{
		ee_init_lock(&ws->blocking.reglock, ORT_LOCK_SPIN);
		ee_init_lock(&ws->blocking.forloop.ordering.lock, ORT_LOCK_SPIN);
		ws->blocking.inited = 1;
	}
	ws->blocking.empty = 1;
	ws->blocking.left = 0;       /* Needed for optimizing blocking for regions */
	ws->blocking.forloop.iter = 0; /* ditto */
	ws->blocking.forloop.ordering.next_iteration = 0;  /* ditto */

	if (!ws->REGION(0).inited)
	{
		ee_init_lock(&ws->REGION(0).reglock, ORT_LOCK_SPIN);
		ee_init_lock(&ws->REGION(0).forloop.ordering.lock, ORT_LOCK_SPIN);
		ws->REGION(0).inited = 1;
	}
	ws->REGION(0).empty = 1;
	ws->headregion = ws->tailregion = 0;

	/* Default case; only needed for combined parallel for/sections */
	me->nowaitregion = 0;
}

/* Enter a workshare region, update the corresponding counters
 * and return 1 if i am the first thread to enter the region.
 * It is guaranteed (by the parser and the runtime routines)
 * that we are within a parallel region with > 1 threads.
 */
static
int enter_workshare_region(ort_eecb_t *me,
                           int wstype, int nowait, int hasordered,
                           int nsections)
{
	ort_workshare_t *ws;
	wsregion_t      *r;
	int             myreg, notfirst = 1;

	ws = &(me->parent->workshare);
	myreg = me->mynextNWregion;
	r = (nowait) ? &(ws->REGION(myreg)) : &(ws->blocking);

	me->nowaitregion = nowait;  /* Remember the status of this region */

	if (nowait)
	{
		/* If too many workshare regions are active, we must block here
		 * until the tail has advanced a bit.
		 */
		OMPI_WAIT_WHILE(ws->headregion - ws->tailregion == MAXACTIVEREGIONS  &&
		                myreg == ws->headregion, YIELD_FREQUENTLY);
		(me->mynextNWregion)++;    /* Make me ready for the next region */
	}

	if (!r->empty) return (0);             /* We are not the first for sure */

	ee_set_lock(&r->reglock);       /* We'll be short */
	if (r->empty)                           /* I am the first to enter */
	{
		if (wstype == _OMP_SECTIONS)          /* Initialize */

			r->sectionsleft = nsections;
		else
			if (wstype == _OMP_FOR)
			{
				r->forloop.iter = 0;    /* The very 1st iteration to be given away */
				if (hasordered)
					r->forloop.ordering.next_iteration = 0;
			}

		r->left = 0;      /* None left yet */

		/* Now, prepare/initialize the next region, if not already inited.
		 * This is done so as to avoid serially initializing all regions when
		 * the team was created
		 */
		if (nowait && myreg + 1 < MAXACTIVEREGIONS)
		{
			if (!ws->REGION(myreg + 1).inited)
			{
				ee_init_lock(&ws->REGION(myreg + 1).reglock, ORT_LOCK_SPIN);
				ee_init_lock(&ws->REGION(myreg + 1).forloop.ordering.lock, ORT_LOCK_SPIN);
				ws->REGION(myreg + 1).inited = 1;
			}
			ws->REGION(myreg + 1).empty = 1;
		}

		/* Do this last to avoid races with threads that test without locking */
		if (nowait)(ws->headregion)++;                    /* Advance the head */
		FENCE;
		r->empty = notfirst = 0;
	}
	ee_unset_lock(&r->reglock);

	return (!notfirst);
}


/* Returns the # of threads that have not yet left the region.
 * A zero means that I am the last thread to leave.
 */
static
int leave_workshare_region(ort_eecb_t *me, int wstype)
{
	ort_workshare_t *ws;
	wsregion_t      *r;
	int             myreg, left;

	ws = &(me->parent->workshare);
	myreg = me->mynextNWregion - 1;   /* It is ahead by 1 */
	r = (me->nowaitregion) ? &(ws->REGION(myreg)) : &(ws->blocking);

#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	left = _faa(&(r->left), 1) + 1;
	if (left == me->num_siblings)
	{
		r->empty = 1;
		r->left = 0;
		if (me->nowaitregion)
			(ws->tailregion)++;  /* no race here */
		else
			if (wstype == _OMP_FOR)         /* blocking for */
			{
				r->forloop.iter = 0;          /* Prepare for next use */
				r->forloop.ordering.next_iteration = 0;
			}
	}
#else
	/* Well, it would be better if we used a seperate lock for leaving */
	ee_set_lock(&r->reglock);
	left = ++(r->left);
	ee_unset_lock(&r->reglock);
	if (left == me->num_siblings)
	{
		r->empty = 1;                     /* The region is now empty */
		r->left = 0;
		if (me->nowaitregion)
			(ws->tailregion)++;             /* Advance the tail */
		else
			if (wstype == _OMP_FOR)         /* blocking for */
			{
				r->forloop.iter = 0;          /* Prepare for next use */
				r->forloop.ordering.next_iteration = 0;
			}
	}
#endif
	return (left);
}


/* Returns 1 if the current thread should execute the SINGLE block
 */
int ort_mysingle(int nowait)
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return (1);
	else
		return (enter_workshare_region(me, _OMP_SINGLE, nowait, 0, 0));
}


void ort_leaving_single()
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings != 1)
		leave_workshare_region(me, _OMP_SINGLE);
}


/* This is called in the parser-generated code *only* if we are in a
 * parallel region AND the number of threads is > 1.
 */
void ort_entering_sections(int nowait, int numberofsections)
{
	enter_workshare_region(__MYCB, _OMP_SECTIONS, nowait, 0, numberofsections);
}


/* Ditto */
void ort_leaving_sections()
{
	leave_workshare_region(__MYCB, _OMP_SECTIONS);
}


/* Returns the id of the next section to execute.
 * Returns < 0 if no more sections left.
 * This is guaranteed (by the parser) to be called only if we are
 * within a PARALLEL region with > 1 threads.
 */
int ort_get_section()
{
	wsregion_t *r;
	int        s;
	ort_eecb_t *me = __MYCB;

	s = me->mynextNWregion - 1;  /* My region ('cause it is 1 ahead) */
	r = (me->nowaitregion) ?
	    &(me->parent->workshare.REGION(s)) :
	    &(me->parent->workshare.blocking);

	if (r->sectionsleft < 0) return (-1);
#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	if (me->parent->cancel_sections == CANCEL_ACTIVATED)
		s = -1;
	else
		s = _faa(&(r->sectionsleft), -1) - 1;
#else
	/* Check for active cancellation in order to terminate
	 * case_id assignment in the current team     */
	if (me->parent->cancel_sections == CANCEL_ACTIVATED)
	{
		ee_set_lock(&r->reglock);
		s = -1;
		ee_unset_lock(&r->reglock);
	}
	else
	{
		ee_set_lock(&r->reglock);
		s = --(r->sectionsleft);
		ee_unset_lock(&r->reglock);
	}
#endif

	return (s);
}




/* This is called by the parser-generated code, even if we are
 * not in a parallel region.
 */
void ort_entering_for(int nowait, int hasordered, ort_gdopt_t *t)
{
	ort_eecb_t *me = __MYCB;

	/* Check if we are not in a parallel region or the team has only 1 thread */
	if (me->num_siblings == 1)
	{
		t->data = (int *) 1;   /* anything, except NULL, would do */
		t->me  = (void *) me;
		t->nth = 1;
		return;
	}

	if (nowait)
	{
		enter_workshare_region(me, _OMP_FOR, nowait, hasordered, 0);
		/* 1 less since mynextNWregion has been increased upon entrance */
		t->data = &(me->parent->workshare.
		            REGION(me->mynextNWregion - 1).forloop.iter);
		t->lock = (void *) & (me->parent->workshare.
		                      REGION(me->mynextNWregion - 1).reglock);
	}
	else
	{
		/* enter_workshare_region() is not needed anymore for blocking
		 * regions. Fields have been pre-initialized by the parent of the team
		 * and @ every use, the last thread to leave reinitilizes them.
		 */
		me->nowaitregion = nowait;  /* Remember the status of this region */
		t->data = &(me->parent->workshare.blocking.forloop.iter);
		t->lock = (void *) & (me->parent->workshare.blocking.reglock);
	}
	t->nth = me->num_siblings;
	t->me = me;
}


int ort_leaving_for()
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings == 1)
		return (0);
	else
		return (leave_workshare_region(me, _OMP_FOR));
}


/*
 * ORDERED
 */

void ort_thischunk_range(int lb, int ub)
{
	ort_eecb_t *me;
	(me = __MYCB)->chunklb = lb;
	me->chunkub = ub;
}


void ort_ordered_begin()
{
	ort_ordered_info_t *o;
	ort_eecb_t         *me = __MYCB;

	if (me->num_siblings == 1)
		return;

	o = (me->nowaitregion) ?
	    /* 1 less since mynextNWregion has been increased upon entrance */
	    &(me->parent->workshare.REGION(me->mynextNWregion - 1).forloop.ordering) :
	    &(me->parent->workshare.blocking.forloop.ordering);

	/* Busy wait (we could make it more sophisticated) */
	OMPI_WAIT_WHILE(me->chunklb > o->next_iteration ||
	                o->next_iteration > me->chunkub, YIELD_FREQUENTLY);
}


void ort_ordered_end()
{
	ort_ordered_info_t *o;
	ort_eecb_t         *me = __MYCB;

	if (me->num_siblings == 1)
		return;

	o = (me->nowaitregion) ?
	    /* 1 less since mynextNWregion has been increased upon entrance */
	    &(me->parent->workshare.REGION(me->mynextNWregion - 1).forloop.ordering) :
	    &(me->parent->workshare.blocking.forloop.ordering);

#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	_faa(&(o->next_iteration), 1);
#else
	ee_set_lock(&o->lock);
	(o->next_iteration)++;
	ee_unset_lock(&o->lock);
#endif
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * FOR SCHEDULES (dynamic, guided, static and runtime)               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Calculates total number of iterations for each loop and
 * returns their product. For each loop, a two-element specs array should
 * be given, containing (lb-ub) and step. An optional itp pointer can be
 * given for each loop to get the number of iterations in this loop.
 */
int ort_num_iters(int num, long specs[][2], int *itp[])
{
	int     i, totaliters = 1, iters;
	ldiv_t  diters;

	for (i = 0; i < num; i++)
	{
		diters = ldiv(specs[i][0], specs[i][1]); /* total number of iterations */
		iters = (int) diters.quot;
		if (diters.rem != 0) iters++;
		if (itp[i]) *(itp[i]) = iters;
		totaliters *= iters;
	}
	return (totaliters);
}


/*
 * The inner workings of what follows is documented seperately in
 * the OMPi docs/ directory.
 */


int ort_get_dynamic_chunk(int niters, int chunksize, int *fiter, int *liter,
                          int *ignored, ort_gdopt_t *t)
{

	ort_eecb_t         *me = __MYCB;


	if (chunksize <= 0) ort_error(1, "fatal: dynamic chunksize <= 0 requested!\n");

	/* Check for active cancellation in order to terminate
	 * iteration assignment in the current team     */
	if (me->parent != NULL)
		if (me->parent->cancel_for == CANCEL_ACTIVATED)
			return (0);

	if (t->nth == 1)
	{

		if (t->data == NULL) return (0);      /* t->data used specially here */
		*fiter = 0;                    /* Get just 1 chunk: all iterations */
		*liter = niters;
		t->data = NULL;
		return (1);
	}
	else
	{
		/* t->data shall hold the next iter to give away */
		if (*(t->data) >= niters) { *fiter = niters + 1; return (0); } /* done */


#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
		*fiter = _faa(t->data, chunksize);

#else
		ee_set_lock((ee_lock_t *) t->lock);
		*fiter = *(t->data);
		(*(t->data)) += chunksize;
		ee_unset_lock((ee_lock_t *) t->lock);
#endif
		*liter = *fiter + chunksize;
		if (*liter > niters)
			*liter = niters;
	}
	return (1);
}


/* SUN suggests dividing the number of remaining iters by 2.
 */
int ort_get_guided_chunk(int niters, int chunksize, int *fiter, int *liter,
                         int *ignored, ort_gdopt_t *t)
{
	int   ch;
	ort_eecb_t         *me = __MYCB;



	if (chunksize <= 0) ort_error(1, "fatal: guided chunksize <= 0 requested!\n");

	/* Check for active cancellation in order to terminate
	 * iteration assignment in the current team     */
	if (me->parent != NULL)
		if (me->parent->cancel_for == CANCEL_ACTIVATED)
			return (0);

	if (t->nth == 1)
	{
		if (t->data == NULL) return (0);      /* t->data used specially here */
		*fiter = 0;                    /* Get just 1 chunk: all iterations */
		*liter = niters;
		t->data = NULL;
		return (1);
	}

	if (*(t->data) >= niters) { *fiter = niters + 1; return (0); } /* done */

#if defined(HAVE_ATOMIC_CAS) && !defined(EE_TYPE_PROCESS)
	do
	{
		*fiter = *(t->data);
		ch = niters - *fiter;
		if (ch > chunksize)
		{
			ch = (ch + t->nth - 1) / t->nth;
			if (ch < chunksize)
				ch = chunksize;
		}

		/* Check for active cancellation in order to terminate
		 * iteration assignment in the current team     */
		if (me->parent->cancel_for == CANCEL_ACTIVATED)
			return (0);
	}
	while (!_cas(t->data, (*fiter), (*fiter) + ch));
#else
	ee_set_lock((ee_lock_t *) t->lock);
	*fiter = *(t->data);
	ch = niters - *fiter;
	if (ch > chunksize)
	{
		ch = (ch + t->nth - 1) / t->nth;
		if (ch < chunksize)
			ch = chunksize;
	}
	(*(t->data)) += ch;
	ee_unset_lock((ee_lock_t *) t->lock);
#endif

	*liter = *fiter + ch;
	return (ch != 0);
}


/* Return the sole chunk a thread gets assigned
 */
int ort_get_static_default_chunk(int niters, int *fiter, int *liter)
{
	ort_eecb_t *me = __MYCB;
	div_t      dchunksize;
	int        chunksize, N = me->num_siblings, myid = me->thread_num;

	if (N == 1)
	{
		*fiter = 0;
		*liter = niters;
		return (*fiter != *liter);
	}
	if (niters <= N)    /* less iterations than threads */
	{
		*fiter = myid;
		*liter = (myid < niters) ? myid + 1 : myid;
		return (*fiter != *liter);
	}

	dchunksize = div(niters, N);
	chunksize = dchunksize.quot;                 /* iterations in a chunk */
	niters = dchunksize.rem;
	if (niters) chunksize++;     /* first niters threads get this chunksize */

	if (myid < niters || niters == 0)       /* I get a full chunk */
	{
		*fiter = myid * chunksize;
		*liter = *fiter + chunksize;
	}
	else                                  /* I get a smaller chunk */
	{
		*fiter = niters * chunksize + (myid - niters) * (chunksize - 1);
		*liter = *fiter + (chunksize - 1);
	}
	return (*fiter != *liter);
}


/* Runtime version of the static schedule (suboptimal but unavoidable).
 * chunkid MUST be initialy equal to 0.
 */
int ort_get_runtimestatic_chunk(int niters, int chunksize,
                                int *fiter, int *liter, int *chunkid, ort_gdopt_t *t)
{
	if (t->nth == 1)
	{
		if (*chunkid >= 0) { *fiter = niters + 1; return (0); } /* Only 1 chunk */
		*chunkid = 1;
		*fiter = 0;                    /* Get just 1 chunk: all iterations */
		*liter = niters;
		return (1);
	}

	if (chunksize == -1) /* No chunksize given */
	{
		if (*chunkid == 1) { *fiter = niters + 1; return (0); } /* Only 1 chunk */
		*chunkid = 1;
		return (ort_get_static_default_chunk(niters, fiter, liter));
	}
	else                 /* chunksize given */
	{
		if (chunksize <= 0) ort_error(1, "fatal: chunksize <= 0 requested!\n");
		if (*chunkid < 0)    /* my very first chunk */
			*chunkid = ((ort_eecb_t *) t->me)->thread_num;
		else
			(*chunkid) += (t->nth);
		*fiter = chunksize * (*chunkid);
		if (*fiter >= niters)
			return (0);
		*liter = *fiter + chunksize;
		if (*liter > niters)
			*liter = niters;
		return (1);
	}
}


/* This returns the required function & chunksize to support the
 * RUNTIME schedule code.
 */
void ort_get_runtime_schedule_stuff(chunky_t *func, int *chunksize)
{
	ort_eecb_t *me = __MYCB;
	*chunksize = __CURRTASK(me)->icvs.rtchunk;  /* -1 if not given */
	switch (__CURRTASK(me)->icvs.rtschedule)
	{
		case omp_sched_dynamic:
			*func = ort_get_dynamic_chunk;
			if (*chunksize == -1) *chunksize = 1;
			break;
		case omp_sched_guided:
			*func = ort_get_guided_chunk;
			if (*chunksize == -1) *chunksize = 1;
			break;
		case omp_sched_auto:
			/* The auto schedule is the static one for now */
			*func = ort_get_runtimestatic_chunk;
			break;
		default:
			*func = ort_get_runtimestatic_chunk;
			break;
	}
}
