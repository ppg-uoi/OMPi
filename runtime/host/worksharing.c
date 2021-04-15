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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* worksharing.c -- OMPi RunTime library, workshare regions support */

#include "ort_prive.h"
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "stddefs.h"


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
#define REGION(idx)  active[(idx) % MAX_ACTIVE_REGIONS]


static inline void reset_wsregion(wsregion_t *r)
{
	if (!r->inited)
	{
		ee_init_lock(&r->reglock, ORT_LOCK_SPIN);
		r->inited = 1;
	}
	r->empty = 1;
	r->left = 0;
	memset(&r->forloop, 0, sizeof(ort_forloop_t));
}


/* Called when a new team is created, so as to initialize
 * the workshare regions support (in the parent of the team).
 * For speed, we only initialize the 1st nowait region;
 * the next region is initialized by the first thread to enter the
 * previous one.
 * It is completely useless if the team has 1 thread.
 */
void init_workshare_regions(ort_eecb_t *me)
{
	ort_workshare_t *ws = &me->mf->workshare;

	reset_wsregion(&ws->blocking);
	reset_wsregion(&ws->REGION(0));
	ws->headregion = ws->tailregion = 0;
	me->nowaitregion = 0; /* only needed for combined parallel for/sections */
}


/* Enter a workshare region, update the corresponding counters
 * and return 1 if i am the first thread to enter the region.
 * It is guaranteed (by the parser and the runtime routines)
 * that we are within a parallel region with > 1 threads.
 */
