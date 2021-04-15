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

/* ORT_PRIVE.H
 * Definitions and types needed for building ORT.
 * This is only included by ort.c.
 */

#ifndef __ORT_PRIVE_H__
#define __ORT_PRIVE_H__

#include "ort.h"
#include "ort_defs.h"
#include <ee.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 *  CONSTANTS AND MACROS                                 *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Maximum number of concurrently active nowait workshare regions.
 * A number equal to a small multiple of the # of threads seems good,
 * although the more the better. However, few programs have many
 * nowait regions so we don't choose a huge number (so as to not occupy
 * too much space).
 */
#define _OMP_SINGLE   0         /* Workshare types */
#define _OMP_SECTIONS 1
#define _OMP_FOR      2


#if defined(EE_TYPE_PROCESS)

	#define ee_pid               oprc_pid
	#define ee_shmfree           oprc_shmfree
	#define ee_shmalloc          oprc_shmalloc

	#define ee_key_t             oprc_key_t
	#define ee_key_create        oprc_key_create
	#define ee_getspecific       oprc_getspecific
	#define ee_setspecific       oprc_setspecific

	#define ee_initialize        oprc_initialize
	#define ee_finalize          oprc_finalize
	#define ee_request           oprc_request
	#define ee_create            oprc_create
	#define ee_yield             oprc_yield
	#define ee_waitall           oprc_waitall
	#define ee_bindme            oprc_bindme
	#define ee_getselfid         oprc_getselfid
	#define ee_getself           oprc_getself

	#define ee_lock_t            oprc_lock_t
	#define ee_init_lock         oprc_init_lock
	#define ee_destroy_lock      oprc_destroy_lock
	#define ee_set_lock          oprc_set_lock
	#define ee_unset_lock        oprc_unset_lock
	#define ee_test_lock         oprc_test_lock

	#if defined(AVOID_OMPI_DEFAULT_BARRIER)
		#define ee_barrier_t         oprc_barrier_t
		#define ee_barrier_init      oprc_barrier_init
		#define ee_barrier_destroy   oprc_barrier_destroy
		#define ee_barrier_wait      oprc_barrier_wait
	#endif

	#if defined(AVOID_OMPI_DEFAULT_TASKS)
		#define ee_new_task          oprc_new_task
		#define ee_new_task_exec     oprc_new_task_exec
		#define ee_taskwait          oprc_taskwait
	#endif

#else

	#define ee_key_t             othr_key_t
	#define ee_key_create        othr_key_create
	#define ee_getspecific       othr_getspecific
	#define ee_setspecific       othr_setspecific

	#define ee_initialize         othr_initialize
	#define ee_finalize           othr_finalize
	#define ee_request            othr_request
	#define ee_create             othr_create
	#define ee_yield              othr_yield
	#define ee_waitall            othr_waitall
	#define ee_bindme             othr_bindme
	#define ee_getselfid          othr_getselfid
	#define ee_getself            othr_getself

	#define ee_lock_t            othr_lock_t
	#define ee_init_lock         othr_init_lock
	#define ee_destroy_lock      othr_destroy_lock
	#define ee_set_lock          othr_set_lock
	#define ee_unset_lock        othr_unset_lock
	#define ee_test_lock         othr_test_lock

	#if defined(AVOID_OMPI_DEFAULT_BARRIER)
		#define ee_barrier_t         othr_barrier_t
		#define ee_barrier_init      othr_barrier_init
		#define ee_barrier_destroy   othr_barrier_destroy
		#define ee_barrier_wait      othr_barrier_wait
	#endif

	#if defined(AVOID_OMPI_DEFAULT_TASKS)
		#define ee_new_task             othr_new_task
		#define ee_new_task_exec        othr_new_task_exec
		#define ee_taskwait             othr_taskwait
		#define ee_task_immediate_start othr_task_immediate_start
		#define ee_task_immediate_end   othr_task_immediate_end
		#define ee_check_throttling     othr_check_throttling
		#define ee_set_currtask         othr_set_currtask
		#define ee_get_currtask         othr_get_currtask
		#define ee_taskenv_alloc        othr_taskenv_alloc
		#define ee_taskenv_free         othr_taskenv_free
	#endif

#endif

#if !defined(AVOID_OMPI_DEFAULT_BARRIER)
	#define ee_barrier_t         ort_defbar_t
	#define ee_barrier_init      ort_default_barrier_init
	#define ee_barrier_destroy   ort_default_barrier_destroy
	#define ee_barrier_wait      ort_default_barrier_wait
