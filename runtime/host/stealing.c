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

#include "ort_prive.h"
#include <stdlib.h>
#include <stdio.h>

#define FAILURE 0
#define SUCCESS 1

/* Allocate memory for my group task queues */
inline void ort_task_queues_init(ort_eecb_t *me, int nthr)
{
#if !defined(AVOID_OMPI_DEFAULT_TASKS)
	int i;

	/* Thread becomes parent for first time */
	if (me->tasking.queue_table == NULL)
	{
		me->tasking.queue_table = (ort_task_queue_t *)
		                          ort_calloc((nthr + 1) * sizeof(ort_task_queue_t));

		for (i = 0; i < nthr + 1; i++) /* Initialize task queues */
		{
			me->tasking.queue_table[i].top         = 0;
			me->tasking.queue_table[i].bottom      = 0;
			me->tasking.queue_table[i].implicit_task_children = NULL;
			me->tasking.queue_table[i].tasks = (ort_task_node_t **)
			                                   ort_calloc(TASKQUEUESIZE * sizeof(ort_task_node_t *));
#if !defined(HAVE_ATOMIC_FAA) || !defined(HAVE_ATOMIC_CAS)
			ee_init_lock((ee_lock_t *) & (me->tasking.queue_table[i].lock),
			             ORT_LOCK_NORMAL);
#endif
		}
		me->tasking.max_children = nthr + 1;
	}
	else
	{
		if (me->tasking.max_children < nthr + 1)  /* realloc needed */
		{
			for (i = 0; i < me->tasking.max_children; i++) /* Initialize task queues */
				free(me->tasking.queue_table[i].tasks);

			/* Reallocate queue_table */
			me->tasking.queue_table = (ort_task_queue_t *)
			                          realloc(me->tasking.queue_table, (nthr + 1) * sizeof(ort_task_queue_t));

			for (i = 0; i < nthr + 1; i++)
				me->tasking.queue_table[i].tasks = (ort_task_node_t **)
				                                   ort_calloc(TASKQUEUESIZE * sizeof(ort_task_node_t *));

#if !defined(HAVE_ATOMIC_FAA) || !defined(HAVE_ATOMIC_CAS)
			for (i = me->tasking.max_children; i < nthr + 1; i++)
				ee_init_lock((ee_lock_t *) & (me->tasking.queue_table[i].lock),
				             ORT_LOCK_NORMAL);
#endif

			me->tasking.max_children = nthr + 1;
		}

		/* Reinitialize queue table elements */
		for (i = 0; i < nthr + 1; i++)
		{
			me->tasking.queue_table[i].top      = 0;
			me->tasking.queue_table[i].bottom   = 0;
			me->tasking.queue_table[i].implicit_task_children = NULL;
		}
	}
#endif
}

#if !defined(AVOID_OMPI_DEFAULT_TASKS)

/* ort_task_worker_enqueue
 * This function is used by a process (worker) in order to enqueue a new
 * task_node to its task queue.
 */
inline int ort_task_worker_enqueue(ort_eecb_t *me, void *(*func)(void *),
                                   void *arg, int final)
{
	ort_task_node_t *new_node;
	int worker_id = me->thread_num;
	ort_eecb_t *my_parent = me->sdn;
	int old_bottom = atomic_read
	                 (&(my_parent->tasking.queue_table[worker_id].bottom));

	new_node = ort_task_alloc(func, arg);

	/* Check whether i use my own task node or an inherited one */
	if (__INHERITASK(me))
		ort_create_task_immediate_node(me);

#if !defined(HAVE_ATOMIC_FAA)
	ee_init_lock((ee_lock_t *) & (new_node->lock), ORT_LOCK_NORMAL);
#endif
	new_node->func              = func;
	/* I already have saved task fuc args */
	new_node->num_children      = 1; /* To ensure that a task is complete before freeing it */
	new_node->next              = NULL;
	new_node->parent            = __CURRTASK(me);
	new_node->icvs              = new_node->parent->icvs;
	new_node->inherit_task_node = 0;
	new_node->isfinal           = final;
	new_node->taskgroup         = new_node->parent->taskgroup;

	if (new_node->parent != NULL)  /* Add a new child to parent task */
	{
#if defined(HAVE_ATOMIC_FAA)
		_faa(&((new_node->parent)->num_children), 1);
#else
		ee_set_lock(&((new_node->parent)->lock));
		(new_node->parent)->num_children++;
		ee_unset_lock(&((new_node->parent)->lock));
#endif
	}
	my_parent->tasking.queue_table[worker_id].tasks[old_bottom % TASKQUEUESIZE] =
	  new_node;

	my_parent->tasking.queue_table[worker_id].bottom++;

#ifdef ORT_DEBUG
	me->tasking.tasks_enqueued++;
#endif

	return SUCCESS;
}


