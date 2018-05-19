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

/* ort.c -- OMPi RunTime library */

/*
 * 2011/12/13:
 *   fixed an initialization bug in tp variables.
 * 2010/12/17:
 *   fixed bug in threadprivate variables (PEH).
 * 2010/11/20:
 *   fixed bugs in ort_initialize(), ort_execute_serial(),
 *   ort_execute_parallel() and ort_get_thrpriv(); redesigned t
 *   threadprivate handling.
 * Version 1.0.2b
 *   ort_ee_dowork() (old ort_get_thread_work()) now also call the thread
 *   function.
 * Version 1.0.1j
 *   ort.c code split & reorganization; see ort_*.c
 * Version 1.0.0e
 *   added OpenMP 3.0 runtime functions
 * Version 1.0.0a
 *   added atomic operations
 * Version 0.9.9
 *   thrinfo renamed to eecb; thrinfo block recyler ditched;
 *   ort_get_thread_work() changed.
 * Version 0.9.2
 *   Recycling of thrinfo team structs; added level to ee_create().
 * Version 0.9.1
 *   New & better threadprivate code
 *   Ditched some functions (ort_destroy_team, ort_assign_key, etc).
 *   ort_create_team() --> ort_execute_parallel().
 * Version 0.8.6
 *   More bug fixes; fixed support for both clock_gettime & gettimeofday.
 * Version 0.8.5n.x
 *   Small fixes and improvements / added ort_finalize().
 * Version 0.8.4.8
 *   19 Nov. 2006
 *     Too many changes to mention. Added code for critical & atomic,
 *     moved stuff around, made cache-aligned allocators, move omp.c code
 *     here, ...
 * Version 0.8.4.5
 *   3 Nov. 2006
 *     Removed all pool-related stuff and moved it to the thread library
 *   5 Nov. 2006
 *     Fixed copyprivate parser bug & added runtime copyprivate support.
 * Version 0.8.4.4
 *   Removed locks for threads waiting for work; now they spin.
 * Version 0.8.4.3
 *   31 Oct. 2006
 *     New code for ORDERED (uses busy waiting and no strange locking.
 *     Also removed the join_lock (which was also locked/unlocked in
 *          non-standard manner). Join is also based on bbusy-waiting.
 *     Now TT structrures are mostly initialized by each thread, not
 *          by the master thread.
 *     _destroy_team() now takes no argument.
 * Version 0.8.4
 *   Too many changes: SSF rounds gone for ever; schedule support
 *   revamped; ordered improved; num_single/sections/for gone away;
 *   and many many others.
 * Version 0.8.3
 *   (1) For SECTIONS and SINGLE, SSF rounds are not used any more.
 *       Now they use a completely different system, through
 *       enter_workshare_region()
 *   (2) sched_info removed completely; It was used in every SSF round
 *       for remembering the runtime schedule of FORs. Now all enviromental
 *       variables are read once in the beginning (_omp_initialize())
 *       and they are saved in global variables.
 */

#include "ort_prive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * GLOBAL VARIABLES / DEFINITIONS / MACROS                           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All global variables ORT handles; if ee=proc, this is also placed
 * in shared memory.
 */
static ort_vars_t ort_globals;  /* All ORT globals stored here */
ort_vars_t        *ort;         /* Pointer to ort_globals */


static int        ort_initialized = 0;

/* Execution entity (thread/process) control block */
#ifdef USE_TLS
	TLS_KEYWORD void *myeecb;
#else
	ee_key_t eecb_key;  /* Key for ort's "thread"-specific data; the actual
	                       data is a pointer to the eecb */
#endif

/* Handy macro */
#define initialize_eecb(eecb) {\
		(eecb)->parent              = NULL;\
		(eecb)->sdn                 = (eecb);\
		(eecb)->num_children        = 0;\
		(eecb)->num_siblings        = 1;     /* We are just 1 thread! */\
		(eecb)->thread_num          = 0;\
		(eecb)->level               = 0;     /* The only one in level 0 */\
		(eecb)->activelevel         = 0;     /* The only one in level 0 */\
		(eecb)->shared_data         = 0;\
		(eecb)->mynextNWregion      = 0;\
		(eecb)->have_created_team   = 0;\
		(eecb)->tasking.queue_table = 0;\
		(eecb)->ee_info             = NULL;  /* *Must* init to NULL */\
		(eecb)->tg_recycler         = NULL;  /* Recycler for taskgroup*/\
	}

/* Process id */
#if defined(EE_TYPE_PROCESS)
	#define __MYPID  ee_pid()
#else
	#define __MYPID  0
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * INITIALIZATION / SHUTDOWN                                         *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * Last function called (just before exit).
 * "exitval" is what the original (user) main() returned.
 */
void ort_finalize(int exitval)
{
	ort_finalize_devices();
	ee_finalize(exitval);
	fflush(stderr);       /* Because _exit may not flush stdio buffers */
	fflush(stdout);
	_exit(exitval);             /* Make sure nothing else is called */
}


/* Called only if the user calls exit() */
void _at_exit_call(void)
{
	ee_finalize(-1);
}


/*
 * First function called.
 */
