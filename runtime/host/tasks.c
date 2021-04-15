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

/* tasks.c -- OMPi RunTime library, tasking */

/*
 * 2010/11/20:
 *   added calls for getting & setting task icvs..
 * Version 1.0.1j:
 *   first time around, out of ort.c code.
 */

#include "ort_prive.h"
#include <stdlib.h>
#include <stdio.h>

#define FAILURE 0
#define NO_TASKS_LEFT -1
#define NO_VICTIM_LEFT -1


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * GLOBAL VARIABLES / DEFINITIONS / MACROS                           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Per-thread task throttling */
#ifdef USE_TLS
	TLS_KEYWORD int throttle;
	#define __start_throttling() (throttle = 1)
	#define __stop_throttling()  (throttle = 0)
	#define __check_throttling() (throttle + 0)
#else
	ee_key_t   throttle_key;    /* For thread-specific task throttling */
	#define __start_throttling() ee_setspecific(throttle_key,(void *) 1)
	#define __stop_throttling()  ee_setspecific(throttle_key,(void *) 0)
	#if !defined(AVOID_OMPI_DEFAULT_TASKS)
		#define __check_throttling() (0 != (uintptr_t) ee_getspecific(throttle_key))
	#else
		#define __check_throttling() (ee_check_throttling())
	#endif
#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * TASKS                                                             *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void start_throttling_due_to_full_queue(void)
{
#ifdef ORT_DEBUG
	__MYCB->tasking.throttled_queue++;
	__MYCB->tasking.in_throttle++;
#endif
	__start_throttling();
}


void ort_init_tasking()
{
#ifdef USE_TLS
#else
	ee_key_create(&throttle_key, 0);
#endif
	__stop_throttling();
}


#if !defined(AVOID_OMPI_DEFAULT_TASKS)

/* ort_task_check_throttling
 * This function checks whether a thread's task queue
 * is 70% full of tasks. If not thread will stop throttling
 */
inline static void ort_task_check_throttling(ort_eecb_t *me)
{
	ort_eecb_t *my_parent = me->sdn;
	int my_id = me->thread_num;
	int old_bottom, old_top;

	/* Now check that i have enough space in Task Queue */
	old_bottom = atomic_read
	             (&(my_parent->tasking.queue_table[my_id].bottom));
	old_top = atomic_read
	          (&(my_parent->tasking.queue_table[my_id].top));

	/* If my queue is less than 70% full */
	if ((old_bottom - old_top) < (int)(TASKQUEUESIZE * 0.7))
	{
		__stop_throttling();
#ifdef ORT_DEBUG
		me->tasking.out_throttle++;
#endif
	}

	return;
}
#endif /* AVOID_OMPI_DEFAULT_TASKS */


void ort_create_task_immediate_node(ort_eecb_t *thr)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	return;
#else
	ort_task_node_t *new_node;

	/* Create a task node in order to save task data */
	new_node = ort_task_alloc(NULL, NULL);

#if !defined(HAVE_ATOMIC_FAA)
	ee_init_lock((ee_lock_t *) & (new_node->lock), ORT_LOCK_NORMAL);
#endif
	new_node->func              = NULL;
	new_node->num_children      = 1; /* To ensure that a task is complete before freeing it */
	new_node->next              = NULL;
	new_node->parent            = __CURRTASK(thr);
	new_node->icvs              = __CURRTASK(thr)->icvs;
	new_node->inherit_task_node = 0;
	new_node->isfinal           = __FINALTASK(thr);
	new_node->taskgroup         = __CURRTASK(thr)->taskgroup;
	new_node->rtid              = 0;
	new_node->dependencies      = NULL;

	/* I have my own task node now, Reduce parent task's counter */
	__INHERITASK(thr) --;

	/* I have my own task node now, Reduce parent task's final counter */
	if (__FINALTASK(thr) > 0)
		__FINALTASK(thr)--;

	__CURRTASK(thr) = new_node;

	/* Check whether i have to stop throttling */
	if (thr->num_siblings != 1)
		ort_task_check_throttling(thr);

	return;
#endif
}


#if !defined(AVOID_OMPI_DEFAULT_TASKS)