/* ort_task_worker_dequeue
 * This function is used by a process (worker) in order to dequeue a
 * new task_node from its task queue.
 */
inline ort_task_node_t  *ort_task_worker_dequeue(ort_eecb_t *me)
{
	ort_task_node_t *extracting_node;
	ort_eecb_t *my_parent = me->sdn;
	int worker_id = me->thread_num;
	int old_top;
	int new_top;
	int size;

	/* Make a first fast check */
	size = atomic_read(&(my_parent->tasking.queue_table[worker_id].bottom))
	       - atomic_read(&(my_parent->tasking.queue_table[worker_id].top)) - 1;

	/* If my queue is almost full, it is safe to enter throttle mode */
	if (size > (int)(TASKQUEUESIZE * 0.7))
		start_throttling_due_to_full_queue();

	if (size < 0) /* Queue is empty */
		return NULL;

#if defined(HAVE_ATOMIC_FAA)
	_faa(&(my_parent->tasking.queue_table[worker_id].bottom), -1);
#else
	ee_set_lock(&(my_parent->tasking.queue_table[worker_id].lock));
	my_parent->tasking.queue_table[worker_id].bottom--;
	ee_unset_lock(&(my_parent->tasking.queue_table[worker_id].lock));
#endif

	old_top = atomic_read(&(my_parent->tasking.queue_table[worker_id].top));
	new_top = old_top + 1;
	size = atomic_read(&(my_parent->tasking.queue_table[worker_id].bottom))
	       - old_top;

	if (size < 0) /* Queue is empty */
	{
		my_parent->tasking.queue_table[worker_id].bottom = old_top;
		return NULL;
	}

	extracting_node = my_parent->tasking.queue_table[worker_id]
	                  .tasks[atomic_read(&((my_parent->tasking.
	                                        queue_table[worker_id]).bottom)) % TASKQUEUESIZE];
	if (size > 0)
		return extracting_node;

	/* If there is only one task left in queue... */
#if defined(HAVE_ATOMIC_CAS)
	/* If a thief stole the last task... */
	if (!_cas(&((my_parent->tasking.queue_table[worker_id]).top),
	          old_top, new_top))
		extracting_node = NULL;/* then return NULL, else return the last task */
#else
	ee_set_lock(&((my_parent->tasking.queue_table[worker_id]).lock));

	if ((my_parent->tasking.queue_table[worker_id]).top == old_top)
		(my_parent->tasking.queue_table[worker_id]).top = new_top;
	else
		extracting_node = NULL;

	ee_unset_lock(&((my_parent->tasking.queue_table[worker_id]).lock));
#endif

	/* old_top + 1 = new_top */
	my_parent->tasking.queue_table[worker_id].bottom = old_top + 1;

	return extracting_node;
}


/* ort_task_thief_steal
 * This function is used by a process (thief) in order to dequeue a new
 * task_node from a victim process's task queue.
 */
inline ort_task_node_t *ort_task_thief_steal(ort_eecb_t *me, int victim_id)
{
	ort_task_node_t *extracting_node;
	ort_eecb_t *my_parent = me->sdn;
	int old_top = atomic_read(&(my_parent->tasking.queue_table[victim_id].top));
	int new_top = old_top + 1;
	int old_bottom = atomic_read(&
	                             (my_parent->tasking.queue_table[victim_id].bottom));
	int size = old_bottom - old_top;


	/* If my queue is almost full, it is safe to enter throttle mode */
	if (me->thread_num == victim_id && size > (int)(TASKQUEUESIZE * 0.7))
		start_throttling_due_to_full_queue();

	if (size <= 0) /* Victim's queue is empty */
		return NULL;
	/* Steal a task from vitim's top! */
	extracting_node = my_parent->tasking.queue_table[victim_id]
	                  .tasks[old_top % TASKQUEUESIZE];

#if defined(HAVE_ATOMIC_CAS)
	/* if thief managed to steal the task... */
	if (_cas(&((my_parent->tasking.queue_table[victim_id]).top),
	         old_top, new_top))
		return extracting_node;
#else
	ee_set_lock(&((my_parent->tasking.queue_table[victim_id]).lock));
	if ((my_parent->tasking.queue_table[victim_id]).top == old_top)
	{
		(my_parent->tasking.queue_table[victim_id]).top = new_top;
		ee_unset_lock(&((my_parent->tasking.queue_table[victim_id]).lock));
		return extracting_node;
	}
	ee_unset_lock(&((my_parent->tasking.queue_table[victim_id]).lock));
#endif

	return NULL;
}
#endif /* AVOID_OMPI_DEFAULT_TASKS */
