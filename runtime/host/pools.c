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

void *ort_allocate_eecb_from_recycle(void)
{
	ort_eecb_t *eecb;

	ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);

	if (ort->eecb_reycler == NULL) /* If recycle bin is empty */
		eecb = ort_calloc_aligned(sizeof(ort_eecb_t), NULL); /* Allocate new eecb */
	else
	{
		/* Pick the first from recycle bin */
		eecb = ort->eecb_reycler;
		ort->eecb_reycler = ort->eecb_reycler->next;
	}

	ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);

	return (void *)eecb;
}

void ort_ee_exit(void)
{
	ort_eecb_t *eecb = __MYCB;

	ee_set_lock((ee_lock_t *) &ort->eecb_rec_lock);

	eecb->next = ort->eecb_reycler;
	ort->eecb_reycler = eecb;

	ee_unset_lock((ee_lock_t *) &ort->eecb_rec_lock);
}

#if !defined(AVOID_OMPI_DEFAULT_TASKS)
/* ort_task_alloc
 * This function tries to use the recycle bin of process 'thread_id'
 * in order to reuse a previusly used 'ort_task_node_t' structure. if recycle
 * bin is empty then a new 'ort_task_node_t' is returned using malloc.
 */
ort_task_node_t *ort_task_alloc(void *(*func)(void *), void *arg)
{
	if (arg != NULL)  /* Return task node that corresponds to this task arg */
		return *((ort_task_node_t **)PP(arg));
	if (func != NULL)
	{
		arg = ort_taskenv_alloc(0, func);
		return (*((ort_task_node_t **)PP(arg)));
	}
	return ort_task_empty_node_alloc();
}


/* ort_task_free
 * This function puts a 'ort_task_node_t' structure in process's 'thread_id'
 * recycle bin in order to reuse it in the future.
 */
void ort_task_free(ort_eecb_t *thr, ort_task_node_t *node)
{
	ort_task_node_pool_t *task_node_pool;
	int my_max_mates = thr->tasking.max_mates;

	/* if node was allocated form a node pool */
	if (node->occupied == 1)
		node->occupied = 0;
	else /* else store node in my recycle bin */
	{
		/* Search for appropriate recycle bin */
		task_node_pool = thr->tasking.task_node_pool;
		while (task_node_pool != NULL)
		{
			if (task_node_pool->task_func == node->func)
			{
				/* I found matching bin */
				node->next = task_node_pool->recycler;
				task_node_pool->recycler = node;

				return;
			}
			task_node_pool = task_node_pool->next;
		}

		/* I don't have specified task node pool. Create one */
		task_node_pool = (ort_task_node_pool_t *)
		                 ort_calloc_aligned(sizeof(ort_task_node_pool_t), NULL);
		task_node_pool->task_func = node->func;
		node->next = NULL;
		task_node_pool->recycler = node;
		task_node_pool->sub_pool = (ort_task_node_t *)
		                           ort_calloc(TASKPOOLSIZE(my_max_mates) * sizeof(ort_task_node_t));

		task_node_pool->next = thr->tasking.task_node_pool;
		thr->tasking.task_node_pool = task_node_pool;
	}
}

void task_pools_init(ort_eecb_t *t)
{
	ort_tasking_t        *td = &(t->tasking);
	ort_task_node_pool_t *tnp;
	int                   i, teamsize = t->num_siblings;

	if (td->task_node_pool == NULL)
	{
		td->max_mates = teamsize;
		return;
	}
	else
		if (td->max_mates >= teamsize)
			return;

	tnp = td->task_node_pool;
	while (tnp != NULL)
	{
		for (i = 0; i < TASKPOOLSIZE(td->max_mates); i++)
		{
			if ((tnp->sub_pool[i]).funcarg != NULL)
				free((tnp->sub_pool[i]).funcarg);
		}

		tnp->sub_pool = (ort_task_node_t *)
		                ort_realloc(tnp->sub_pool, TASKPOOLSIZE(teamsize) * sizeof(ort_task_node_t));

		for (i = 0; i < TASKPOOLSIZE(teamsize); i++)
		{
			(tnp->sub_pool[i]).next      = NULL;
			(tnp->sub_pool[i]).func      = NULL;
			(tnp->sub_pool[i]).occupied  = 0;
			(tnp->sub_pool[i]).funcarg   = NULL;
			(tnp->sub_pool[i]).taskgroup = 0;
		}

		tnp = tnp->next;
	}

	td->max_mates = teamsize;
}

#endif /* AVOID_OMPI_DEFAULT_TASKS */

void *ort_taskenv_alloc(int size, void *(*task_func)(void *))
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
#if 1
	return (ee_taskenv_alloc(size, task_func));
#else
	return (malloc(size));