int ort_task_execute_this(ort_eecb_t *me, ort_task_node_t *task_to_execute)
{
	ort_task_node_t *prev_task_to_execute;
	int             task_siblings, exec_task_children;

	if (task_to_execute->rtid == -1)  /* If i am to execute a normal task */
	{
		/* First update thread executing status... */
		prev_task_to_execute = __CURRTASK(me);
		__CURRTASK(me) = task_to_execute;

		/* Function arguments pointer is stored in next pointer */
		if (!me->sdn->mf->cancel_par_active)
		{
			if (__CURRTASK(me)->taskgroup == NULL)
				(task_to_execute->func)(NP(task_to_execute->funcarg));
			else
				if (__CURRTASK(me)->taskgroup->is_canceled == false)
					(task_to_execute->func)(NP(task_to_execute->funcarg));
		}
		
		/* OpenMP 4.0:
		 * If in a taskgroup, i have to execute my child tasks before returning 
		 */
		if (task_to_execute->taskgroup != NULL)
			ort_taskwait(0);

		/* Finished my job; update thread executing status */
		tdeps_after_execution(task_to_execute, me);
		__CURRTASK(me) = prev_task_to_execute;
	}
	else /* A special parallel for task */
		spftasks_execute_node(me, task_to_execute);

	if (task_to_execute->parent != NULL)   /* If task has a parent */
	{
#if defined(HAVE_ATOMIC_FAA)
		task_siblings = _faa(&((task_to_execute->parent)->num_children), -1);
#else
		ee_set_lock(&((task_to_execute->parent)->lock));
		task_siblings = task_to_execute->parent->num_children;
		task_to_execute->parent->num_children--;
		ee_unset_lock(&((task_to_execute->parent)->lock));
#endif
		if (task_siblings == 1)
		{
			if (task_to_execute->parent->dependencies)
				tdeps_free_tdepinfo(task_to_execute->parent->dependencies);
			ort_task_free(me, task_to_execute->parent);
		}
	}

	/* This task is over, subtract the virtual child of this task */
#if defined(HAVE_ATOMIC_FAA)
	exec_task_children = _faa(&(task_to_execute->num_children), -1);
#else
	ee_set_lock(&((task_to_execute)->lock));
	exec_task_children = task_to_execute->num_children;
	task_to_execute->num_children--;
	ee_unset_lock(&((task_to_execute)->lock));
#endif

	/* Free memory, if task has no children */
	if (exec_task_children == 1)
	{
		if (task_to_execute->dependencies)
			tdeps_free_tdepinfo(task_to_execute->dependencies);
		ort_task_free(me, task_to_execute);
	}
}


/* execute_any_task:
 * When a thread calls this functions, it dequeues a task from its queue, 
 * or steals a task from a victim and executes it.
 */
static int execute_any_task(ort_eecb_t *me, int startfrom)
{
	int thread_id = me->thread_num;
	ort_eecb_t *my_parent = me->sdn;
	ort_task_node_t *task_to_execute;
	int victim;
	int my_team_members = my_parent->mf->num_children;
	int my_thread_id = thread_id;
	int search_limit;

	if (OMPI_STEAL_POLICY == LIFO)
		task_to_execute = ort_task_worker_dequeue(me);
	else
		task_to_execute = ort_task_thief_steal(me, thread_id);

	/* My task queue is empty; have to steal something... */
	if (task_to_execute == NULL)
	{
		if (startfrom >= 0)              /* Start stealing from there */
		{
			thread_id    = startfrom - 1;
			search_limit = my_team_members + thread_id + 1;
		}
		else
			search_limit = my_team_members + thread_id;

		for (victim = thread_id + 1; victim < search_limit; victim++)
		{
			if (victim == my_thread_id)
				continue;

			task_to_execute = ort_task_thief_steal(me, victim % my_team_members);
			/* If victim's queue is also empty, try next victim */
			if (task_to_execute == NULL)
			{
#ifdef ORT_DEBUG
				me->tasking.fail_theft_attemts++;
#endif
				continue;
			}

			ort_task_execute_this(me, task_to_execute);
#ifdef ORT_DEBUG
			me->tasking.tasks_executed_by_thief++;
#endif
			return victim % (my_parent->mf->num_children);
		}
		return -1; /* There was no task left to execute */
	}
	else
	{
		ort_task_execute_this(me, task_to_execute);
#ifdef ORT_DEBUG
		me->tasking.tasks_executed_by_worker++;
#endif
		return me->thread_num;
	}
}


