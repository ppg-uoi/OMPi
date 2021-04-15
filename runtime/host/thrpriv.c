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

/* thrpriv.c -- OMPi RunTime library; threadprivate variables */

#include <stdlib.h>
#include <string.h>
#include "ort_prive.h"


/* See the internals documentation for the details of the implementations
 */
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
	nkeys = parent->mf->tpkeys[thrid = e->thread_num].alloted;
	vars = parent->mf->tpkeys[thrid].vars;
	if (vid >= nkeys)
	{
		vars = (vars == NULL) ? ort_alloc((vid + 10) * sizeof(void *)) :
		       ort_realloc(vars, (vid + 10) * sizeof(void *));
		if (vars == NULL)
			ort_error(1, "[ort_get_thrpriv]: memory allocation failed\n");
		memset(&vars[nkeys], 0, (vid + 10 - nkeys)*sizeof(void *));
		parent->mf->tpkeys[thrid].alloted = nkeys = vid + 10;
		parent->mf->tpkeys[thrid].vars = vars;
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


/* The interface */
void *ort_get_thrpriv(void **key, int size, void *origvar)
{
	return (get_ee_thrpriv(__MYCB, (int *) key, size, origvar));
}
