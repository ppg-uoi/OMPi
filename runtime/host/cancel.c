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

/* cancel.c -- OpenMP cancelation-related routines */

#include "ort_prive.h"


/* Check for active cancellation in a for region
 */
static int check_cancel_for(void)
{
	ort_eecb_t *me = __MYCB;

	if (me->parent == NULL)
		return ( me->cancel_for_me );
	else
		if (me->parent != NULL)
			return ( TEAMINFO(me)->cancel_for_active );
	return 0;
}


/* Check for active cancellation in a parallel region
 */
static int check_cancel_parallel(void)
{
	return ( TEAMINFO(__MYCB)->cancel_par_active );
}


/* Check for active cancellation in a sections region
 */
static int check_cancel_sections(void)
{
	ort_eecb_t *me = __MYCB;

	if (me->parent != NULL)
		return ( TEAMINFO(me)->cancel_sec_active );
	else
		return ( me->cancel_sec_me );
	return 0;
}


/* Check for active cancellation in a taskgroup region
 */
static int check_cancel_taskgroup(void)
{
	return ( __CURRTASK(__MYCB)->taskgroup != NULL &&
	         __CURRTASK(__MYCB)->taskgroup->is_canceled );
}
 

/* This function is called when a cancellation point is reached
 */
int ort_check_cancel(int type)
{
	switch (type)
	{
		case 0:
			return check_cancel_parallel();
		case 1:
			return check_cancel_taskgroup();
		case 2:
			return check_cancel_for();
		case 3:
			return check_cancel_sections();
	}
	return 0;
}


/* Function for enabling thread cancellation in a parallel region
 */
static void enable_cancel_parallel(void)
{
	ort_eecb_t *me = __MYCB;

	if (CANCEL_ENABLED())
		if (me->parent != NULL)
			TEAMINFO(me)->cancel_par_active = true;
}


/* Function for enabling thread cancellation in a section region
 */
static void enable_cancel_sections(void)
{
	ort_eecb_t *me = __MYCB;

	if (CANCEL_ENABLED())
	{
		if (me->parent != NULL)
			TEAMINFO(me)->cancel_sec_active = true;
		else
			me->cancel_sec_me = true;
	}
}

/* Function for enabling thread cancellation in a for region
 */
static void enable_cancel_for(void)
{
	ort_eecb_t  *me = __MYCB;
	
	if (CANCEL_ENABLED())
	{
		if (me->parent != NULL)
			TEAMINFO(me)->cancel_for_active = true;
		else
			me->cancel_for_me = true;
	}
}


/* Function for enabling thread cancellation in a taskgroup region
 */
static void enable_cancel_taskgroup(void)
{
	ort_eecb_t  *me = __MYCB;
	if (__CURRTASK(me)->taskgroup != NULL && CANCEL_ENABLED())
		__CURRTASK(me)->taskgroup->is_canceled = true;
}


/* This function is called when #pragma omp cancel is reached
 */
int ort_enable_cancel(int type)
{
	ort_eecb_t *me = __MYCB;

	switch (type)
	{
		case 0:
			enable_cancel_parallel();
			return check_cancel_parallel();
		case 1:
			enable_cancel_taskgroup();
			return check_cancel_taskgroup();
		case 2:
			enable_cancel_for();
			if (me->parent == NULL)
			{
				if (check_cancel_for())
				{
					me->cancel_for_me = 0;
					return 1;
				}
				else
					return 0;
			}
			else
				/* FIXME: shouldn't here be an enable_cancel_for() as in sections?*/
				return ( check_cancel_for() );
		case 3:
			if (me->parent == NULL)
			{
				enable_cancel_sections();
				if (check_cancel_sections())
				{
					me->cancel_sec_me = 0;
					return 1;
				}
				else
					return 0;
			}
			else
			{
				enable_cancel_sections();
				return ( check_cancel_sections() );
			}
	}
}