/* Only execute tasks from my queue (i.e. do not attempt to steal)
 */
void ort_execute_my_tasks(ort_eecb_t *me)
{
	ort_task_node_t *tnode;

	for (;;)
	{
		if (OMPI_STEAL_POLICY == LIFO)
			tnode = ort_task_worker_dequeue(me);
		else
			tnode = ort_task_thief_steal(me, me->thread_num);

		if (tnode == NULL)     /* drained */
			return;

		ort_task_execute_this(me, tnode);
#ifdef ORT_DEBUG
		me->tasking.tasks_executed_by_worker++;
#endif
	}
}

#endif /* AVOID_OMPI_DEFAULT_TASKS */


/* This function creates a new task node and sets this task as the
 * calling thread's current task. Called when a task is about to be
 * executed immediately.
 */
inline void *ort_task_immediate_start(int final)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	return (void *)ee_task_immediate_start(final);
#else
	ort_eecb_t *me = __MYCB;

#ifdef ORT_DEBUG
	me->tasking.throttled++;
#endif

	/* Speed up immediate task execution; I inherited task from my father */
	__INHERITASK(me) ++;

	/* Increase final counter for "final" information bookkeeping */
	if (__FINALTASK(me) > 0 || final > 0)
		__FINALTASK(me)++;

	/* Check whether i have to stop throttling */
	if (me->num_siblings != 1)
		ort_task_check_throttling(me);

	return me;
#endif
}


inline void ort_task_immediate_end(void *my_eecb)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	ee_task_immediate_end(my_eecb);
#else
	ort_eecb_t      *me = (ort_eecb_t *)my_eecb;
	ort_task_node_t *task_node;

	if (__INHERITASK(me) > 0)
	{
		/* I executed a final task immemdiately, information bookkeeping */
		if (__FINALTASK(me) > 0)
			__FINALTASK(me)--;

		/* I inherited task from my father, nothing to do */
		__INHERITASK(me) --;
		return;
	}

	task_node = __CURRTASK(me);
	__CURRTASK(me) = task_node->parent;    /* Restore task node */

	if (task_node->num_children == 1)
		ort_task_free(me, task_node);          /* Recycle task node */

	return;
#endif
}


/* Create a new task.
 * Here it is assumed that the compiler has already issued a check for no 
 * throttling. Since only me can put tasks in my queue, no throttling 
 * guarantees that I do have space for a new task.
 * However, because of OpenMP v45 #target-related constructs, there is a
 * chance that this function gets called without a prior check for throttling.
 * Consequently, we go through a seemingly redunant throttling check.
 * This check also enables the implementation a single (but slow) tasking
 * code generation,   if the user so desires (yeah, sure...).
 * @param func:       the task function
 * @param arg:        the argument to the task function
 * @param now:        if true, execute the task immediately
 * @param final:      came with a final clause
 * @param untied:     came with an untied clause
 * @param deparray:   array with all dependences
 * @param noutdeps:   # out dependences
 * @param nindeps:    # in dependences
 * @param ninoutdeps: # inout dependences
 */
void ort_new_task(void *(*func)(void *arg), void *arg, 
                  int now, int final, int untied, int priority,
                  void **deparray, int noutdeps, int nindeps, int ninoutdeps)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	ee_new_task(final, untied, func, arg);

#else
	ort_eecb_t      *me = __MYCB;
	ort_task_node_t *tnode;
	int             mefinal = ( __FINALTASK(me) > 0 );

	if (priority < 0) priority = 0;
	if (priority > ort->icvs.max_task_prio) priority = ort->icvs.max_task_prio;
	
	/* If dependencies are present, follow another route (unless I am final) */
	if  (deparray && !mefinal)
	{
		tdeps_issue_task(ort_task_alloc_init(func, arg, final, -1, me),
		                 deparray, noutdeps, nindeps, ninoutdeps);
		return;
	}

	/* Check whether i am a final task or if i should throttle (VVD, OpenMP v45);
	 * if the latter test seems to slow things down, we should add a new
	 * parameter to ort_new_task() to inform it whether the throttlilng check
	 * has already been performed.
	 */
	if (now || mefinal || ort_task_throttling())
	{
#ifdef ORT_DEBUG
		if (isfinal)
			me->tasking.throttled_final++;
#endif

		/* If so then execute immediately; my children will also be final */
		tnode = ort_task_immediate_start(mefinal);
		(*func)(arg);

		if (arg != NULL)
			ort_task_free(me, *((ort_task_node_t **)PP(arg)));
		ort_task_immediate_end(tnode);

		return;
	}

	ort_task_worker_enqueue(me, ort_task_alloc_init(func, arg, final, -1, me));
	testnotset(me->sdn->tasking.never_task);      /* Inform my mates */