#endif


/* This is for the case where some eelib supports nested parallelism
 * but requires dynamic to be turned on.
 */
#define check_nested_dynamic(n,d)\
	if ((n) && !(d) && !ort->eecaps.supports_nested_nondynamic) {\
		ort_warning("the EE library reports that nested and NOT dynamic\n"\
		            "   parallelism cannot be supported.\n"\
		            "   Try enabling dynamic adjustment using either of:\n"\
		            "    >> OMP_DYNAMIC environmental variable, or\n"\
		            "    >> omp_set_dynamic() call.\n\n"\
		            "*** disabling support for nested parallelism for now ***\n"\
		            "[end of ORT warning]\n");\
		(n) = 0;\
	}


/* Busy waiting with yield.
 * We do a FENCE in the beginning for many reasons. One is that
 * usually we wait on stuff other threads may modify. Another
 * is that at least one optimizing compiler optimizes away the
 * loop, incorrectly.
 */
#define OMPI_WAIT_WHILE(f, trials_before_yielding) { \
		volatile int time = 0; \
		for ( ; (f); time++) \
			if (time == (trials_before_yielding)) { \
				time = -1; \
				ee_yield(); \
			}; \
	}

#define testnotset(X) if((X)==0) {(X)=1; FENCE;}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 *  TYPES                                                *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * Types related to tasks
 */
#define TASKQUEUESIZE (ort->taskqueuesize)
#define DYNAMIC_TASKQUEUESIZE (ort->dynamic_taskqueuesize)
#define OMPI_STEAL_POLICY (ort->ompi_steal_policy)

#define ALLOCATE_ENV(X,Y) {\
		(X).funcarg = ort_calloc((Y)+ sizeof(void*)); \
		*((void **)((X).funcarg)) = &(X); \
	}


#define ALLOCATE_PENV(X,Y) { \
		(X)->funcarg = ort_calloc((Y)+sizeof(void*));\
		*((void **)((X)->funcarg)) = (X);\
	}

/* value of next next pointer (used for reading) */
#define NP(X) (void*)(((char*)(X)) + sizeof(void*))
/* value of previous pointer (used for reading) */
#define PP(X) (((char*)(X)) - sizeof(void*))

#define CHAR_PAD_CACHE(A) ((CACHE_LINE - (A % CACHE_LINE))/sizeof(char))

#define TASKPOOLSIZE(X) (TASKQUEUESIZE+(X)+3)
#define atomic_read(X) *((int*)X)


#if !defined(AVOID_OMPI_DEFAULT_TASKS)

struct half_node
{
	void            *(*func)(void *);    /* Task function */
	struct Node     *parent;             /* Task's parent id */
	volatile struct Node *next;          /* For use in garbage collector */
	int             isfinal;             /* OpenMP 3.1 */
	int             taskgroup;           /* OpenMP 4.0 */
#if !defined(HAVE_ATOMIC_FAA)
	ee_lock_t     lock;                /* Lock queue for atomic access */
#endif
};

typedef struct taskgroup_s
{
	struct taskgroup_s *next;            /* For use in garbage collector */
	struct taskgroup_s *parent;          /* Taskgroup's parent */
	volatile int       is_canceled;      /* Cancel flag for taskgroup */
} taskgroup_t;


typedef struct Node
{
	void            *(*func)(void *);    /* Task function */
	struct Node     *parent;             /* Task's parent id */
	struct Node     *next;               /* For use in garbage collector */
	int              isfinal;            /* OpenMP 3.1 */
	taskgroup_t     *taskgroup;          /* OpenMP 4.0 */
#if !defined(HAVE_ATOMIC_FAA)
	ee_lock_t        lock;               /* Lock queue for atomic access */
#endif

	/* Padding here... */
	char             pad[CHAR_PAD_CACHE(sizeof(struct half_node))];
	void            *funcarg;        /* Task function argument */
	volatile int     num_children;   /* Number of task's descendants*/
	/* Check out whether i inherited task node from my father */
	int              inherit_task_node;
	volatile int     occupied;
	int              rtid;           /* Special task id (nested loop to task) */
	ort_task_icvs_t  icvs;           /* OpenMP3.0 */

	void            *dependencies;   /* OpenMP 4.0 task dependencies (opaque) */
} ort_task_node_t;