int ort_initialize(int *argc, char ***argv, int nmodules, ...)
{
	ort_eecb_t      *initial_eecb;
	ort_task_node_t *initial_task;
  va_list         ap;

	if (ort_initialized) return 0;

	ort = &ort_globals;

	ort->icvs.ncpus           = ort_get_num_procs(); /* Default ICV values */
	ort->icvs.stacksize       = -1;                  /* OpenMP 3.0 */
	ort->icvs.threadlimit     = -1;                  /* (unlimited) OpenMP 3.0 */
	ort->icvs.levellimit      = -1;                  /* (unlimited) OpenMP 3.0 */
	ort->icvs.waitpolicy      = _OMP_ACTIVE;         /* (ignored) OpenMP 3.0 */
	ort->icvs.nthreads        = -1;        /* per-task; no preference for now */
	ort->icvs.rtschedule      = omp_sched_auto;
	ort->icvs.rtchunk         = -1;
	ort->icvs.dynamic         = 1;
	ort->icvs.nested          = 0;
	ort->icvs.proc_bind       = omp_proc_bind_true;  /* OpenMP 4.0.0 */
	ort->icvs.cancel          = 0;                   /* OpenMP 4.0.0 */
	ort->icvs.def_device      = 0; /* This is HOST      OpenMP 4.0.0 */
	ort->added_devices        = 0;                   /* OpenMP 4.0.0 */
	ort_get_default_places();                        /* OpenMP 4.0.0 */

	/* Initialize module and device structures */
	va_start(ap, nmodules);
	ort_discover_modules(nmodules, ap);
	va_end(ap);

	if(ort->added_devices > 1)
		ort->icvs.def_device = 1; /* This the first available device. */

	/* Initialize declared variables structure */
	ort_initialize_decl();

	/* We need the number of added devices in order to set the limits of
	 * environmental variable OMP_DEFAULT_DEVICE
	 */
	ort_get_environment();                       /* Get environmental variables */

	/* Initialize taskqueuesize */
	if (ort->dynamic_taskqueuesize)
		ort->taskqueuesize = 3 * (ort->icvs.ncpus);

	/* Initialize the thread library */
	if (ort->icvs.nthreads > 0) ort->icvs.nthreads--; /* 1- for eelib */
	if (ee_initialize(argc, argv, &ort->icvs, &ort->eecaps) != 0)
		ort_error(1, "cannot initialize the thread library.\n");

	if (__MYPID == 0)               /* Everybody in case of threads */
	{
		/* Check for conformance to user requirements */
		if (ort->icvs.nthreads == -1)  /* Let the eelib set the default */
			ort->icvs.nthreads = ort->eecaps.default_numthreads + 1;
		else                          /* user asked explicitely */
		{
			if (ort->eecaps.max_threads_supported > -1 &&
			    ort->icvs.nthreads < ort->eecaps.max_threads_supported)
				if (!ort->icvs.dynamic || !ort->eecaps.supports_dynamic)
					ort_error(1, "the library cannot support the requested number (%d) "
					          "of threads.\n", ort->icvs.nthreads + 1);
			ort->icvs.nthreads++;        /* Restore value */
		}
		/* Fix discrepancies */
		if (ort->icvs.dynamic && !ort->eecaps.supports_dynamic)
			ort->icvs.dynamic = 0;
		if (ort->icvs.nested  && !ort->eecaps.supports_nested)
			ort->icvs.nested  = 0;
		check_nested_dynamic(ort->icvs.nested, ort->icvs.nested); /* is eelib ok? */

		/* OpenMP 3.0 stuff */
		if (ort->icvs.levellimit == -1)
			ort->icvs.levellimit = ort->eecaps.max_levels_supported;
		else
			if (ort->eecaps.max_levels_supported != -1 &&
			    ort->eecaps.max_levels_supported < ort->icvs.levellimit)
				ort->icvs.levellimit = ort->eecaps.max_levels_supported;

		if (ort->icvs.threadlimit == -1)
			ort->icvs.threadlimit = ort->eecaps.max_threads_supported;
		else
			if (ort->eecaps.max_threads_supported != -1 &&
			    ort->eecaps.max_threads_supported < ort->icvs.threadlimit)
				ort->icvs.threadlimit = ort->eecaps.max_threads_supported;

		/* Initialize the 3 locks we need */
		ee_init_lock((ee_lock_t *) &ort->atomic_lock, ORT_LOCK_SPIN);
		ee_init_lock((ee_lock_t *) &ort->preparation_lock, ORT_LOCK_NORMAL);
		ee_init_lock((ee_lock_t *) &ort->eecb_rec_lock, ORT_LOCK_NORMAL);

		/* Recycle bin of eecbs is empty */
		ort->eecb_reycler = NULL;

		ort->thrpriv_num = 0;

		/* The initial thread */
		initial_eecb = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
		initialize_eecb(initial_eecb);
		/* At least 1 row is needed for initial thread's threadprivate vars */
		initial_eecb->tpkeys = ort_calloc(ort->icvs.ncpus * sizeof(ort_tptable_t));
		initial_eecb->tpksize = ort->icvs.ncpus;

		/* The master's impicit task ("initial" task) */
		initial_task = (ort_task_node_t *) ort_calloc(sizeof(ort_task_node_t));

		initial_task->icvs.dynamic         = ort->icvs.dynamic;
		initial_task->icvs.nested          = ort->icvs.nested;
		initial_task->icvs.rtschedule      = ort->icvs.rtschedule;
		initial_task->icvs.rtchunk         = ort->icvs.rtchunk;
		initial_task->icvs.nthreads        = ort->icvs.nthreads;
		/* OpenMP 4.0.0 */
		initial_task->icvs.def_device      = ort->icvs.def_device; /* OpenMP 4.0 */
		initial_task->icvs.threadlimit     = ort->icvs.threadlimit;
		initial_task->icvs.proc_bind       = ort->icvs.proc_bind;
		initial_task->taskgroup            = 0;
		initial_task->icvs.cur_de          = NULL;

		__SETCURRTASK(initial_eecb, initial_task);
		__SETCURRIMPLTASK(initial_eecb, initial_task);

#ifdef USE_TLS
#else
		ee_key_create(&eecb_key, 0);  /* This key stores a pointer to the eecb */
#endif
		__SETMYCB(initial_eecb);
		ort_init_tasking();

#ifdef ORT_DEBUG
		ort_debug_thread("<this is the master thread>");
#endif

		/* Upon exit .. */
		atexit(_at_exit_call);
	}

#if defined(EE_TYPE_PROCESS)
	ort_share_globals();
#endif

	if (display_env)
		display_env_vars();


	return (ort_initialized = 1);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * SHARED MEMORY FOR THE PROCESS MODEL                               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#if defined(EE_TYPE_PROCESS)

static ort_sglvar_t *ort_sglvar_list;   /* The list of requests */
static void         *ort_sglvar_area;   /* The start of the alloted memory */
static int shared_data_id;

void ort_share_globals()
{
	ort_sglvar_t *req;
	char         *mem;
	int          rsize;
	int          list_size = ((ort_sglvar_list != NULL) ? ort_sglvar_list->size :
	                          0);

	/* Memory layout : a) shared globals (sgl), b) ort_globals, c) master_eecb, d) master_task */

	ort_shmalloc(&ort_sglvar_area,
	             list_size + sizeof(ort_globals) + sizeof(ort_eecb_t) + sizeof(ort_task_node_t),
	             &shared_data_id);
	memcpy(ort_sglvar_area + list_size, ort, sizeof(ort_globals));
	memcpy(ort_sglvar_area + list_size + sizeof(ort_globals), __MYCB,
	       sizeof(ort_eecb_t));
	memcpy(ort_sglvar_area + list_size + sizeof(ort_globals) + sizeof(ort_eecb_t),
	       __CURRTASK(__MYCB), sizeof(ort_task_node_t));
	ort = (ort_vars_t *)(((char *) ort_sglvar_area) + list_size);
	__SETMYCB(ort_sglvar_area + list_size + sizeof(ort_globals));
	__SETCURRTASK(__MYCB, ort_sglvar_area + list_size + sizeof(
	                ort_globals) + sizeof(ort_eecb_t));

	for (mem = ort_sglvar_area, req = ort_sglvar_list; req != NULL;)
	{
		rsize = (req->next != NULL) ? req->size - req->next->size
		        : req->size;
		*(req->varptr) = (void *) mem;
		if (req->initvalue)
			memcpy(mem, req->initvalue, rsize);
		else
			memset(mem, 0, rsize);

		mem += rsize;

		req = (ort_sglvar_list = req)->next;
		free(ort_sglvar_list);
	}
}


/* Should mark all allocation requests; it is guaranteed by the parser
 * that this is called *before* main() starts. Thus, it should only file
 * the requests and do the actual allocations later, when ort_initialize()
 * is called (*dataptr should then be made to point to an allocated
 * space of size bytes).
 */
void ort_sglvar_allocate(void **varptr, int size, void *initer)
{
	ort_sglvar_t *req = (ort_sglvar_t *) ort_calloc(sizeof(ort_sglvar_t));

	req->varptr     = varptr;
	req->size       = ort_sglvar_list ? size + ort_sglvar_list->size : size;
	req->initvalue  = initer;
	req->next       = ort_sglvar_list;
	ort_sglvar_list = req;
}

#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * THREADS & THEIR TEAMS                                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Here an ee initializes its control block and calls the actual thread
 * function (this should be called from the ee library, by the actual ee).
 */
void ort_ee_dowork(int eeid, void *parent_info)
{
	ort_eecb_t *t = __MYCB, *parent = (ort_eecb_t *) parent_info;

	if (t == NULL)         /* 1st time around */
	{
		__SETMYCB(t = (ort_eecb_t *)ort_allocate_eecb_from_recycle());
		t->ee_info = NULL;   /* not needed actually due to calloc */
	}

	/* Prepare my control block */
	t->parent         = parent;
	t->num_siblings   = parent->num_children;
	t->level          = parent->level + 1;       /* 1 level deeper */
	t->thread_num     = eeid;                    /* Thread id within the team */
	t->num_children   = 0;
	t->shared_data    = NULL;
	t->sdn            = parent;
	t->mynextNWregion = 0;
	t->nowaitregion   = 0;          /* VVD--actually we don't need to do this */
	t->activelevel    = parent->activelevel +       /* OpenMP 3.0 - team of 1 */
	                    ((t->num_siblings > 1) ? 1 : 0);  /* implies inactive */

#ifdef ORT_DEBUG
	ort_debug_thread("in ort_ee_dowork(); about to execute func.");
#endif

	ort_start_implicit_task(t);                 /* This is an implicit task */
	(*(parent->workfunc))(t->sdn->shared_data); /* Execute the actual function */
	ort_finish_implicit_task(t);                /* Implicit task done */
}


/* This prepares everything so that I become the master of a new team
 */
void prepare_master(ort_eecb_t *me,
                    int teamsize, void *(*func)(void *), void *shared)
{
	me->num_children    = teamsize;
	me->shared_data     = (shared == NULL) ? me->sdn->shared_data : shared;
	me->workfunc        = func;
	me->cancel_parallel = 0;
	me->cancel_sections = 0;
	me->cancel_for      = 0;
	ee_barrier_init(&me->barrier, teamsize);
	ort_task_queues_init(me, teamsize - 1);

	if (!me->have_created_team)
	{
		ee_init_lock(&me->copyprivate.lock, ORT_LOCK_SPIN);
		me->workshare.blocking.inited = 0;
		/* VVD-new */
		me->me_master = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
		me->have_created_team = 1;
	}

	/* We never shrink the tpkeys array. we may consider freeing the
	 * actual variables some day.
	 */
	if (me->tpksize < teamsize)    /* Need more space for children thrpriv vars */
	{
		me->tpkeys = (me->tpksize == 0) ?
		             ort_alloc((teamsize + 3) * sizeof(ort_tptable_t)) :
		             ort_realloc(me->tpkeys, (teamsize + 3) * sizeof(ort_tptable_t));
		memset(&me->tpkeys[me->tpksize], 0,  /* zero out new entries */
		       ((teamsize + 3) - me->tpksize)*sizeof(ort_tptable_t));
		me->tpksize = teamsize + 3;
		FENCE;
	}

	if (teamsize > 1)
		init_workshare_regions(me);

	assert(me->me_master != NULL);
}

/*
 * This function is called in case of target if(false).
 * A new eecb (and task node) is acquired. Icvs are initialized from
 * the initial ones which means that again nested level is 0 etc.
 * Finally thread execute the kernel func code.
 */
void ort_execute_kernel_on_host(void *(*func)(void *), void *shared)
{
	ort_eecb_t eecb_for_dev, *prev_eecb;
	ort_task_node_t task_for_dev;

	initialize_eecb(&eecb_for_dev);
	/* At least 1 row is needed for initial thread's threadprivate vars */
	eecb_for_dev.tpkeys = ort_calloc(ort->icvs.ncpus * sizeof(ort_tptable_t));
	eecb_for_dev.tpksize = ort->icvs.ncpus;

	/* The impicit task ("initial" task) */
	task_for_dev.icvs.dynamic         = ort->icvs.dynamic;
	task_for_dev.icvs.nested          = ort->icvs.nested;
	task_for_dev.icvs.rtschedule      = ort->icvs.rtschedule;
	task_for_dev.icvs.rtchunk         = ort->icvs.rtchunk;
	task_for_dev.icvs.nthreads        = ort->icvs.nthreads;
	/* OpenMP 4.0.0 */
	task_for_dev.icvs.def_device      = ort->icvs.def_device; /* OpenMP 4.0 */
	task_for_dev.icvs.threadlimit     = ort->icvs.threadlimit;
	task_for_dev.icvs.proc_bind       = ort->icvs.proc_bind;
	task_for_dev.taskgroup            = 0;
	task_for_dev.icvs.cur_de          = NULL;

	/* Save old eecb */
	prev_eecb = __MYCB;
	/* Assign new eecb */
	__SETMYCB(&eecb_for_dev);

	__SETCURRTASK(&eecb_for_dev, &task_for_dev);
	__SETCURRIMPLTASK(&eecb_for_dev, &task_for_dev);

	/* Execute kernel func. */
	(*func)(shared);

	/* Retrive old eecb */
	__SETMYCB(prev_eecb);
}

/* This is called upon entry in a parallel region.
 *   (1) I inquire OTHR for num_threads threads
 *   (2) I set up my eecb fields for my children to use
 *   (3) I create the team
 *   (4) I participate, having acquired a new eecb
 *   (5) I wait for my children to finish and resume my old eecb
 *
 * If num_threads = -1, the team will have icvs.nthreads threads.
 */
void ort_execute_parallel(void *(*func)(void *), void *shared, int num_threads,
                          int iscombined, int procbind_type)
{
	ort_eecb_t *me;
	ort_task_node_t *metask;
	int        nthr = 0;
	ort_device_t *device;
	omp_proc_bind_t proc_bind = omp_proc_bind_false;

	me = __MYCB;
	metask = __CURRTASK(me);
	device = ort_get_device(metask->icvs.def_device);


	/*
	 * First determine how many threads will be created
	 */

	if (num_threads <= 0)               /* No num_threads() clause */
		num_threads = metask->icvs.nthreads;

	if (num_threads > 1 && ((ort->icvs.levellimit == -1) ||
	                        ort->icvs.levellimit > me->activelevel))
	{
		if (me->activelevel == 0)               /* 1st level of parallelism */
		{
			nthr = ee_request(num_threads - 1, 1);
			if (nthr != num_threads - 1 && ! metask->icvs.dynamic)
			{
			TEAM_FAILURE:
				ort_error(3, "failed to create the requested number (%d) of threads.\n"
				          "   Try enabling dynamic adjustment using either of:\n"
				          "    >> OMP_DYNAMIC environmental variable, or\n"
				          "    >> omp_set_dynamic() call.\n", num_threads);
			}
		}
		else                              /* Nested level */
		{
			if (metask->icvs.nested &&  /* is nested parallelism enabled? */
			    ((ort->icvs.levellimit == -1) ||
			     ort->icvs.levellimit > me->activelevel))
			{
				nthr = ee_request(num_threads - 1, me->activelevel + 1);
				/* Now here we have an interpretation problem wrt the OpenMP API,
				 * in the case nthr != num_threads-1.
				 * Should we breakdown or can we ditch "icvs.nested" and create a
				 * 1 thread team?
				 * Well, we do a bit of both:
				 * - if the ee library returned 0, we do the latter with a warning.
				 * - otherwise, we do the former; it is easier since otherwise
				 *   we must re-contact the othr library to explain to her that
				 *   after all we won't need the threads we requested.
				 */
				if (nthr == 0)
					ort_warning("parallelism at level %d disabled due "
					            "to lack of threads.\n   >> Using a team of 1 thread.\n",
					            me->activelevel + 1);   /* GF */
				else
					if (nthr != num_threads - 1 && !__CURRTASK(me)->icvs.dynamic)
						goto TEAM_FAILURE;
			}
			else
				nthr = 0;      /* Only me will execute it */
		}
	}

	/*
	 * Next, initialize everything needed, create the team & participate
	 */

	prepare_master(me, nthr + 1, func, shared);

	if (nthr != 0)   /* Start the threads (except myself) */
		ee_create(nthr, me->activelevel, me, &me->ee_info);

#ifdef ORT_DEBUG
	ort_debug_thread("just created team and about to participate");
#endif

	__SETMYCB(me->me_master);        /* Change my cb */

	ort_ee_dowork(0, me);

	/*
	 * All done; destroy the team (me now points to my "parent")
	 */

	if (nthr > 0)
		ee_waitall(&me->ee_info);      /* Wait till all children finish */

	me->num_children = 0;
	__SETMYCB(me);                   /* assume my parent eecb */
}


/* Execute a function only by the running thread. This is only called
 * in the parser-generated code when checking an IF condition at a
 * parallel section.
 */
void ort_execute_serial(void *(*func)(void *), void *shared)
{
	ort_eecb_t *me = __MYCB;

	prepare_master(me, 1, func, shared);
	__SETMYCB(me->me_master);         /* Change my key */

	ort_ee_dowork(0, me);

	me->num_children = 0;
	__SETMYCB(me);                   /* assume my parent eecb */
}


void *ort_get_parent_ee_info(void)
{
	return (__MYCB->parent->ee_info);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * THREADPRIVATE STUFF                                               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static void *get_ee_thrpriv(ort_eecb_t *e, int *varid, int size, void *origvar)
{
	int  vid, thrid, nkeys;
	void **vars;
	ort_eecb_t *parent;

	if (*varid == 0)    /* This var was never used before; must get an id */
	{
		/* we use the ort->preparation_lock, so as not to define 1 more lock */

		ee_set_lock((ee_lock_t *) &ort->preparation_lock);

		if (*varid == 0)
			*varid = ++(ort->thrpriv_num);
		FENCE;
		ee_unset_lock((ee_lock_t *) &ort->preparation_lock);
	}

	vid = *varid;

	/* For the initial thread, tpvars are stored in its 0-th child space */
	parent = (e->level > 0) ? e->parent : e;
	nkeys = parent->tpkeys[thrid = e->thread_num].alloted;
	vars = parent->tpkeys[thrid].vars;
	if (vid >= nkeys)
	{
		vars = (vars == NULL) ? ort_alloc((vid + 10) * sizeof(void *)) :
		       ort_realloc(vars, (vid + 10) * sizeof(void *));
		if (vars == NULL)
			ort_error(1, "[ort_get_thrpriv]: memory allocation failed\n");
		memset(&vars[nkeys], 0, (vid + 10 - nkeys)*sizeof(void *));
		parent->tpkeys[thrid].alloted = nkeys = vid + 10;
		parent->tpkeys[thrid].vars = vars;
	}

	if (vars[vid] == NULL)
	{
		if (thrid == 0)
		{
			if (e->level > 0)  /* master thread; get the parent's var */
				vars[vid] = get_ee_thrpriv(e->parent, varid, size, origvar);
			else               /* initial thread; references origvar */
			{
				/* was: vars[vid] = origvar; */
				if ((vars[vid] = ort_alloc(size)) == NULL)
					ort_error(1, "[ort_get_thrpriv]: out of initial thread memory\n");
				memcpy(vars[vid], origvar, size);   /* initialize */
			}
		}
		else
		{
			if ((vars[vid] = ort_alloc(size)) == NULL)
				ort_error(1, "[ort_get_thrpriv]: out of memory\n");
			memcpy(vars[vid], origvar, size);   /* initialize */
		}
	}
	return (vars[vid]);
}


void *ort_get_thrpriv(void **key, int size, void *origvar)
{
	return (get_ee_thrpriv(__MYCB, (int *) key, size, origvar));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * ATOMIC, CRITICAL, REDUCTION AND COPYPRIVATE SUPPORT               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* The binding thread set of an atomic region is the set of all threads
 * in the program, not just the threads of the current team (OpenMP V2.5).
 * Thus ort->atomic_lock is global.
 */
void ort_atomic_begin()
{
	ee_set_lock((ee_lock_t *) &ort->atomic_lock);    /* ##### */
}


void ort_atomic_end()
{
	ee_unset_lock((ee_lock_t *) &ort->atomic_lock);    /* ##### */
}


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


void ort_reduction_begin(omp_lock_t *redlock)
{
	/* Because OMPi's parser declares all reduction locks as globals,
	 * they are initialized to NULL, thus we can safely differentiate
	 * between uninitialized and initialized ones.
	 */
	if (*redlock == NULL) ort_prepare_omp_lock(redlock, ORT_LOCK_SPIN);

#if defined(EE_TYPE_PROCESS_MPI)
	ee_set_lock((ee_lock_t *) redlock);
#else
	ee_set_lock((ee_lock_t *) *redlock);
#endif
}


void ort_reduction_end(omp_lock_t *redlock)
{
#if defined(EE_TYPE_PROCESS_MPI)
	ee_unset_lock((ee_lock_t *) redlock);
#else
	ee_unset_lock((ee_lock_t *) *redlock);
#endif
}


/* The SINGLE onwer initialization for copyprivate data.
 * It creates an array of pointers to its private data.
 */
void ort_broadcast_private(int num, ...)
{
	va_list     ap;
	ort_eecb_t  *me;
	ort_cpriv_t *cp;
	int         i;

	if ((me = __MYCB)->num_siblings == 1)  /* Nothing here if I am solo */
		return;

	cp = &(me->parent->copyprivate);
	cp->owner   = me->thread_num;
	cp->copiers = me->num_siblings;
	cp->data    = (volatile void **) malloc(num * sizeof(void *));

	va_start(ap, num);
	for (i = 0; i < num; i++)
		cp->data[i] = va_arg(ap, void *);
	va_end(ap);
}


/* All threads copy copyprivate date from the SINGLE owner.
 * The arguments are pointer-size pairs.
 */
void ort_copy_private(int num, ...)
{
	va_list     ap;
	int         i;
	void        **from, *arg;
	ort_cpriv_t *cp;
	ort_eecb_t  *me;

	if ((me = __MYCB)->num_siblings == 1)  /* Nothing here if I am solo */
		return;

	cp = &(me->parent->copyprivate);
	if (cp->owner != me->thread_num)   /* I am not the owner */
	{
		va_start(ap, num);
		from = (void **) cp->data;
		for (i = 0; i < num; i++)
		{
			arg = va_arg(ap, void *);
			memcpy(arg, from[i], va_arg(ap, int));
		}
		va_end(ap);
	}

#if defined(HAVE_ATOMIC_FAA) && !defined(EE_TYPE_PROCESS)
	i = _faa(&(cp->copiers), -1) - 1;
#else
	ee_set_lock(&cp->lock);
	i = --cp->copiers;
	ee_unset_lock(&cp->lock);
#endif

	if (i == 0)
		free(cp->data);                    /* Free allocated data */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * Interoperability;                                                 *
 *    an external calling thread may become an OpenMP master.        *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Internal funuction to force an arbitrary eecb; if a new eecb is
 * returned, IT MUST NOT BE SIMPLY free()ed due to its aligned allocation.
 */
void *ort_prepare_my_eecb(void *eecb)
{
	if (eecb == NULL)
		eecb = ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
	initialize_eecb((ort_eecb_t *) eecb);
	__SETMYCB((ort_eecb_t *) eecb);
	return (eecb);
}


/* Function for enabling thread cancellation in a parallel region
 */
void ort_enable_cancel_parallel(void)
{
	ort_eecb_t *me = __MYCB;

	if (omp_get_cancellation() == ENV_CANCEL_ACTIVATED)
		if (me->parent != NULL)
			me->parent->cancel_parallel = CANCEL_ACTIVATED;
}


/* Function for enabling thread cancellation in section region
 */
void ort_enable_cancel_sections(void)
{
	ort_eecb_t *me = __MYCB;

	if (omp_get_cancellation() == ENV_CANCEL_ACTIVATED)
		if (me->parent != NULL)
			me->parent->cancel_sections = CANCEL_ACTIVATED;
		else
			me->cancel_sections_me = CANCEL_ACTIVATED;
}

/* Function for enabling thread cancellation in for region
 */
void ort_enable_cancel_for(void)
{
	ort_eecb_t  *me = __MYCB;
	if (omp_get_cancellation() == ENV_CANCEL_ACTIVATED)
	{
		if (me->parent != NULL)
			me->parent->cancel_for = CANCEL_ACTIVATED;
		else
			me->cancel_for_me = CANCEL_ACTIVATED;

	}
}


/* Function for enabling thread cancellation in a taskgroup region
 */
void ort_enable_cancel_taskgroup(void)
{
	ort_eecb_t  *me = __MYCB;
	if (__CURRTASK(me)->taskgroup != NULL &&
		omp_get_cancellation() == ENV_CANCEL_ACTIVATED)
		__CURRTASK(me)->taskgroup->is_canceled = CANCEL_ACTIVATED;
}


/* This function is called when #pragma omp cancel type is reached
 */
int ort_enable_cancel(int type)
{
	ort_eecb_t *me = __MYCB;

	switch (type)
	{
		case 0:
			ort_enable_cancel_parallel();
			return ort_check_cancel_parallel();
		case 1:
			ort_enable_cancel_taskgroup();
			return ort_check_cancel_taskgroup();
		case 2:
			ort_enable_cancel_for();
			if (me->parent == NULL)
			{
				if (ort_check_cancel_for() == CANCEL_ACTIVATED)
				{
					me->cancel_for_me = 0;
					return 1;
				}
				else
					return 0;
			}
			else
			{
				if (ort_check_cancel_for() == CANCEL_ACTIVATED)
					return 1;
				else
					return 0;
			}
		case 3:
			if (me->parent == NULL)
			{
				ort_enable_cancel_sections();
				if (ort_check_cancel_sections())
				{
					me->cancel_sections_me = 0;
					return 1;
				}
				else
					return 0;
			}
			else
			{
				ort_enable_cancel_sections();
				if (ort_check_cancel_sections())
					return 1;
				else

					return 0;
			}
	}
}


/* This function is called when cancellation point is reached
 */
int ort_check_cancel(int type)
{
	switch (type)
	{
		case 0:
			return ort_check_cancel_parallel();
		case 1:
			return ort_check_cancel_taskgroup();
		case 2:
			return ort_check_cancel_for();
		case 3:
			return ort_check_cancel_sections();
	}
	return 0;
}


/* Check for active cancellation in for region
 */
int ort_check_cancel_for(void)
{
	ort_eecb_t *me = __MYCB;
	if (me->parent == NULL)
	{
		if (me->cancel_for_me == CANCEL_ACTIVATED)
			return 1;
	}
	else
		if (me->parent != NULL)
		{
			if (me->parent->cancel_for == CANCEL_ACTIVATED)
				return 1;
		}

	return 0;
}


/* Check for active cancellation in a parallel region
 */
int ort_check_cancel_parallel(void)
{
	ort_eecb_t *me = __MYCB;
	if (me->parent->cancel_parallel == CANCEL_ACTIVATED)
		return 1;

	return 0;
}


/* Check for active cancellation in sections region
 */
int ort_check_cancel_sections(void)
{
	ort_eecb_t *me = __MYCB;

	if (me->parent != NULL)
	{
		if (me->parent->cancel_sections == CANCEL_ACTIVATED)
			return 1;
	}
	else
	{
		if (me->cancel_sections_me == CANCEL_ACTIVATED)
			return 1;
	}
	return 0;
}


/* Check for active cancellation  in a taskgroup region
 */
int ort_check_cancel_taskgroup(void)
{
	ort_eecb_t *me = __MYCB;
	if (__CURRTASK(me)->taskgroup != NULL &&
		__CURRTASK(me)->taskgroup->is_canceled == CANCEL_ACTIVATED)
		return 1;
	else
		return 0;
}


/* Any user thread which is not an openmp thread may become an independent
 * OpenMP master. Repeated calls just reset the eecb.
 */
int ompi_makeme_openmp_master()
{
	ort_eecb_t *me = __MYCB;

	if (me == NULL)
		ort_prepare_my_eecb(NULL);
	else
	{
		if (me->level)
			return (-1);       /* Error; called from a non-master openmp thread */
		initialize_eecb(me); /* Reset */
	}
	return (0);
}


/* Allocate, initialize and return an eecb structure
 * (psthreads/task optimization)
 */
void *ort_alloc_eecb(ort_eecb_t *eecb, int thrid, void *parent_info)
{
	ort_eecb_t *parent = (ort_eecb_t *) parent_info;

	if (eecb == NULL)
		eecb = ort_calloc_aligned(sizeof(ort_eecb_t), NULL);

	eecb->parent            = parent;
	eecb->sdn               = parent;
	eecb->num_children      = 0;
	eecb->num_siblings      = parent->num_children;
	eecb->thread_num        = thrid;               /* Thread id within the team */
	eecb->level             = parent->level + 1       ;       /* 1 level deeper */
	eecb->activelevel       = parent->activelevel +  /* OpenMP 3 - team of 1 is */
	                          ((eecb->num_siblings > 1) ? 1 : 0); /* NOT parallel */
	eecb->shared_data       = 0;
	eecb->mynextNWregion    = 0;
	eecb->have_created_team = 0;
	eecb->ee_info           = 0;           /* not needed actually due to calloc */

	return (void *) eecb;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * UTILITY FUNCTIONS                                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ort_error(int exitcode, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "[ORT error]: ");
	vfprintf(stderr, format, ap);
	va_end(ap);

	exit(exitcode);
}


void ort_warning(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[ORT warning]: ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}


void *ort_alloc(int size)
{
	void *a;

	if ((a = malloc(size)) == NULL)
		ort_error(1, "[ort_alloc]: memory allocation failed\n");
	return (a);
}


void *ort_calloc(int size)
{
	void *a;

	if ((a = calloc(1, size)) == NULL)
		ort_error(1, "memory allocation failed\n");
	return (a);
}


void *ort_realloc(void *original, int size)
{
	void *a;

	if ((a = realloc(original, size)) == NULL)
		ort_error(1, "[ort_realloc]: memory reallocation failed\n");
	return (a);
}


void *ort_alloc_aligned(int size, void **actual)
{
	if (actual == NULL)
	{
		void *tmp;

		tmp = ort_alloc(size + CACHE_LINE - 1);
		return ((void *)((((ptrint)(tmp)) + CACHE_LINE - 1) &
		                 ((ptrint)(-CACHE_LINE))));
	}
	*actual = ort_alloc(size + CACHE_LINE - 1);
	return ((void *)((((ptrint)(*actual)) + CACHE_LINE - 1) &
	                 ((ptrint)(-CACHE_LINE))));
}


void *ort_calloc_aligned(int size, void **actual)
{
	if (actual == NULL)
	{
		void *tmp;

		tmp = ort_calloc(size + CACHE_LINE - 1);
		return ((void *)((((ptrint)(tmp)) + CACHE_LINE - 1) &
		                 ((ptrint)(-CACHE_LINE))));
	}
	*actual = ort_calloc(size + CACHE_LINE - 1);
	return ((void *)((((ptrint)(*actual)) + CACHE_LINE - 1) &
	                 ((ptrint)(-CACHE_LINE))));
}


void *ort_realloc_aligned(int size, void **actual)
{
	if ((*actual = realloc(*actual, size + CACHE_LINE - 1)) == NULL)
		ort_error(1, "memory reallocation failed\n");
	return ((void *)((((ptrint)(*actual)) + CACHE_LINE - 1) &
	                 ((ptrint)(-CACHE_LINE))));
}


#if defined(EE_TYPE_PROCESS)
void ort_shmalloc(void **p, int size, int *upd)
{
	ee_shmalloc(p, size, upd);

	if (!(*p))
		ort_error(1, "shmalloc failed\n");
}

void ort_shmfree(void *p)
{
	ee_shmfree(p);
}
#endif


/* This is only called from parser-generate code. */
void ort_fence(void)
{
	FENCE;
}


/* User-program (omp) locks are all defined as void *, including
 * the parser-generated locks for critical and reduction directives.
 * Upon initialization of such a lock, an actual othr lock is
 * allocated and initialized, through the following function.
 */

/* Allocate & initialize a user lock safely
 */
void ort_prepare_omp_lock(omp_lock_t *lock, int type)
{
	void *new;

	ee_set_lock((ee_lock_t *) &ort->preparation_lock);

	if (*lock == NULL)
	{
		/* The problem we have here is with non-global user locks.
		 * Those are not initialized necessarily to NULL, thus
		 * we cannot know if a lock is already initialized or
		 * not. We must assume it is not initialized. Otherwise,
		 * if many threads try to initialize it, we may end up
		 * with dangling malloc()s. Of course, the programmer
		 * who lets each thread initialize the same user lock,
		 * is a bad programmer.
		 */
#if defined(EE_TYPE_PROCESS)
		*lock = ee_init_lock((ee_lock_t *) - 1, type);
		FENCE;
#else
		new = ort_alloc(sizeof(ee_lock_t));
		ee_init_lock((ee_lock_t *) new, type);
		FENCE; /* 100% initialized, before been assigned to "lock" */
		*lock = new;
#endif
	}
	ee_unset_lock((ee_lock_t *) &ort->preparation_lock);
}


/* For internal tests only */
void ort_debug_thread(char *fmt, ...)
{
	va_list ap;
	static ee_lock_t *l;
	ort_eecb_t *t = __MYCB;

#define indent() { int i; for (i = 0; i < t->level; i++) fputs("   ", stderr); }
	if (l == NULL)
	{ l = malloc(sizeof(ee_lock_t)); ee_init_lock(l, ORT_LOCK_NORMAL); }

	ee_set_lock(l);

	if (t == NULL)
	{
		va_start(ap, fmt);
		if (fmt)
		{
			fprintf(stderr, "(  *** uninitialized thread ***\n");
			fprintf(stderr, "  MESSAGE:\n");
			fprintf(stderr, "    "); vfprintf(stderr, fmt, ap);
			fprintf(stderr, "\n)\n");
		}
		va_end(ap);
		ee_unset_lock(l);
		return;
	}
	indent(); fprintf(stderr, "( ::%ld::\n", (long int) t);
	indent(); fprintf(stderr, "   |       id = %d\n", t->thread_num);
	indent(); fprintf(stderr, "   |    level = %d\n", t->level);
	indent(); fprintf(stderr, "   | teamsize = %d\n", t->num_siblings);
	indent(); fprintf(stderr, "   |  ee_info = %ld\n", (long int) t->ee_info);
	indent(); fprintf(stderr, "   |   parent = %ld\n", (long int) t->parent);

	va_start(ap, fmt);
	if (fmt)
	{
		fprintf(stderr, "\n");
		indent(); fprintf(stderr, "  MESSAGE:\n");
		indent(); fprintf(stderr, "    "); vfprintf(stderr, fmt, ap);
		indent(); fprintf(stderr, "\n");
	}
	va_end(ap);

	indent(); fprintf(stderr, ")\n");
	ee_unset_lock(l);
#undef indent
}