#endif
}


#if !defined(AVOID_OMPI_DEFAULT_TASKS)
static int check_for_tasks(ort_eecb_t *me)
{
	int              teamsize = me->num_siblings, victim, retry = NO_VICTIM_LEFT;
	ort_task_queue_t *q = me->sdn->tasking.queue_table;

	/* Search for # of unfinished tasks in my mates queues */
	for (victim = me->thread_num + 1; victim < teamsize + me->thread_num; victim++)
	{
		if ((q[victim % teamsize]).implicit_task_children != NULL &&
		    *((q[victim % teamsize]).implicit_task_children) > 1)
			return (victim % teamsize);
	}

	return retry;
}
#endif


/* How = 0 (wait for my children), 
 *       1 (wait for all team tasks),
 *       2 (wait at the end of parallel)
 */

#if defined(EE_TYPE_PROCESS)

void ort_taskwait(int how)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t *me = __MYCB;
	ee_taskwait(how, me->parent->ee_info, me->thread_num);
#endif
}

#else

void ort_taskwait(int how)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_eecb_t *me = __MYCB;
	ee_taskwait(how, me->parent->ee_info, me->thread_num);
#else
	ort_eecb_t *me = __MYCB;
	int        victim = NO_VICTIM_LEFT;

	if (me->num_siblings == 1)
		return;
	else if (how < 2 && me->parent->tasking.never_task == 0)
		return;
	else if (how == 2)
	{
		if(__CURRTASK(me)->rtid == -1) /* Normal task lightweight barrier */
			parallel_barrier_wait(TEAMINFO(me)->barrier, me->thread_num);
		else /* Parallel for task lightweight barrier */
			goto EXECUTE_TEAM_TASKS;
				return;
	};

	if (how > 0)   /* help with all the tasks in current team */
	{
		EXECUTE_TEAM_TASKS:
		do
		{
			while ((victim = execute_any_task(me, victim)) != NO_TASKS_LEFT)
				;
		}
		while ((victim = check_for_tasks(me)) != NO_VICTIM_LEFT);
	}
	else           /* execute till all my child tasks finish */
		while (__CURRTASK(me)->num_children > 1)
			execute_any_task(me, victim);
#endif
}

#endif


/* Task throttling.
 * For the moment, this is a per-thread flag that should be adjusted
 * adaptively.
 * A simple policy would be to __start_throttling() when the number of
 * tasks in my private queue exceeds c*N where c is a constant and N
 * is the number of processors. If later I discover that the number
 * fell below this threshold, I __stop_throttling().
 */
int ort_task_throttling(void)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	return __check_throttling();
#else

	ort_eecb_t *me = __MYCB;
	
	/* Check if i am already throttling */
	if (__check_throttling())
		return 1;
	/* Check if my team consists of one thread */
	if (me->num_siblings == 1)
	{
#ifdef ORT_DEBUG
		me->tasking.throttled_serial++;
#endif
		__start_throttling();
		return 1;
	}

	/* Check queue */
	{
		ort_task_queue_t *Q = &(me->sdn->tasking.queue_table[me->thread_num]);
		int old_bottom      = atomic_read( &(Q->bottom) );
		int old_top         = atomic_read( &(Q->top) );

		if ((old_bottom - old_top) >= TASKQUEUESIZE) /* Q about to be full */
		{
#ifdef ORT_DEBUG
			me->tasking.throttled_queue++;
			me->tasking.in_throttle++;
#endif
			__start_throttling();
			return 1;
		}
		else
			return 0;  /* No reason to throttle */
	}