/* Task node recycle bin */
typedef struct task_node_pool ort_task_node_pool_t;
struct task_node_pool
{
	void               *(*task_func)(void *);
	int                   capacity;
	ort_task_node_t      *sub_pool;
	ort_task_node_t      *recycler;
	ort_task_node_pool_t *next;
};


typedef struct Queue
{
	volatile int top;     /* Must be volatile in order to read it atomicaly */
	volatile int bottom;
	ort_task_node_t **tasks;
#if !defined(HAVE_ATOMIC_FAA) || !defined(HAVE_ATOMIC_CAS)
	ee_lock_t lock;            /* Lock queue for atomic access */
#endif

	/* Pointers to task-counters of my children implicit task */
	volatile int *implicit_task_children;
} ort_task_queue_t;

/* Hold data for task implementation */
typedef struct
{
	/* Thread's private data */
	/* Have to know what task i currently execute. Needed in task wait */
	ort_task_node_t *current_executing_task;
	/* Common data between me and rest threads of the group (my children).*/
	/* Obviusly used when i become father */
	/* table that holds task queues of all threads in my group */
	ort_task_queue_t *queue_table;
	/* Task environment pool */
	ort_task_node_pool_t *task_node_pool;
	/* Maximum number of mates in my team */
	int max_mates;
	/* Maximum number of children that i have created */
	int max_children;
	/* If tasks are left to be done from the members of my group */
	volatile int never_task;
	/* OpenMP 4.0 Used to identify closely nested implicit task */
	ort_task_node_t *current_implicit_task;
#ifdef ORT_DEBUG
	long tasks_enqueued;
	long tasks_executed_by_worker;
	long tasks_executed_by_thief;
	long immediate_execution_due_to_full_pool;
	long immediate_execution_due_to_full_task_queue;
	long immediate_execution_due_to_new_task_exec;
	long throttled;             /* Executed immediately */
	long throttled_pool;        /*    .. due to full pool */
	long throttled_queue;       /*    .. due to full task queue */
	long throttled_if;          /*    .. due to if(FALSE) clause */
	long throttled_final;       /*    .. due to final(TRUE) clause */
	long throttled_serial;      /*    .. outside of parallel */
	long in_throttle;           /* # times i got in throttling mode */
	long out_throttle;          /* # times i got out of throttling */
	long fail_theft_attemts;    /* # times tried to steal from empty queue */
#endif
} ort_tasking_t;

#else

/* Hold data for task implementation */
typedef struct
{
	/* Thread's private data */
	/* Have to know what task i currently execute. Needed in task wait */
	ort_task_node_t *current_executing_task;
} ort_tasking_t;

#endif


/*
 * Other types
 */


/* Doacross loops configuration:
 * !DOACROSS_FAST: lowest memory requirements, only works for static schedules
 *  DOACROSS_FAST: fastest, works for all schedules, high memory consumption
 */
#define DOACROSS_FAST

/* For FOR loops */
typedef struct
{
	int  hasordered;                /* flag for plain ordered clause */                  
	int  ordnum;                    /* ordered(num) - only used for doacross */
	int  colnum;                    /* collapse(num) - ditto */
	int  niters;                    /* total # iterations (after collapsing) */
	volatile u_long iter;           /* The next iteration to be scheduled */
	volatile u_long *curriter;      /* Current iteration for each child */
#ifdef DOACROSS_FAST
	volatile unsigned int *mapit;   /* Doacross iteration bitmap */
#else
	int  schedtype;
	int  chsize;
#endif
} ort_forloop_t;


/* For workshare regions */
typedef struct
{
	ee_lock_t    reglock;                         /* Lock for the region */
	volatile int empty;                 /* True if no thread entered yet */
	volatile int left;            /* # threads that have left the region */
	int          inited;              /* 1 if the region was initialized */

	/* SECTIONS & FOR specific data */
	volatile int  sectionsleft; /* Remaining # sections to be given away */
	ort_forloop_t forloop;      /* Stuff for FOR regions */
} wsregion_t;


/* A table of simultaneously active workshare regions */
typedef struct
{
	/* This is for BLOCKING (i.e. with no NOWAIT clause) regions. */
	wsregion_t blocking;
	/* This is for keeping track of active NOWAIT regions.  */
	volatile int headregion, tailregion;   /* volatile since all threads watch */
	wsregion_t active[MAX_ACTIVE_REGIONS];
} ort_workshare_t;