static
int enter_workshare_region(ort_eecb_t *me,
                     int wstype, int nowait, ort_forloop_t *loop, int nsections)
{
	ort_workshare_t *ws;
	wsregion_t      *r;
	int             myreg, notfirst = 1;

	ws = &(TEAMINFO(me)->workshare);
	myreg = me->mynextNWregion;
	r = (nowait) ? &(ws->REGION(myreg)) : &(ws->blocking);

	me->nowaitregion = nowait;  /* Remember the status of this region */

	if (nowait)
	{
		/* If too many workshare regions are active, we must block here
		 * until the tail has advanced a bit.
		 */
		OMPI_WAIT_WHILE(ws->headregion - ws->tailregion == MAX_ACTIVE_REGIONS  &&
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
				r->forloop = *loop;
				r->forloop.iter = 0;    /* The very 1st iteration to be given away */
				r->forloop.curriter = NULL;
#ifdef DOACROSS_FAST
				if (loop->hasordered && !loop->ordnum)
#else
				if (loop->hasordered || loop->ordnum)
#endif
					r->forloop.curriter = (volatile u_long *)
					                      ort_calloc(me->num_siblings * SIZEOF_LONG);
#ifdef DOACROSS_FAST
				r->forloop.mapit = NULL;
				if (loop->ordnum)       /* Allocate iteration map */
					r->forloop.mapit = (volatile unsigned int *) ort_calloc(SIZEOF_INT *
					              (1 + (loop->niters - 1) / (8*SIZEOF_INT)));
#endif
			}

		r->left = 0;      /* None left yet */

		/* Now, prepare/initialize the next region, if not already inited.
		 * This is done so as to avoid serially initializing all regions when
		 * the team was created
		 */
		if (nowait && myreg + 1 < MAX_ACTIVE_REGIONS)
			reset_wsregion(&ws->REGION(myreg + 1));

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

	ws = &(TEAMINFO(me)->workshare);
	myreg = me->mynextNWregion - 1;   /* It is ahead by 1 */
	r = (me->nowaitregion) ? &(ws->REGION(myreg)) : &(ws->blocking);

#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	left = _faa(&(r->left), 1) + 1;
#else
	/* Well, it would be better if we used a seperate lock for leaving */
	ee_set_lock(&r->reglock);
	left = ++(r->left);
	ee_unset_lock(&r->reglock);
#endif

	if (left == me->num_siblings)
	{
		r->empty = 1;
		r->left = 0;
		if (me->nowaitregion)
			(ws->tailregion)++;             /* advance tail; no race here */
		else
			if (wstype == _OMP_FOR)         /* blocking for */
			{
				if (r->forloop.curriter)
					free((void*) r->forloop.curriter);
#ifdef DOACROSS_FAST
				if (r->forloop.mapit)
					free((void*) r->forloop.mapit);
#endif
				memset(&r->forloop,0,sizeof(ort_forloop_t)); /* Prepare for next use */
			}
	}

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
		return (enter_workshare_region(me, _OMP_SINGLE, nowait, NULL, 0));
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
	enter_workshare_region(__MYCB, _OMP_SECTIONS, nowait, NULL, numberofsections);
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
	    &(TEAMINFO(me)->workshare.REGION(s)) :
	    &(TEAMINFO(me)->workshare.blocking);

	if (r->sectionsleft < 0) return (-1);
#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	if (TEAMINFO(me)->cancel_sec_active)
		s = -1;
	else
		s = _faa(&(r->sectionsleft), -1) - 1;
#else
	/* Check for active cancellation in order to terminate
	 * case_id assignment in the current team     */
	if (TEAMINFO(me)->cancel_sec_active)
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
void ort_entering_for(int nowait, int hasordered)
{
	ort_eecb_t    *me = __MYCB;
#ifdef DOACROSS_FAST
	ort_forloop_t loop = { hasordered, 0, 0, 0, 0, NULL, NULL };
#else
	ort_forloop_t loop = { hasordered, 0, 0, 0, 0, NULL, FOR_SCHED_NONE, 0 };
#endif

	/* Ditch if we are not in a parallel region or the team has only 1 thread */
	if (me->num_siblings > 1)
	{
		if (nowait)
			enter_workshare_region(me, _OMP_FOR, nowait, &loop, 0);
		else
		{
			/* enter_workshare_region() is not needed anymore for non-ordered
			 * blocking regions. Fields have been pre-initialized by the parent of
			 * the team and @ every use, the last thread to leave reinitializes them.
			 */
			me->nowaitregion = nowait;  /* Remember the status of this region */
			if (hasordered)
				enter_workshare_region(me, _OMP_FOR, nowait, &loop, 0);
		}
	}
	else
		me->nowaitregion = 1;         /* Ugly hack to mark 1st chunk */
}


static u_long mult_iters(int ordnum, long params[][3])
{
	u_long nits = 1;
	
	for (--ordnum; ordnum >= 0; ordnum--)
		nits *= params[ordnum][2];
	return (nits);
}


void ort_entering_doacross(int nowait, int ordnum, int colnum,
                           int schedtype, int chsize, long params[][3])
{
	ort_eecb_t    *me = __MYCB;
	u_long        niters = mult_iters(ordnum, params);
	ort_forloop_t loop = {
#ifdef DOACROSS_FAST
		0, ordnum, colnum, (int) niters, 0, NULL, NULL
#else
		0, ordnum, colnum, (int) niters, 0, NULL, schedtype, chsize
#endif
	};

	/* Ditch if we are not in a parallel region or the team has only 1 thread */
	if (me->num_siblings > 1)
	{
		if (!nowait)
			me->nowaitregion = nowait;  /* Remember the status of this region */
		enter_workshare_region(me, _OMP_FOR, nowait, &loop, 0);
	}
	else
		me->nowaitregion = 1;         /* Ugly hack to mark 1st chunk */
}


/* 1 less since mynextNWregion has been increased upon entrance */
#define MYLOOP(me) ( (me)->nowaitregion ? \
	    &(TEAMINFO(me)->workshare.REGION((me)->mynextNWregion - 1).forloop) : \
	    &(TEAMINFO(me)->workshare.blocking.forloop) )


/* Same for doacross, too */
int ort_leaving_for()
{
	ort_eecb_t *me = __MYCB;

	if (me->num_siblings == 1)
		return (0);
	else
	{
		/* Mark my last iteration; because I may never get one, store infinity */
		ort_forloop_t *loop = MYLOOP(me);
		if (loop->curriter)
			loop->curriter[ me->thread_num ] = ULONG_MAX;
		return (leave_workshare_region(me, _OMP_FOR));
	}
}


/*
 * (PLAIN) ORDERED
 */


/* Only called when a loop has plain oredered regions */
void ort_for_curriter(u_long iter)
{
	ort_eecb_t *me = __MYCB;
	if (me->num_siblings != 1)
		MYLOOP(me)->curriter[ me->thread_num ] = iter;
}


bool check_ordered(volatile u_long *curriter, u_long myiter, int nthr)
{
	int i;

	for (i = 0; i < nthr; i++)
		if (curriter[i] < myiter)
			return true;
	return false;
}


void ort_ordered_begin()
{
	ort_eecb_t *me = __MYCB;
	volatile u_long *threaditers = MYLOOP(me)->curriter;

	if ( me->num_siblings != 1 && threaditers[me->thread_num] > 0 )
		OMPI_WAIT_WHILE(check_ordered(threaditers, threaditers[ me->thread_num ],
		                me->num_siblings), YIELD_FREQUENTLY);
}


void ort_ordered_end()
{
	ort_eecb_t *me = __MYCB;

	if (me->num_siblings > 1)
		MYLOOP(me)->curriter[ me->thread_num ]++;  /* Lie, but for good purpose */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * FOR SCHEDULES (dynamic, guided, static and runtime)               *
 *                                                                   *
 * OMPi normallizes all loops and uses unsigned long) to count the   *
 * number of iterations.                                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * The inner workings of what follows is documented seperately in
 * the OMPi docs/ directory.
 */


int ort_get_dynamic_chunk(u_long niters, u_long chunksize, int monotonic,
                     u_long *fiter, u_long *liter, int *ignored)
{
	ort_eecb_t      *me = __MYCB;
	volatile u_long *iter;

	if (chunksize == 0) ort_error(1, "fatal: dynamic chunksize 0 requested!\n");

	/* Check for active cancellation in order to terminate
	 * iteration assignment in the current team
	 */
	if (me->parent != NULL)
		if (TEAMINFO(me)->cancel_for_active)
			return (0);

	if (me->num_siblings == 1)
	{
		if (me->nowaitregion == 0) return 0;
		*fiter = 0;              /* Get just 1 chunk: all iterations */
		*liter = niters;
		me->nowaitregion = 0;    /* Ugly hack to mark 1st chunk */
		return (1);
	}

	/* iter shall hold the next iter to give away */
	iter = &( MYLOOP(me)->iter );
	if (*iter >= niters)
	{
		SKIPLOOP:
			*fiter = *liter = niters + 1;
			return 0;
	}

#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	*fiter = _faa(iter, chunksize);
#else
	{
		ee_lock_t *lock = (me->nowaitregion) ? 
	          &(TEAMINFO(me)->workshare.REGION(me->mynextNWregion - 1).reglock) :
	          &(TEAMINFO(me)->workshare.blocking.reglock);
						
		ee_set_lock(lock);
		*fiter = *iter;
		(*iter) += chunksize;
		ee_unset_lock(lock);
	}
#endif

	if (*fiter >= niters)   /* double check; races may lead us here... */
		goto SKIPLOOP;

	*liter = *fiter + chunksize;
	if (*liter > niters)
		*liter = niters;
	return (1);
}


/* SUN suggests dividing the number of remaining iters by 2.
 */
int ort_get_guided_chunk(u_long niters, u_long chunksize, int monotonic,
                     u_long *fiter, u_long *liter, int *ignored)
{
	ort_eecb_t      *me = __MYCB;
	volatile u_long *iter;
	long            ch;

	if (chunksize == 0) ort_error(1, "fatal: guided chunksize 0 requested!\n");

	/* Check for active cancellation in order to terminate
	 * iteration assignment in the current team
	 */
	if (me->parent != NULL)
		if (TEAMINFO(me)->cancel_for_active)
			return (0);

	if (me->num_siblings == 1)
	{
		if (me->nowaitregion == 0) return 0;
		*fiter = 0;              /* Get just 1 chunk: all iterations */
		*liter = niters;
		me->nowaitregion = 0;    /* Ugly hack to mark 1st chunk */
		return (1);
	}

	iter = &( MYLOOP(me)->iter );
	if (*iter >= niters)
	{
		SKIPLOOP:
			*fiter = *liter = niters + 1;
			return 0;
	}

#if defined(HAVE_ATOMIC_CAS) && !defined(EE_TYPE_PROCESS)
	do
	{
		*fiter = *iter;
		ch = niters - *fiter;
		if (ch > chunksize)
		{
			ch = (ch + me->num_siblings - 1) / me->num_siblings;
			if (ch < chunksize)
				ch = chunksize;
		}
	}
	while (!_cas(iter, (*fiter), (u_long) ((*fiter) + ch)));
#else
	{
		ee_lock_t *lock = (me->nowaitregion) ? 
	          &(TEAMINFO(me)->workshare.REGION(me->mynextNWregion - 1).reglock) :
	          &(TEAMINFO(me)->workshare.blocking.reglock);
						
		ee_set_lock(lock);
		*fiter = *iter;
		ch = niters - *fiter;
		if (ch > chunksize)
		{
			ch = (ch + me->num_siblings - 1) / me->num_siblings;
			if (ch < chunksize)
				ch = chunksize;
		}
		(*iter) += ch;
		ee_unset_lock(lock);
	}
#endif

	if (*fiter >= niters)   /* double check; races may lead us here... */
	  goto SKIPLOOP;

	*liter = *fiter + ch;
	return (ch != 0);
}


/* Return the sole chunk a thread gets assigned
 */
int ort_get_static_default_chunk(u_long niters, u_long *fiter, u_long *liter)
{
	ort_eecb_t *me = __MYCB;
	int        N = me->num_siblings, myid = me->thread_num;
	u_long     chunksize;

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

	chunksize = niters / N;
	niters = niters % N;
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


#ifndef DOACROSS_FAST

/* Return the thread id that is assigned the given iteration
 * The inverse function of ort_get_static_default_chunk().
 */
int _get_static_default_iter_thread(int iterid, int niters, int nthr)
{
	int chunksize;

	if (nthr == 1)
		return (0);
	if (niters <= nthr)    /* less iterations than threads */
		return (iterid);
	chunksize = niters / N;
	niters = niters % N;
	if (niters) chunksize++;     /* first niters threads get this chunksize */

	if (niters == 0 || iterid < niters * chunksize)
		return (iterid/chunksize);
	else                                  /* I get a smaller chunk */
	{
		iterid -= niters * chunksize;
		return (niters + (iterid/(chunksize-1)));
	}
}


/* Return the thread id that is assigned the given iteration when the
 * schedule is static with specified chunksize.
 * Iterations are given in `rounds' of (nthr * chsize).
 * First, we determine the round and then the iteration within it.
 */
int _get_static_chunksize_iter_thread(int iterid, int chsize, int nthr)
{
	if (nthr == 1)
		return (0);
	iterid = iterid % (nthr*chsize);  /* Get id within the round */
	return (iterid / chsize);
}

#endif


/* Runtime version of the static schedule (suboptimal but unavoidable).
 * chunkid MUST be initialy equal to 0.
 */
int ort_get_runtimestatic_chunk(u_long niters, u_long chunksize, int monotonic,
                                u_long *fiter, u_long *liter, int *chunkid)
{
	ort_eecb_t *me = __MYCB;

	if (me->num_siblings == 1)
	{
		if (*chunkid >= 0) { *fiter = niters + 1; return (0); } /* Only 1 chunk */
		*chunkid = 1;
		*fiter = 0;                    /* Get just 1 chunk: all iterations */
		*liter = niters;
		return (1);
	}

	if (chunksize == 0)  /* No chunksize given */
	{
		if (*chunkid == 1) { *fiter = niters + 1; return (0); } /* Only 1 chunk */
		*chunkid = 1;
		return ( ort_get_static_default_chunk(niters, fiter, liter) );
	}
	else                 /* chunksize given */
	{
		if (chunksize == 0) ort_error(1, "fatal: runtime chunksize is 0\n");
		if (*chunkid < 0)    /* my very first chunk */
			*chunkid = me->thread_num;
		else
			(*chunkid) += me->num_siblings;
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
void ort_get_runtime_schedule_stuff(chunky_t *func, u_long *chunksize)
{
	ort_eecb_t *me = __MYCB;

	*chunksize = __CURRTASK(me)->icvs.rtchunk;  /* -1 if not given */
	switch (__CURRTASK(me)->icvs.rtschedule)
	{
		case omp_sched_dynamic:
			*func = ort_get_dynamic_chunk;
			if (*chunksize == 0) *chunksize = 1;
			break;
		case omp_sched_guided:
			*func = ort_get_guided_chunk;
			if (*chunksize == 0) *chunksize = 1;
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


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * DOACROSS LOOPS (ORDERED WITH DEPEND CLAUSES)                      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//FIXME: iteration count type must be fixed everywhere


/**
 * Given the index value, calculate the iteration number
 * @param index  the index value
 * @param params param[0/1/2] are the loop lower bound/step/#iterations
 * @param err    becomes false if the index is invalid
 * @return the iteration number
 */
static inline u_long index2iterid(long index, long params[3], int *err)
{
	long q = (index-params[0]) / params[1];
	long r = (index-params[0]) % params[1];
	*err = (r || q < 0 || q >= params[2]);
	return q;
}


static u_long 
get_iteration_id(int nord, long idx[], long params[][3], int *err)
{
	u_long globiter = 0, iter;
	int i;

	for (i = 0; i < nord; i++)
	{
		iter = index2iterid(idx[i], params[i], err) + globiter;
		if (*err) return 0;       /* illegal iteration -- give up */
		globiter = (i == nord-1) ? iter : iter * ((u_long) params[i+1][2]);
	}
	return globiter;
}


void ort_doacross_wait(long params[][3], int ndeps, long *deps)
{
	ort_eecb_t *me = __MYCB;
	u_long iter;
	int i, j, n, nthr = me->num_siblings, err;
	ort_forloop_t *loop = MYLOOP(me);
#ifdef DOACROSS_FAST
	unsigned int mask;
	volatile unsigned int *mapelem;
#else
	volatile u_long *mapelem;
#endif

	if (nthr == 1)
		return;

	n = loop->ordnum;
	for (j = 0; j < ndeps; j++)
	{
		iter = get_iteration_id(n, &deps[j*n], params, &err);
		if (err)
			continue;

		/* Synchronize */
#ifdef DOACROSS_FAST
		mapelem = loop->mapit + (iter / (8*SIZEOF_INT));
		mask = 1 << (iter % (8*SIZEOF_INT));
		OMPI_WAIT_WHILE(((*mapelem & mask) == 0), YIELD_IMMEDIATELY);
#else
		if (loop->schedtype == FOR_SCHED_STATIC)
			i = _get_static_default_iter_thread(iter, loop->niters, nthr);
		else
			if (loop->schedtype == FOR_SCHED_STATIC_CHUNK)
				i = _get_static_chunksize_iter_thread(iter, loop->chsize, nthr);
			else
				ort_error(1, "[doacross wait]: nonFAST supports only static schedules\n");
		if (i == me->thread_num)
			continue;
		mapelem = &( loop->curriter[i] );
		OMPI_WAIT_WHILE(*mapelem < iter, YIELD_IMMEDIATELY);
#endif
	}
}


#ifdef DOACROSS_FAST

void ort_doacross_post(long params[][3], long *idx)
{
	ort_eecb_t *me = __MYCB;
	ort_forloop_t *loop = MYLOOP(me);
	u_long iter;
	int err;
	unsigned int mask, val;
	volatile unsigned int *mapelem;

	if (me->num_siblings == 1)
		return;

	iter = get_iteration_id(loop->ordnum, idx, params, &err);
	if (err)
		return;
	
	mapelem = loop->mapit + (iter / (8*SIZEOF_INT));
	mask = 1 << (iter % (8*SIZEOF_INT));
#if defined(HAVE_ATOMIC_CAS) && !defined(EE_TYPE_PROCESS)
		/* We could simply use __sync_fetch_and_or() instead... */
	while (((val = *mapelem) & mask) == 0)   /* Check that it got set */
		_cas(mapelem, val, val | mask);
#else
	{
		wsregion_t *r = me->nowaitregion ?
				&(TEAMINFO(me)->workshare.REGION(me->mynextNWregion - 1)) :
				&(TEAMINFO(me)->workshare.blocking);
		ee_set_lock(&r->reglock);
			if ((*mapelem & mask) == 0)  /* double check */
				*mapelem |= mask;
		ee_unset_lock(&r->reglock);
	}
#endif
}


#else

void ort_doacross_post(long params[][3], long *idx)
{
	ort_eecb_t *me = __MYCB;
	ort_forloop_t *loop = MYLOOP(me);
	u_long iter;
	int err;
	volatile u_long *myiter;

	if (me->num_siblings == 1)
		return;
	iter = get_iteration_id(loop->ordnum, idx, params, &err);
	if (err)
		return;
	if (*(myiter = loop->curriter + me->thread_num) != iter)
		*myiter = iter;     /* OMPi normalizes all loops */
}

#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * TEMPORARY PLACE FOR CRITICAL REGIONS (moved here so that          *
 * ort_prepare_omp_lock is not bound early by the linker)            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* As in atomic, the binding set is all the threads in the program
 */
void ort_critical_begin(omp_lock_t *critlock)
{
	/* Because critical locks have external scope, they are initialized
	 * to NULL, thus we can safely differentiate between uninitialized
	 * and initialized ones.
	 */
	if (*critlock == NULL) ort_prepare_omp_lock(critlock, ORT_LOCK_SPIN);

#if defined(EE_TYPE_PROCESS_MPI)
	ee_set_lock((ee_lock_t *) critlock);
#else
	ee_set_lock((ee_lock_t *) *critlock);
#endif
}

void ort_critical_end(omp_lock_t *critlock)
{
#if defined(EE_TYPE_PROCESS_MPI)
	ee_unset_lock((ee_lock_t *) critlock);
#else
	ee_unset_lock((ee_lock_t *) *critlock);
#endif
}
