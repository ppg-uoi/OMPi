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

/* ort.c -- OMPi RunTime library */

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
		(eecb)->mf                  = NULL;\
		(eecb)->parent              = NULL;\
		(eecb)->sdn                 = (eecb);\
		(eecb)->num_siblings        = 1;     /* We are just 1 thread! */\
		(eecb)->thread_num          = 0;\
		(eecb)->level               = 0;     /* The only one in level 0 */\
		(eecb)->activelevel         = 0;     /* The only one in level 0 */\
		(eecb)->shared_data         = 0;\
		(eecb)->mynextNWregion      = 0;\
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


typedef struct initlist_s {
	void (*func)(void);
	struct initlist_s *next;
} initlist_t;
static initlist_t *ort_initreqs;   /* The list of auto-initializers */


/* Call all initreqs functions */
static void initreqs_do()
{
	initlist_t *req;

	for (req = ort_initreqs; req != NULL; )
	{
		(req->func)();
		req = (ort_initreqs = req)->next;
		free(ort_initreqs);
	}
	ort_initreqs = NULL;
}


/* Add a function that will be called by ort_initialize during startup.
 * It is guaranteed by the parser that this is called *before* main() starts.
 */
void ort_initreqs_add(void (*initfunc)(void))
{
	initlist_t *req = (initlist_t *) ort_calloc(sizeof(initlist_t));
	req->func = initfunc;
	req->next = ort_initreqs;
	ort_initreqs = req;
}


/*
 * Last function called (just before exit).
 * "exitval" is what the original (user) main() returned.
 */
void ort_finalize(int exitval)
{
	ort_finalize_devices();
	ee_finalize(exitval);
	free_kerneltable();
	fflush(stderr);       /* Because _exit may not flush stdio buffers */
	fflush(stdout);
	_exit(exitval);             /* Make sure nothing else is called */
}


/* Called only if the user calls exit() */
void _at_exit_call(void)
{
	ee_finalize(-1);
}


static void initialize_threading(void)
{
	ort_eecb_t *initial_eecb;
	ort_task_node_t *initial_task;
	void league_initial();

	if (ort->icvs.nthreads > 0) ort->icvs.nthreads--;  /* 1- for eelib */
	if (ee_initialize(ort->argc, ort->argv, &ort->icvs, &ort->eecaps) != 0)
		ort_error(1, "cannot initialize the thread library.\n");

	if (__MYPID != 0)               /* Nobody in case of threads */
		return;
	
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
	check_nested_dynamic(ort->icvs.nested, ort->icvs.dynamic); /* is eelib ok? */

	/* OpenMP 3.0 stuff */
	if (ort->eecaps.max_levels_supported != -1 &&
	    ort->eecaps.max_levels_supported < ort->icvs.levellimit)
		ort->icvs.levellimit = ort->eecaps.max_levels_supported;

	if (ort->eecaps.max_threads_supported != -1 &&
	    ort->eecaps.max_threads_supported < ort->icvs.threadlimit)
		ort->icvs.threadlimit = ort->eecaps.max_threads_supported;

	/* Initialize the 3 locks we need */
	ee_init_lock((ee_lock_t *) &ort->atomic_lock, ORT_LOCK_SPIN);
	ee_init_lock((ee_lock_t *) &ort->preparation_lock, ORT_LOCK_NORMAL);
	ee_init_lock((ee_lock_t *) &ort->eecb_rec_lock, ORT_LOCK_NORMAL);

	/* Recycle bin of eecbs is empty */
	ort->eecb_recycler = NULL;

	ort->thrpriv_num = 0;

	/* The initial thread */
	initial_eecb = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
	initialize_eecb(initial_eecb);
	initial_eecb->mf = (ort_mcbf_t *) mcbf_alloc();
	/* At least 1 row is needed for initial thread's threadprivate vars */
	initial_eecb->mf->tpkeys = ort_calloc(ort->icvs.ncpus * sizeof(ort_tptable_t));
	initial_eecb->mf->tpksize = ort->icvs.ncpus;

	/* If binding is enabled, bind initial thread to the first place of list */
	if (ort->icvs.proc_bind != omp_proc_bind_false && 
	    ort->eecaps.supports_proc_binding)
		initial_eecb->currplace = ee_bindme(ort->place_partition, 0);
	/* The place partition of the initial thread is the whole place list */
	initial_eecb->pfrom = 0;
	initial_eecb->pto   = numplaces(ort->place_partition) - 1;
	
	/* 
	 * Tasking
	 */
	
	/* The master's impicit task ("initial" task) */
	initial_task = (ort_task_node_t *) ort_calloc(sizeof(ort_task_node_t));
	initial_task->rtid             = -1;
	initial_task->icvs.dynamic     = ort->icvs.dynamic;
	initial_task->icvs.nested      = ort->icvs.nested;
	initial_task->icvs.rtschedule  = ort->icvs.rtschedule;
	initial_task->icvs.rtchunk     = ort->icvs.rtchunk;
	initial_task->icvs.nthreads    = ort->icvs.nthreads;
	/* OpenMP 4.0 */
	initial_task->icvs.def_device  = ort->icvs.def_device; /* OpenMP 4.0 */
	initial_task->icvs.threadlimit = ort->icvs.threadlimit;
	initial_task->icvs.proc_bind   = ort->icvs.proc_bind;
	initial_task->taskgroup        = 0;
	initial_task->icvs.cur_de      = NULL;

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
	
	league_initial();             /* This is the very initial league */
	ort->initleague_cgsize = 1;   /* It has 1 thread */
	atexit(_at_exit_call); /* Upon exit .. */
}