/* Holds pointers to copyprivate vars. */
typedef struct
{
	volatile void  **data;
	int            owner;
	int            copiers;
	ee_lock_t      lock;
} ort_cpriv_t;


/* Holds the key-value pairs for threadprivate variables */
typedef struct
{
	int  alloted;    /* size of vars table */
	void **vars;
} ort_tptable_t;


/* Master Control Block Fields (MCBF).
 * Contains the fields used when acting as a master thread.
 * These fields were previously in ort_eecb_t.
 */
typedef struct ort_eecb_s ort_eecb_t;
typedef struct ort_mcbf_s ort_mcbf_t;
struct ort_mcbf_s
{
	/*
	 * The barrier is declared first, hoping it will be cache aligned
	 */

	/* First, the fields used by my children (when I am the parent)
	 */
	ee_barrier_t    *barrier;                        /* Barrier for my children */
	void            *redinfo;              /* Reduction data/results of my kids */
	int             num_children;
	void            *(*workfunc)(void *);   /* The func executed by my children */
	ort_workshare_t workshare;     /* Some fields volatile since children snoop */
	ort_cpriv_t     copyprivate;  /* For copyprivate; owner stores data here and
                                   the rest of the children grab it from here */
	ort_tptable_t   *tpkeys;               /* Threadprivate vars of my children */
	int             tpksize;         /* in essence, max # children ever created */
	ort_eecb_t      *me_master;      /* For use when i become master of a group */
	volatile int    cancel_par_active;           /* Flags to signal cancelation */
	volatile int    cancel_sec_active;
	volatile int    cancel_for_active;

	omp_proc_bind_t bind_override;    /* _false when no proc_bind clause exists */
};


/* Execution entity control block (eecb).
 * ORT keeps such a block for every ee; it contains fields necessary
 * for runtime bookkeeping.
 * The eecb's form a tree, where child ee's have pointers to their
 * parent's eecb.
 */
struct ort_eecb_s
{
	/* First, the fields used by my children (when I am the parent)
	 */
	ort_mcbf_t *mf;

	/* Fields for me, as a member of a team
	 */
	ort_eecb_t *parent;          /* this is also used for recycling freed EECBs */

	int cancel_sec_me;                         /* Cancel flags in serial region */
	int cancel_for_me;
	int thread_num;                                /* Thread id within the team */
	int num_siblings;                                   /* # threads in my team */
	int level;                            /* At what level of parallelism I lie */
	int activelevel;             /* At what *active* level of parallelism I lie */
	void *shared_data;          /* Pointer to shared struct of current function */
	ort_eecb_t *sdn;            /* Where I will get shared data from; normally
                                   from my parent, except at a false parallel
                                   where I get it from myself since I am
                                   the only thread to execute the region. */
	ort_eecb_t *me_partask;              /* When i run a task from parallel for */
	int mynextNWregion;        /* Non-volatile; I'm the only thread to use this */
	int nowaitregion;             /*  True if my current region is a NOWAIT one */
	int currplace; /* The place I'm currently executing (place_partition index) */
	int pfrom, pto;                      /* Current place subpartition interval */
	int cgid;                       /* The contention group I belong to (>= -1) */

	taskgroup_t *tg_recycler;                         /* Taskgroup recycler */
	/* Tasking structures
	 */
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_tasking_t tasking;
#endif
	/* Thread-library specific data
	 */
	void *ee_info;                                 /* Handled by the ee library */
};


/* List holding pointers to user's shared global variables (only if ee=proc) */
typedef struct ort_sglvar_s ort_sglvar_t;
struct ort_sglvar_s
{
	void         **varptr;      /* Pointer to user's global variable */
	int          size;          /* sizeof(var) */
	void         *initvalue;    /* Initializer */
	ort_sglvar_t *next;
};

#define FIFO        1
#define LIFO        0

#define FALSE       0
#define TRUE        1
#define AUTO        2


/* League of teams (OpenMP 4.5/5.0) */
typedef struct 
{
	int            numteams;    /* # initial teams participating in the league */
	int            threadlimit; /* max # threads per contention group */
	int volatile  *cg_size;     /* # active threads per contention group */
	ort_eecb_t   **cg_inithr;   /* the initial threads of the contention groups */
} league_t;                 /* NULL cg_inithr signifies the very initial team */


/* All global variables ORT handles; if ee=proc, this is also placed
 * in shared memory.
 */
