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

inline int ort_pending_tasks_left(private_eecb_t *me)
{
	int i;

	for (i = 0; i < me->num_siblings; i++)
		if ((me->parent->tasking.pending_tasks[i] != NULL)
		    && (*(me->parent->tasking.pending_tasks[i]) > 0))
			return 1;

	return 0;
}

/*
 * mode = 0 --> Execute only my tasks
 * mode = 1 --> Execute all tasks
 */
void ort_execute_tasks(private_eecb_t *me, int mode)
{
	ort_task_node_t *task_to_execute, *currtask = __CURRTASK(me);

	while (1)
	{
		while (1)
		{
			/* First find a task, lock the task queue */
			ee_set_lock(&(me->parent->tasking.lock));

			if (me->parent->tasking.free_cell > 0)
			{
				me->parent->tasking.free_cell--;

				task_to_execute = me->parent->tasking.tasks[me->parent->tasking.free_cell];
				ee_unset_lock(&(me->parent->tasking.lock));

				__CURRTASK(me) = task_to_execute;

				/* Got a task! Now lets execute it */
				(task_to_execute->func)(task_to_execute->funcarg);

				__CURRTASK(me) = currtask;

				/* Task executed, inform parent task about it */
				ee_set_lock(&(task_to_execute->parent->lock));
				task_to_execute->parent->num_children--;
				ee_unset_lock(&(task_to_execute->parent->lock));

				task_to_execute->status = 1;
			}
			else
			{
				/* Task queue is empty now... */
				ee_unset_lock(&(me->parent->tasking.lock));
				break;
			}
		}

		/* Check whether a sibling is still executing a task... */
		if (mode == 1 && !ort_pending_tasks_left(me)) break;
		/* ... or if all my tasks have been executed */
		else
			if (mode == 0 && __CURRTASK(me)->num_children == 0) break;
	}
}

#else

int ort_pending_tasks_left(private_eecb_t *me)
{
	return 0;
}

void ort_execute_tasks(private_eecb_t *me, int mode)
{
}

#endif