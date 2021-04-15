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

/* taskspf.c -- special tasking for nested parallel loops */

#include "ort_prive.h"
#include <stdlib.h>
#include <stdio.h>


/* Arguments of the special tasks */
typedef struct 
{
	ort_eecb_t *arg;
} pf_task_args_t;


/* Called by the task schedulers, as task nodes are pull out of the queues 
 */
void spftasks_execute_node(ort_eecb_t *me, ort_task_node_t *tnode)
{
	ort_eecb_t *pteecb;
	
	if (me->me_partask == NULL)
		me->me_partask = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t),NULL);
	pteecb = me->me_partask;

	/* The special task was initialized when it was created */
	/* function arguments are stored funcarg next pointer */
	pteecb->parent       = (ort_eecb_t *)
	                       (((pf_task_args_t *)(NP(tnode->funcarg)))->arg);
	pteecb->sdn          = pteecb->parent;
	pteecb->num_siblings = TEAMINFO(pteecb)->num_children;
	pteecb->thread_num   = tnode->rtid;  /* Thread id within the team */
	pteecb->level        = me->level + 1;               /* Same level */
	pteecb->activelevel  = me->activelevel + 1;         /* OpenMP 3.0 */

	/* Check whether my pools need to be reallocated */
	task_pools_init(pteecb);

	/* OpenMP 3.1 */
	if (pteecb->activelevel < ort->set_nthrlevs)
		tnode->icvs.nthreads = ort->nthr_per_level[pteecb->activelevel];

	/* OpenMP 4.0 */
	if (pteecb->activelevel-1 < ort->set_bindlevs)
		tnode->icvs.proc_bind = ort->bind_per_level[pteecb->activelevel-1];

	/* Save # of children in order to use it in barrier task wait */
	pteecb->parent->tasking.queue_table[pteecb->thread_num].implicit_task_children
		= &(tnode->num_children);

	__SETMYCB(pteecb);                        /* Change my cb */
	__CURRTASK(pteecb) = tnode;               /* Change my current task */ 
	(tnode->func)(pteecb->sdn->shared_data);  /* Execute the special task */
	/* I help my siblings in a lightweight barrier at the end of __func__ */
	/* The counter of special task's father is handled in task_execute */

	__SETMYCB(me);                            /* Restore my actual cb */
}


/* A new implicit task must be allocated in order for parallel for --> task 
 * to run immediately.s
 */
static inline
void spftask_immediate(ort_eecb_t *me,void*(*func)(void*),void*arg,int rtid)
{
	ort_eecb_t *pteecb;
	ort_task_node_t *tnode = ort_task_alloc(func, arg); /* Implicit task node */

#if !defined(HAVE_ATOMIC_FAA)
	ee_init_lock((ee_lock_t *) &(tnode->lock), ORT_LOCK_NORMAL); 
#endif

	tnode->func              = func; /* I have already saved the task func args */
	tnode->num_children      = 0;
	tnode->next              = NULL;
	tnode->parent            = __CURRTASK(me);
	tnode->icvs              = tnode->parent->icvs;
	tnode->inherit_task_node = 0;
	tnode->rtid              = rtid;
	tnode->dependencies      = NULL;

	if (me->me_partask == NULL)
		me->me_partask = (ort_eecb_t *) ort_calloc_aligned(sizeof(ort_eecb_t),NULL);
	pteecb = me->me_partask;

	pteecb->parent           = me;
	pteecb->sdn              = me;
	pteecb->mf->num_children = 0;
	pteecb->num_siblings     = me->mf->num_children;
	pteecb->thread_num       = rtid;           /* Thread id within the team */
	pteecb->level            = me->level + 1;                 /* Same level */
	pteecb->activelevel      = me->activelevel + 1;           /* OpenMP 3.0 */

	/* My current task has now a new child */
	__CURRTASK(me)->num_children++;

	/* Check whether my pools need to be reallocated */
	task_pools_init(pteecb);

	/* OpenMP 3.1 */
	if (pteecb->activelevel < ort->set_nthrlevs)
		tnode->icvs.nthreads = ort->nthr_per_level[pteecb->activelevel];

	/* OpenMP 4.0 */
	if (pteecb->activelevel-1 < ort->set_bindlevs)
		tnode->icvs.proc_bind = ort->bind_per_level[pteecb->activelevel-1];

	/* Save # of children in order to use it in barrier task wait */
	me->tasking.queue_table[rtid].implicit_task_children = &(tnode->num_children);

	__SETMYCB(pteecb);                        /* Change my cb */
	__CURRTASK(pteecb) = tnode;               /* Change my current task */ 
	func(pteecb->sdn->shared_data);           /* Execute the special task */
	/* I help my siblings in a lightweight barrier at the end of __func__ */

	__CURRTASK(me)->num_children--;   /* A child less for my current task */
	__SETMYCB(me);                            /* Restore my actual cb */
	ort_task_free(me, tnode);
}


/* Create special tasks (instead of threads) to execute a nested combined
 * parallel workshare region.
 * Each of the ntasks tasks gets an id from 0 to ntasks-1, offset by offs.
 */
void spftasks_create(ort_eecb_t *me, int ntasks, int offs, void *(*func)(void*))
{
	int i;
	pf_task_args_t *task_args;
	
#ifdef ORT_DEBUG
	ort_debug_thread("about to change eecb and create %d tasks\n", nthr+1);
#endif
	for (i = 0; i < ntasks; i++)
	{
		/* Allocate space */
		task_args = (pf_task_args_t *) 
		            ort_taskenv_alloc(sizeof(pf_task_args_t), func);
		task_args->arg = (void *) me;

		if (ort_task_throttling())
			         spftask_immediate(me, func, (void *) task_args, i+offs);
		else
		{
			ort_task_worker_enqueue(me, 
			         ort_task_alloc_init(func, (void *) task_args, 0, i+offs, me));
			testnotset(me->parent->tasking.never_task);
		}
	}
}
