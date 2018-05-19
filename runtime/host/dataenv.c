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

/* dataenv.c -- structure for device environments */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dataenv.h"
#include "ort_prive.h"

/*
 * Get next available entry from preallocated entry pool
 * and return it...
 * Pool has enough space for all variables
 * (Given by the compilers)
 */
inline static dtentry getdtentry(dataenv t)
{
	assert(t->used_entries < t->num_of_entries); /* Debug */
	return ((dtentry) & (t->entries[t->used_entries++]));
}

static
dtentry Deentry(dataenv d, void *host_addr, dtentry bnext)
{
	dtentry e = getdtentry(d);
	e->host_addr  = host_addr;
	e->dev_addr   = NULL;
	e->bucketnext = bnext;
	e->de         = d;

	return (e);
}

/* Put a symbol with its value in the table */
void *dataenv_put(dataenv d, void *host_addr, void *dev_addr)
{
	dtentry *bucket;
	bucket = &(d->table[((unsigned long int) host_addr) % STSIZE]);
	*bucket = Deentry(d, host_addr, *bucket);
	(*bucket)->dev_addr = dev_addr;

	return (dev_addr);
}


/* Notice that we return the most recent entry, not necessarily in
 * the current scope.
 */
void *dataenv_get(dataenv t, void *host_addr, int device_id)
{
	dtentry e;
	for (e = t->table[((unsigned long int) host_addr) % STSIZE ]; e;
	     e = e->bucketnext)
		if (e->host_addr == host_addr && e->de->device_id == device_id)
			return (e->dev_addr); /* found it */
	return (NULL);
}

/*
 * Same as above only that I am interested only for variables that
 * exist in my target data level.
 */
void *dataenv_get_current_level(dataenv t, void *host_addr, int device_id)
{
	dtentry e;
	for (e = t->table[((unsigned long int) host_addr) % STSIZE ]; e;
	     e = e->bucketnext)
	{
		if (e->de->depth < t->depth) return NULL;

		if (e->host_addr == host_addr && e->de->device_id == device_id)
			return (e->dev_addr); /* found it */
	}

	return (NULL);
}

/* Recycle bin for environment hash tables... */
inline static dataenv get_dataenv(void)
{
	return (dataenv)ort_alloc(sizeof(struct dataenv_));
}

inline static void free_dataenv(dataenv ht)
{
	free(ht);
}
/*
 * An empty tree node table
 * If parent exists then parent
 * pointers are copied to new
 * table
 */
dataenv dataenv_start(int tdvars, dataenv parent, int device_id)
{
	dataenv t = get_dataenv();
	int    i;

	if (parent == NULL)
	{
		for (i = 0; i < STSIZE; i++)
			t->table[i] = NULL;
		t->depth = 0;
	}
	else
	{
		memcpy(&(t->table), &(parent->table), STSIZE * sizeof(dtentry));
		t->depth = parent->depth + 1;
	}

	/* Allocate space (a pool) for new entries; equals to the number
	 * of new target environment variables
	 */
	t->entries = (dtentry)ort_alloc(tdvars * sizeof(struct dtentry_));
	t->num_of_entries = tdvars;
	t->used_entries = 0;
	t->device_id = device_id;
	t->parent = parent;

	return (t);
}

/* Free an environment data... */
void dataenv_end(dataenv t, void (*dev_free)(void *, void *, int))
{
	int i;
	ort_device_t *d = ort_get_device(t->device_id);
	/*
	 * I must free the allocated space of variables.
	 *  - free_entry is the number of registered variables in this scope
	 *  - num_of_new_entries is the maximum number of variables to be registered
	 *    in this scope.
	 */
	for (i = 0; i < t->used_entries; i++)
		dev_free(d, t->entries[i].dev_addr, 0);

	/* free entries */
	free(t->entries);
	/* Recycle hast table... */
	free_dataenv(t);
}

void dataenv_show(dataenv t)
{
	dtentry e;
	dataenv p;
	int     i;

	for (p = t; p != NULL; p = p->parent)
	{
		printf("Current scope (%d):\n--------------\n", p->depth);
		for (i = 0, e = p->entries; (i < p->used_entries
		                                 && e != NULL); i++, e++)
			printf("%*s%p (%d)\n", 2 * (i + 1), " ", e->host_addr,
			       e->de->device_id);
	}
}