/* This is the ORT initialization part that comes after modules discovery.
 * It is a seperate function because some esoteric modules may block during 
 * their discovery/initialization; just before blocking they have a chance 
 * to complete ORT initialization by calling this function explicitly.
 */
void ort_init_after_modules()
{
	/* The following need to know the number of discovered devices */
	ort_decltarg_initialize();       /* Initialize declared variables structure */
	ort_get_environment();                       /* Get environmental variables */

	/* Initialize OMPi's taskqueuesize, if requested */
	if (ort->dynamic_taskqueuesize)
		ort->taskqueuesize = 3 * (ort->icvs.ncpus);

	/* Initialize the execution entities */
	initialize_threading();

#if defined(EE_TYPE_PROCESS)
	ort_share_globals();
#endif

	initreqs_do();
}


/*
 * First function called.
 * The embedmode flag was added for special cases, e.g. when ORT is "embedded"
 * within another ORT (e.g. in the proc device module). There is nothing
 * special about it though. Basically used for suppressing multiple info
 * message printouts.
 */
int ort_initialize(int *argc, char ***argv, int embedmode, int nmodules, ...)
{
	va_list ap;

	if (ort_initialized) return 0;

	/* Some defaults */
	ort = &ort_globals;
	ort->icvs.ncpus           = ort_get_num_procs(); /* Default ICV values */
	ort->icvs.stacksize       = -1;                  /* OpenMP 3.0 */
	ort->icvs.threadlimit     = 1 << 30;             /* (unlimited) OpenMP 3.0 */
	ort->icvs.levellimit      = 1 << 30;             /* (unlimited) OpenMP 3.0 */
	ort->icvs.waitpolicy      = _OMP_ACTIVE;         /* OpenMP 3.0 */
	ort->icvs.nthreads        = -1;        /* per-task; no preference for now */
	ort->icvs.rtschedule      = omp_sched_auto;
	ort->icvs.rtchunk         = 0;
	ort->icvs.dynamic         = 1;
	ort->icvs.nested          = 0;
	ort->icvs.proc_bind       = omp_proc_bind_false; /* OpenMP 4.0 */
	ort->icvs.cancel          = 0;                   /* OpenMP 4.0 */
	ort->icvs.def_device      = 0; /* This is the HOST  OpenMP 4.0 */
	ort->place_partition      = NULL;                /* OpenMP 4.0 */
	/* Call of ort_get_default_places() was moved to ort_get_environment() in
	 * order to avoid a redundant (second) topology detection.
	 */
	ort->num_devices          = 0;                   /* OpenMP 4.0 */
	ort->icvs.max_task_prio   = 0;                   /* OpenMP 4.5 */
	ort->icvs.display_affinity = 0;                  /* OpenMP 5.0 */
	ort->icvs.affinity_format = ort_get_default_affinity_format();/* OpenMP 5.0 */
	ort->icvs.targetoffload   = OFFLOAD_DEFAULT;     /* OpenMP 5.0 */
	ort->module_host.sharedspace = 1;
	ort->embedmode            = embedmode;
	ort->argc                 = argc;
	ort->argv                 = argv;

	/* Initialize modules and device structures */
	va_start(ap, nmodules);
	ort_discover_modules(nmodules, ap);      /* Host is always added as dev 0 */
	va_end(ap);
	if (ort->num_devices > 1)
		ort->icvs.def_device = 1;              /* The first available device. */
	
	/* Initializations that come after module discovery */
	ort_init_after_modules();
	
	if (ort->icvs.targetoffload == OFFLOAD_DISABLED)
		ort->num_devices = ort->icvs.def_device = 0;     /* ditch all devices */

	if (display_env && !embedmode)            /* If asked, show environment */
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

	/* Memory layout :
	 * a) shared globals (sgl), b) ort_globals, c) master_eecb, d) master_task
	 */
	ort_shmalloc(&ort_sglvar_area, list_size +
	               sizeof(ort_globals)+sizeof(ort_eecb_t)+sizeof(ort_task_node_t),
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
 * TEAMS                                                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Setup the very initial league */
void league_initial()
{
	ort->league.numteams = 1;
	ort->league.threadlimit = ort->icvs.threadlimit;
	ort->league.cg_size = &ort->initleague_cgsize;
	ort->league.cg_inithr = NULL;
}


/* Install a new league */
void league_start(int numteams, int thrlimit)
{
	int i;
	ort->league.numteams = numteams;
	ort->league.threadlimit = thrlimit;
	ort->league.cg_size = ort_alloc(numteams * sizeof(int));
	ort->league.cg_inithr = ort_alloc(numteams * sizeof(ort_eecb_t *));
	for (i = 0; i < numteams; i++)
		ort->league.cg_size[i] = 1;
}


/* Destory league and re-install the initial one */
void league_finish()
{
	if (INITLEAGUE())
		return;
	free((void *) ort->league.cg_size);
	free(ort->league.cg_inithr);
	league_initial();
}


void ort_start_teams(void *(*func)(void *), void *shared, int num_teams,
                          int thr_limit)
{
	ort_eecb_t      *me = __MYCB;
	ort_task_node_t *mytask = __CURRTASK(me);
	int             thrlim_bak;
	
	/* Here we are supposed to start a number (up to num_threads) of initial 
	 * threads, each one forming a team of up to thr_limit threads
	 */
	if (num_teams < 0)    /* No num_teams specified */
		num_teams = 1;
	if (me->level != 0 || !INITLEAGUE())
		ort_error(1, "'teams' request from an invalid region\n");
#if 1   /* trivial inmplementation */
	thrlim_bak = mytask->icvs.threadlimit;
	league_start(1, thr_limit);      /* Start a new league of 1 team */
	if (thr_limit > 0 && ort->icvs.threadlimit > thr_limit)
		mytask->icvs.threadlimit = thr_limit;   /*  a lower thread limit */
	ort_execute_serial(func, shared);   /* 1 team of 1 thread for now... */
	mytask->icvs.threadlimit = thrlim_bak;
#else
	/* Ideally we would create num_teams teams as per the user request or 
	 * maybe as many teams as the NUMA nodes.
	 * We would ee_request as normal (see parallel.c) and then ee_create;
	 * we only need to ensure a level of 0.
	 * We also need to have αν infrastructure for reductions! */
	 */
#endif
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * ATOMIC AND COPYPRIVATE SUPPORT                                    *
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

	cp = &(TEAMINFO(me)->copyprivate);
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

	cp = &(TEAMINFO(me)->copyprivate);
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


void *eecb_alloc(void)
{
	ort_eecb_t *eecb;

	ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);

	if (ort->eecb_recycler == NULL) /* If recycle bin is empty */
		eecb = ort_calloc_aligned(sizeof(ort_eecb_t), NULL); /* Allocate new eecb */
	else
		ort->eecb_recycler = (eecb = ort->eecb_recycler)->parent;

	ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
	return eecb;
}


void eecb_free(ort_eecb_t *eecb)
{
	if (!eecb->mf)
	{
		ee_barrier_destroy(&eecb->mf->barrier);
		ort_reductions_finalize(eecb);
		free(eecb->mf);
		eecb->mf = NULL;
	}

	ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);

	eecb->parent = ort->eecb_recycler;
	ort->eecb_recycler = eecb;

	ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
}


ort_task_node_t *prepare_tasknode(ort_task_node_t *t)
{
    if (t == NULL)
        t = (ort_task_node_t *) ort_calloc(sizeof(ort_task_node_t));
    
	t->rtid             = -1;
	t->icvs.dynamic     = ort->icvs.dynamic;
	t->icvs.nested      = ort->icvs.nested;
	t->icvs.rtschedule  = ort->icvs.rtschedule;
	t->icvs.rtchunk     = ort->icvs.rtchunk;
	t->icvs.nthreads    = ort->icvs.nthreads;
	/* OpenMP 4.0 */
	t->icvs.def_device  = ort->icvs.def_device; /* OpenMP 4.0 */
	t->icvs.threadlimit = ort->icvs.threadlimit;
	t->icvs.proc_bind   = ort->icvs.proc_bind;
	t->taskgroup        = 0;
	t->icvs.cur_de      = NULL;
    
    return (t);
}


void *mcbf_alloc(void)
{
	ort_mcbf_t *mf = (ort_mcbf_t *) ort_calloc_aligned(sizeof(ort_mcbf_t), NULL);
	ee_init_lock(&mf->copyprivate.lock, ORT_LOCK_SPIN);
	mf->workshare.blocking.inited = 0; // not required due to calloc
	/* VVD-new */
	mf->me_master = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
	return (void *) mf;
}


/* Any user thread which is not an openmp thread may become an independent
 * OpenMP master. Repeated calls just reset the eecb.
 */
int ompi_makeme_openmp_master()
{
	ort_eecb_t *me = __MYCB;
	ort_task_node_t *tasking;

	if (me == NULL)
	{
		me = ort_calloc_aligned(sizeof(ort_eecb_t), NULL);
		__SETMYCB(me);
	}
	else
		if (me->level)
			return (-1);       /* Error; called from a non-master openmp thread */
	initialize_eecb(me);   /* Reset */
	
	tasking = prepare_tasknode(__CURRTASK(__MYCB));
	__SETCURRTASK(__MYCB, tasking);
	__SETCURRIMPLTASK(__MYCB, tasking);

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
	eecb->num_siblings      = parent->mf->num_children;
	eecb->thread_num        = thrid;               /* Thread id within the team */
	eecb->level             = parent->level + 1       ;       /* 1 level deeper */
	eecb->activelevel       = parent->activelevel +  /* OpenMP 3 - team of 1 is */
	                          ((eecb->num_siblings > 1) ? 1 : 0); /* NOT parallel */
	eecb->shared_data       = NULL;
	eecb->mynextNWregion    = 0;
	eecb->ee_info           = NULL;        /* not needed actually due to calloc */

	return (void *) eecb;
}


/* This function is called in case of target if(false).
 * A new OpenMP thread is set up (new eecb and task node acquired).
 * ICVs are initialized from the initial ones.
 * Finally the new "thread" executes the kernel func code.
 */
void ort_execute_kernel_on_host(void *(*func)(void *), void *shared)
{
	ort_eecb_t eecb_for_dev, *prev_eecb;
	ort_task_node_t task_for_dev;

	initialize_eecb(&eecb_for_dev);
	eecb_for_dev.mf = (ort_mcbf_t *) mcbf_alloc();
	/* At least 1 row is needed for initial thread's threadprivate vars */
	eecb_for_dev.mf->tpkeys = ort_calloc(ort->icvs.ncpus * sizeof(ort_tptable_t));
	eecb_for_dev.mf->tpksize = ort->icvs.ncpus;

	/* The impicit task ("initial" task) */
	task_for_dev.icvs.dynamic     = ort->icvs.dynamic;
	task_for_dev.icvs.nested      = ort->icvs.nested;
	task_for_dev.icvs.rtschedule  = ort->icvs.rtschedule;
	task_for_dev.icvs.rtchunk     = ort->icvs.rtchunk;
	task_for_dev.icvs.nthreads    = ort->icvs.nthreads;
	/* OpenMP 4.0 */
	task_for_dev.icvs.def_device  = ort->icvs.def_device; /* OpenMP 4.0 */
	task_for_dev.icvs.threadlimit = ort->icvs.threadlimit;
	task_for_dev.icvs.proc_bind   = ort->icvs.proc_bind;
	task_for_dev.taskgroup        = 0;
	task_for_dev.icvs.cur_de      = NULL;

	/* Save old eecb */
	prev_eecb = __MYCB;
	/* Assign new eecb */
	__SETMYCB(&eecb_for_dev);

	__SETCURRTASK(&eecb_for_dev, &task_for_dev);
	__SETCURRIMPLTASK(&eecb_for_dev, &task_for_dev);

	/* Execute kernel func. */
	(*func)(shared);

	/* Restore old eecb */
	__SETMYCB(prev_eecb);
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
		ort_error(1, "[ort_alloc]: memory allocation failed for %d bytes\n", size);
	return (a);
}


/* This should allocate space globally (e.g. in a shared memory region); it
 * is basically useless:
 * a) only used by ort_prepare_omp_lock()
 * b) only used (shadowed) by the proc module
 * c) only works for GCC
 * It is only defined so that it can be shadowed by proc module's version.
 */
#ifdef __GNUC__
__attribute__ ((weak)) void *ort_alloc_global(int size);
#endif
void *ort_alloc_global(int size)
{
	return ( ort_alloc(size) );
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


/* The next two functions are for userland calls */
void *ort_memalloc(int size)
{
	return ort_alloc(size);
}


void ort_memfree(void *ptr)
{
	free(ptr);
}


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
		new = ort_alloc_global(sizeof(ee_lock_t));
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