#endif
}

/* Only called from othr.c, when in nestable locks */
void *ort_get_current_task()
{
	return (void *) __CURRTASK(__MYCB);
}

void ort_start_implicit_task(ort_eecb_t *thr)
{
	ort_eecb_t      *parent_thread = thr->parent;
	ort_task_node_t *tnode, *parent_task;

#if defined(AVOID_OMPI_DEFAULT_TASKS)

	if (thr->num_siblings == 1)
	{
		parent_task  = __CURRTASK(parent_thread);

		tnode = (ort_task_node_t *) calloc(1, sizeof(ort_task_node_t));
		tnode->func         = parent_thread->mf->workfunc;
		tnode->num_children = 0;
		tnode->next         = NULL;
		tnode->parent       = parent_task;
		tnode->inherit_task_node = 0;
		tnode->icvs         = parent_task->icvs;
		tnode->isfinal      = 0;
		tnode->taskgroup    = NULL;
		/* OpenMP 3.1 */
		if (thr->activelevel != parent_thread->activelevel) /* not a team of 1 */
			if (thr->activelevel < ort->set_nthrlevs)/* Use the user-supplied value */
				tnode->icvs.nthreads = ort->nthr_per_level[thr->activelevel];

		/* OpenMP 4.0 */
		if (thr->activelevel-1 < ort->set_bindlevs)/* Use the user-supplied value */
			tnode->icvs.proc_bind = ort->bind_per_level[thr->activelevel-1];

		__SETCURRTASK(thr, tnode);
		__SETCURRIMPLTASK(thr, tnode);
	}

#else
	task_pools_init(thr);

	/* Check whether i use my own task node or an inherited one */
	if (__INHERITASK(parent_thread))
		ort_create_task_immediate_node(parent_thread);

	parent_task = __CURRTASK(parent_thread);

	tnode = ort_task_alloc(NULL, NULL);
	tnode->rtid         = -1; /* Not a parallel for task */
	tnode->func         = NULL;
	tnode->num_children = 1; /* To ensure a task is complete before freeing it */
	tnode->next         = NULL;
	tnode->parent       = parent_task;
	tnode->inherit_task_node = 0;
	tnode->icvs         = parent_task->icvs;
	tnode->isfinal      = 0;
	tnode->taskgroup    = NULL;
	tnode->dependencies = NULL;  /* OpenMP 4.0 task dependencies */
	
	/* OpenMP 3.1 */
	if (thr->activelevel != parent_thread->activelevel) /* i.e. not a team of 1 */
		if (thr->activelevel < ort->set_nthrlevs)  /* Use the user-supplied value */
			tnode->icvs.nthreads = ort->nthr_per_level[thr->activelevel];

	/* OpenMP 4.0 */
	if (thr->activelevel != parent_thread->activelevel) /* i.e. not a team of 1 */
		if (thr->activelevel-1 < ort->set_bindlevs)/* Use the user-supplied value */
			tnode->icvs.proc_bind = ort->bind_per_level[thr->activelevel-1];

	/* Save # of children in order to use it in barrier task wait */
	(parent_thread->tasking.queue_table[thr->thread_num]).implicit_task_children
	  = &(tnode->num_children);

#if defined(HAVE_ATOMIC_FAA)
	_faa(&(parent_task->num_children), 1);
#else
	ee_set_lock(&(parent_task->lock));
	(parent_task->num_children)++;
	ee_unset_lock(&(parent_task->lock));
#endif
	__SETCURRTASK(thr, tnode);
	__SETCURRIMPLTASK(thr, tnode);

#endif
}

void ort_finish_implicit_task(ort_eecb_t *thr)
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
	ort_task_node_t *tnode;

	if (thr->num_siblings == 1)
	{
		tnode = __CURRTASK(thr);
		__SETCURRTASK(thr, tnode->parent);
		__SETCURRIMPLTASK(thr, tnode->parent);
		free(tnode);
	}
#else
	ort_task_node_t *tnode = __CURRTASK(thr);

#if defined(HAVE_ATOMIC_FAA)
	_faa(&(tnode->parent->num_children), -1);
#else
	ee_set_lock(&(tnode->parent->lock));
	tnode->parent->num_children--;
	ee_unset_lock(&(tnode->parent->lock));