typedef struct
{
	ort_icvs_t         icvs;               /* Global ICVs */
	ort_caps_t         eecaps;             /* Capabilities of the EELIB */
	int                embedmode;          /* To suppress some printouts */
	
	ort_module_t       module_host;        /* Module to use host as a device */
	ort_module_t      *modules;            /* Device modules (except host) */
	int                num_modules;
	ort_device_t      *ort_devices;        /* not working with ee_process yet */
	int                num_devices;        /* device 0 is the host */

	volatile ee_lock_t atomic_lock;        /* Global lock for atomic */
	volatile ee_lock_t preparation_lock;   /* For initializing user locks */
	volatile ee_lock_t eecb_rec_lock;
	ort_eecb_t        *eecb_recycler;      /* not working with ee_process yet */
	
	int                thrpriv_num;        /* # threadprivate variables */
	int                nthr_per_level[MAX_NUMTHR_LEVELS]; /* nthreads[] list */
	omp_proc_bind_t    bind_per_level[MAX_BIND_LEVELS]; /* binds[] list, v4.0.0 */
	int                set_nthrlevs;       /* # levels of nthreads defined */
	int                set_bindlevs;       /* # levels of nthreads defined */
	
	int                ompi_steal_policy;  /* Worker steal? FIFO:LIFO */
	int                taskqueuesize;      /* Size of task queues */
	int                dynamic_taskqueuesize; /* Adapt task queue size */
	int                partotask_policy;   /* Nested parallel-for/sections2task*/
	
	int              **place_partition;    /* OpenMP4 */
	league_t           league;             /* The current league */
	int                initleague_cgsize;  /* The initial contention group size */ 
	int               *argc;               /* Pointer to main's argument */
	char            ***argv;               /* Pointer to main's argument */
} ort_vars_t;


#define INITLEAGUE()  (ort->league.cg_inithr == NULL)
#define CG_SIZE(cgid) (ort->league.cg_size[cgid])

#define CANCEL_ENABLED() (ort->icvs.cancel)   /* True if enabled */

#define TEAMINFO(eecb) ((eecb)->parent->mf)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                       *
 *  VARIABLES & MORE MACROS                              *
 *                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


extern ort_vars_t *ort;
/* My eecb */
#ifdef USE_TLS
	/* FIXME: This only works for pthreads */
	extern TLS_KEYWORD void *myeecb;
	#define __SETMYCB(v) (myeecb = v)
	#define __MYCB       ((ort_eecb_t *) myeecb)
#else
	extern ee_key_t eecb_key;
	#define __SETMYCB(v) ee_setspecific(eecb_key,v)
	#define __MYCB       ((ort_eecb_t *) ee_getspecific(eecb_key))
#endif

#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	#define __CURRTASK(eecb)             ((eecb)->tasking.current_executing_task)
	#define __SETCURRTASK(eecb,task)     ((eecb)->tasking.current_executing_task = task)
	#define __CURRIMPLTASK(eecb)         ((eecb)->tasking.current_implicit_task)
	#define __SETCURRIMPLTASK(eecb,task) ((eecb)->tasking.current_implicit_task = task)
	#define __INHERITASK(eecb)           ((eecb)->tasking.current_executing_task->inherit_task_node)
	#define __FINALTASK(eecb)            ((eecb)->tasking.current_executing_task->isfinal)
#else
	#define __CURRTASK(eecb)         (ee_get_currtask((eecb)->ee_info, (eecb)->thread_num))
	#define __SETCURRTASK(eecb,task) ee_set_currtask(task)
	#define __INHERITASK(eecb)       (ee_get_currtask((eecb)->ee_info, (eecb)->thread_num)->inherit_task_node)
	#define __FINALTASK(eecb)        (ee_get_currtask((eecb)->ee_info, (eecb)->thread_num)->isfinal)
#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                         *
 *  FUNCTIONS etc (also used by the parser, see ort.defs)  *
 *                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int ee_pid();

#if defined(EE_TYPE_PROCESS)
	void ort_allocate_shared_globals_space();
	void ort_share_globals();
	void ort_shmalloc(void **p, int size, int *upd);
	void ort_shmfree(void *p);
	void ee_shmfree(int *p);
#endif

int   ort_initialize(int *argc, char ***argv, int embedmode, int nmodules, ...);
void  ort_finalize(int exitval);
void *ort_calloc(int size);
void  ort_prepare_omp_lock(omp_lock_t *lock, int type);
void  ort_execute_kernel_on_host(void *(*func)(void *), void *shared);
void *eecb_alloc(), eecb_free(ort_eecb_t *eecb);
void *mcbf_alloc(void);

