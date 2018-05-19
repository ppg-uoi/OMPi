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

#include "e_lib.h"
#include "shared_data.h"
#include "device_globals.h"

#ifdef LOCK_TASK_QUEUE

void ort_new_task(void *(*func)(void *arg), void *arg, int final, int untied)
{
	int i;
	private_eecb_t  *me = __MYCB;
	ort_task_node_t *current_task = __CURRTASK(me);

	/* Check whether I have to execute this task immediately: */
	/*  a. I execute a final task */
	if (current_task->isfinal == 1)
	{
		/* I must execute the task immediately */
		/* TODO: immediate task node for icvs */
		(*func)(arg);
		return;
	}

	/*  b. Find an empty task node in my pool */
	for (i = 0; i < TASK_QUEUE_SIZE; i++)
		if (me->tasking.tpool[i].status < 2)
			break;

	if (i < TASK_QUEUE_SIZE)
	{
		/* Found an empty task node */
		/*  c. Now find an empty space in task queue */
		/* Lock task queue */
		ee_set_lock(&(me->parent->tasking.lock));
		if (me->parent->tasking.free_cell < TASK_QUEUE_SIZE)
		{
			/* There is space in task queue, first create the task */
			if (me->tasking.tpool[i].status == 0)
				ee_init_lock(&(me->tasking.tpool[i].lock), 0);

			me->tasking.tpool[i].func = func; /* Local address of func */
			me->tasking.tpool[i].funcarg =
			  ort_e_get_global_address(me->core, arg); /* Global address here */
			me->tasking.tpool[i].parent =
			  ort_e_get_global_address(me->core, current_task); /* Global address here */
			me->tasking.tpool[i].isfinal = final;
			me->tasking.tpool[i].num_children = 0;
			me->tasking.tpool[i].status = 2; /* Occupied task node... */

			/* One more child for my current task */
			/* Lock my current task */
			ee_set_lock(&(current_task->lock));
			current_task->num_children++;
			ee_unset_lock(&(current_task->lock));

			/* Now enqueue the task */
			me->parent->tasking.tasks[me->parent->tasking.free_cell] =
			  ort_e_get_global_address(
			    me->core, &(me->tasking.tpool[i])); /* Global address here */

			me->parent->tasking.free_cell ++;
			ee_unset_lock(&(me->parent->tasking.lock));
		}
		else
		{
			ee_unset_lock(&(me->parent->tasking.lock));
			/* Task queue is full, have to execute the task immediately */
			/* TODO: immediate task node for icvs */
			(*func)(arg);
		}
	}
	else
	{
		/* No empty task node found, have to execute the task immediately */
		/* TODO: immediate task node for icvs */
		(*func)(arg);
	}
}

#else

void ort_new_task(void *(*func)(void *arg), void *arg, int final, int untied)
{
}

#endif