#endif

	if (thr->num_siblings > 1)   /* lightweight barrier: */
		ort_taskwait(1);           /* basicaly help with any tasks you can find */

#ifdef ORT_DEBUG
	{
		void ort_task_stats(void);
		ort_task_stats();
	}
#endif

	__SETCURRTASK(thr, tnode->parent);
	__SETCURRIMPLTASK(thr, tnode->parent);

	if (tnode->dependencies)
		tdeps_free_tdepinfo(tnode->dependencies);
	ort_task_free(thr, tnode);              /* recycle task node */
#endif
}


/* OpenMP 4.0 */
/*
 * This function sets the taskgroup flag of current task to
 * true in order to denote that thread has entered a taskgroup
 * area. From now on all child tasks wil get taskgroup flag
 * also enabled. If a task has this flag enabled then an implicit
 * taskwait is called before this task finishes executing
 */
void ort_entering_taskgroup(void)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)

	ort_eecb_t *me = __MYCB;
	taskgroup_t *new_tg = taskgroup_alloc();

	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);

	new_tg->parent = __CURRTASK(me)->taskgroup;
	new_tg->is_canceled = 0;
	new_tg->next = NULL;

	__CURRTASK(me)->taskgroup = new_tg;
	
#endif
}


/* OpenMP 4.0 */
/*
 * This function sets the taskgroup flag of current task to
 * false in order to denote that thread will exit a taskgroup
 * area. Before exiting the taskgroup area a taskwait is
 * executed.
 */
void ort_leaving_taskgroup(void)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)

	/* Wait for my children */
	ort_taskwait(0);
	taskgroup_t *deleted_tg;

	/* Taskgroup are is finished */
	deleted_tg = __CURRTASK(__MYCB)->taskgroup;
	__CURRTASK(__MYCB)->taskgroup = __CURRTASK(__MYCB)->taskgroup->parent;
	taskgroup_free(deleted_tg);

#endif
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * DEBUGGING                                                         *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef ORT_DEBUG

void ort_task_stats(void)
{
	ort_eecb_t *me = __MYCB;

	fprintf(stderr, "task stats @ thread %d:\n\t"
	        "             enqueued: %ld\n\t"
	        "  dequeued & executed: %ld\n\t"
	        "                stole: %ld\n\t"
	        "throttled (immediate): %ld\n\t"
	        " {\n\t"
	        "     due to full pool:   %ld\n\t"
	        "    due to full queue:   %ld\n\t"
	        "     due to if(FALSE):   %ld\n\t"
	        "         due to final:   %ld\n\t"
	        "  outside of parallel:   %ld\n\t"
	        "     rest (fast code):   %ld\n\t"
	        "    got in throttling:   %ld\n\t"
	        "got out of throttling:   %ld\n\t"
	        "      failed stealing:   %ld\n\t"
	        " }\n\n",
	        me->thread_num,
	        me->tasking.tasks_enqueued,
	        me->tasking.tasks_executed_by_worker,
	        me->tasking.tasks_executed_by_thief,
	        me->tasking.throttled,
	        me->tasking.throttled_pool,
	        me->tasking.throttled_queue,
	        me->tasking.throttled_if,
	        me->tasking.throttled_final,
	        me->tasking.throttled_serial,
	        me->tasking.throttled - (me->tasking.throttled_pool +
	                                 me->tasking.throttled_queue +  me->tasking.throttled_if +
	                                 me->tasking.throttled_serial + me->tasking.throttled_final),
	        me->tasking.in_throttle,
	        me->tasking.out_throttle,
	        me->tasking.fail_theft_attemts
	       );

	me->tasking.tasks_enqueued = 0;
	me->tasking.tasks_executed_by_thief = 0;
	me->tasking.tasks_executed_by_worker = 0;
	me->tasking.throttled = 0;
	me->tasking.throttled_pool = 0;
	me->tasking.throttled_queue = 0;
	me->tasking.throttled_if = 0;
	me->tasking.throttled_final = 0;
	me->tasking.throttled_serial = 0;
	me->tasking.in_throttle = 0;
	me->tasking.out_throttle = 0;
	me->tasking.fail_theft_attemts = 0;
}

#endif