/*
 * From env.c
 */
extern int display_env;
void ort_get_environment(void);
void display_env_vars(void);

/*
 * From places.c
 */
void  ort_get_default_places(void);
void  ort_getenv_places(char *s);
void  places_show();
char *places_get_list_str(int pfrom, int pto);
void  bindme(int eeid, ort_eecb_t *t, ort_eecb_t *parent);

/*
 * From affinity.c
 */
void ort_set_affinity_format(const char *format);
char *ort_get_default_affinity_format(void);
size_t ort_get_affinity_format(char *buffer, size_t size);
void ort_display_affinity(const char *format);
size_t ort_capture_affinity(char *buffer, size_t size, const char *format);

/*
 * From barrier.c
 */
void parallel_barrier_wait(ort_defbar_t *bar, int eeid);

/*
 * From worksharing.c
 */
/* Workshare-related functions */
void init_workshare_regions(ort_eecb_t *me);
int  ort_check_section();

/*
 * From tasks.c & taskspf.c
 */
void  ort_init_tasking();
void  start_throttling_due_to_full_queue(void);
void  ort_new_task(void *(*func)(void *arg), void *arg,
                   int now, int final, int untied, int priority,
                   void **deparray, int noutdeps, int nindeps, int ninoutdeps);
void  ort_taskwait(int waitall);
int   ort_task_execute_this(ort_eecb_t *me, ort_task_node_t *task_to_execute);
int   ort_task_throttling(void);
void  ort_create_task_immediate_node(ort_eecb_t *thr);
void *ort_task_immediate_start(int final);
void  ort_task_immediate_end(void *tn);
void  ort_execute_my_tasks(ort_eecb_t *me);
void  ort_start_implicit_task(ort_eecb_t *thr);
void  ort_finish_implicit_task(ort_eecb_t *thr);

void  spftasks_execute_node(ort_eecb_t *me, ort_task_node_t *tnode);
void  spftasks_create(ort_eecb_t *me, int ntasks, int offs, void *(*f)(void*));

/*
 * From taskdeps.c
 */
void tdeps_issue_task(ort_task_node_t *tnode, 
                      void **deparray, int out, int in, int inout);
void tdeps_after_execution(ort_task_node_t *tnode, void *me);
void tdeps_free_tdepinfo(void *dependencies);

/*
 * From pools.c
 */
ort_task_node_t *ort_task_alloc(void *(*func)(void *), void *arg);
ort_task_node_t *ort_task_alloc_init(void *(*func)(void *), void *arg, 
                             int final, int rtid, ort_eecb_t *thr);
void             ort_task_free(ort_eecb_t *thr, ort_task_node_t *node);
taskgroup_t     *taskgroup_alloc(void);
void             taskgroup_free(taskgroup_t *arg);
void             task_pools_init(ort_eecb_t *t);
void            *ort_taskenv_alloc(int size, void *(*task_func)(void *));

/*
 * From stealing.c
 */
void             ort_task_queues_init(ort_eecb_t *me, int nthr);
void             ort_task_worker_enqueue(ort_eecb_t *me, ort_task_node_t *tnde);
ort_task_node_t *ort_task_worker_dequeue(ort_eecb_t *me);
ort_task_node_t *ort_task_thief_steal(ort_eecb_t *me, int victim_id);

/*
 * From modules.c
 */
void ort_discover_modules(int nmodules, va_list ap);
void ort_initialize_device(int device_id);
void ort_finalize_devices();

/* 
 * From hostdev.c
 */
ort_module_t *ort_get_host_module();

/*
 * From target.c
 */
ort_device_t *ort_get_device(int dev_id);
void          ort_decltarg_initialize();
int           ort_illegal_device(char *reason, int devid);
void         *ort_checkmapped_var(void *var, ort_device_t *d, int *how);
int           target_associate_ptr(void *hostaddr, void *umedaddr, 
                                   u_long size, u_long devoff, int devid);
int           target_disassociate_ptr(void *hostaddr, int devid);

/*
 * From reduction.c
 */
void ort_reductions_init(ort_eecb_t *me, int teamsize);
void ort_reductions_finalize(ort_eecb_t *me);

#endif     /* __ORT_PRIVE_H__ */