#endif
#else
	ort_eecb_t      *me = __MYCB;
	ort_task_node_pool_t *task_node_pool;
	ort_task_node_t      *new_node;
	int i;
	int my_max_mates = me->tasking.max_mates;

	task_node_pool = me->tasking.task_node_pool;

	while (task_node_pool != NULL)
	{
		if (task_node_pool->task_func == task_func) /* I found matching pool*/
		{
			/* Try to match an empty task_env_pool node */
			for (i = 0; i < (TASKPOOLSIZE(my_max_mates)); i++)
				if (task_node_pool->sub_pool[i].occupied == 0)
				{
					/* I found an empty task node */
					if (task_node_pool->sub_pool[i].funcarg == NULL)
						/* This is the first time i use this task node */
						ALLOCATE_ENV(task_node_pool->sub_pool[i], size);


					task_node_pool->sub_pool[i].occupied = 1;
					return NP(task_node_pool->sub_pool[i].funcarg);
				}

			/* Pool is full, allocate a task node and return it */
			new_node = task_node_pool->recycler;

			if (new_node == NULL)
			{
				new_node = (ort_task_node_t *)ort_calloc_aligned(sizeof(ort_task_node_t), NULL);
				new_node->next = NULL;
				ALLOCATE_PENV(new_node, size);
				new_node->occupied = 2;
			}
			else
				task_node_pool->recycler = task_node_pool->recycler->next;

			return NP(new_node->funcarg);
		}

		task_node_pool = task_node_pool->next;
	}

	/* I did not find a matching rbin. allocate some space */
	task_node_pool = (ort_task_node_pool_t *)ort_calloc_aligned(sizeof(
	    ort_task_node_pool_t), NULL);
	task_node_pool->task_func = task_func;
	task_node_pool->recycler = NULL;
	task_node_pool->sub_pool = (ort_task_node_t *)\
	                           ort_calloc(TASKPOOLSIZE(my_max_mates) * sizeof(ort_task_node_t));

	task_node_pool->next = me->tasking.task_node_pool;
	me->tasking.task_node_pool = task_node_pool;


	ALLOCATE_ENV(task_node_pool->sub_pool[0], size);
	task_node_pool->sub_pool[0].occupied = 1;
	return NP(task_node_pool->sub_pool[0].funcarg);
#endif
}

#if !defined(AVOID_OMPI_DEFAULT_TASKS)
ort_task_node_t *ort_task_empty_node_alloc(void)
{
	ort_eecb_t      *me = __MYCB;
	ort_task_node_pool_t *task_node_pool;
	ort_task_node_t      *new_node;
	int i;
	int my_max_mates = me->tasking.max_mates;

	task_node_pool = me->tasking.task_node_pool;

	while (task_node_pool != NULL)
	{
		if (task_node_pool->task_func == NULL) /* I found matching pool*/
		{
			/* Try to match an empty task_env_pool node */
			for (i = 0; i < (TASKPOOLSIZE(my_max_mates)); i++)
				if (task_node_pool->sub_pool[i].occupied == 0)
				{
					/* I found an empty task node */
					task_node_pool->sub_pool[i].funcarg = NULL;
					task_node_pool->sub_pool[i].occupied = 1;
					return &(task_node_pool->sub_pool[i]);
				}

			/* Pool is full, allocate a task node and return it */
			new_node = task_node_pool->recycler;

			if (new_node == NULL)
			{
				new_node = (ort_task_node_t *)ort_calloc_aligned(sizeof(ort_task_node_t), NULL);
				new_node->next = NULL;
				new_node->occupied = 2;
			}
			else
				task_node_pool->recycler = task_node_pool->recycler->next;

			return new_node;
		}

		task_node_pool = task_node_pool->next;
	}

	/* I did not find a matching rbin. allocate some space */
	task_node_pool = (ort_task_node_pool_t *)ort_calloc_aligned(sizeof(
	    ort_task_node_pool_t), NULL);
	task_node_pool->task_func = NULL;
	task_node_pool->recycler = NULL;
	task_node_pool->sub_pool = (ort_task_node_t *)\
	                           ort_calloc(TASKPOOLSIZE(my_max_mates) * sizeof(ort_task_node_t));

	task_node_pool->next = me->tasking.task_node_pool;
	me->tasking.task_node_pool = task_node_pool;

	task_node_pool->sub_pool[0].funcarg = NULL;
	task_node_pool->sub_pool[0].occupied = 1;
	return &(task_node_pool->sub_pool[0]);
}
#endif


/* taskgroup_alloc
 * This function tries to use the recycle bin of taskgroup_t
 * in order to reuse a previusly used 'taskgroup_t' structure. if recycle
 * bin is empty then a new 'taskgroup_t' is returned using malloc.
 */
taskgroup_t *taskgroup_alloc(void)
{
	ort_eecb_t *me = __MYCB;

	if (me->tg_recycler != NULL)
	{
		taskgroup_t *tmp;
		tmp = me->tg_recycler;
		me->tg_recycler = me->tg_recycler->next;

		return tmp;
	}
	else
		return (taskgroup_t *)malloc(sizeof(taskgroup_t));
}


/* taskgroup_free
 * This function puts a 'taskgroup_t' structure in
 * recycle bin in order to reuse it in the future.
 */
void taskgroup_free(taskgroup_t *arg)
{
	ort_eecb_t *me = __MYCB;

	arg->next = me->tg_recycler;
	me->tg_recycler = arg;
}

void ort_taskenv_free(void *ptr, void *(*task_func)(void *))
{
#if defined(AVOID_OMPI_DEFAULT_TASKS)
#if 1
	ee_taskenv_free(ptr);
#else
	free(ptr);
#endif
	return;
#else
	;
#endif
}
