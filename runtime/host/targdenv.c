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

/* targdenv.c -- handling target (device) data environments */
/* See DOI:10.1007/978-3-319-24595-9_15 (IWOMP 2015) for details of older
 * implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "targdenv.h"
#include "ort_prive.h"


/*
 * Data environment mechanism (hash tables)
 */


/* We have an intepretation problem:
 * Is a device environment globally visible by all threads/tasks??
 * If so, imagine a variable shared among the threads of a team.
 * A thread can arbitrarily #enter/#exit this variable to/from the device 
 * environment, affecting kernel executions of the other threads,
 * possibly leading to all kinds of problems.
 * /tasks may enter
 * Should the global device environments remain unchanged till the end of a 
 * task or can they change arbitarily? 
 * In the first case, a new environment hierarchy can start off as a copy 
 * of the global environment and stick to it. Any changes made by other
 * tasks are not visible.
 * In the second case, the global environment may change dynamically and 
 * thus it should be kept in 1 copy and handled differently.
 * We choose the second...
 */

/* There is a global environment per device (as opposed to an environment
 * hierarchy per task). It has a "depth" field of 0.
 * 
 * A global environment contains 3 types of entities: a) #declare target TO 
 * variables, b) #declare target LINK variables and c) other user-injected 
 * variables (such as #target enter data ones) which have to be ejected 
 * manually by the user as well. We use the "decltarg" field to mark the first 
 * two types of entities and the "bylink" field to differentiate among them.
 * 
 * Declare-target TO variables are always mapped. However, to avoid unnecessary
 * mappings to devices that may never be used, we actually follow a lazy
 * policy where the actual mapping and initialization is left for the first 
 * time the variable is accessed/queried. We use the "imedaddr" field to 
 * determine whether the variable has been actually mapped and initialized 
 * (imedaddr != NULL) or not.
 * 
 * Declare-target LINK variables are always unmapped unless the user maps them 
 * explicitly in #target or #target data constructs (and they are unmapped
 * automatically at the end of this construct) or #target enter constructs. 
 * We use the "imedaddr" field to determine if they are currently mapped 
 * (imedaddr != NULL) or not.
 * 
 * Since a global environment is shared among all tasks that use it,
 * it has to be protected by locks. We do not lock here; we leave it for
 * higher levels.
 */
static tdenv_t globalenvs;     /* The global device environments */

/* The global (depth=0) data environment for all devices */
void tdenv_init_globals()
{
	int i;

	globalenvs = (tdenv_t) ort_calloc(ort->num_devices * sizeof(struct tdenv_s));
	for (i = 0; i < ort->num_devices; i++)
		globalenvs[i].devid = i;
}


tdenv_t tdenv_global_env(int devid)
{
	return (&globalenvs[devid]);
}


tditem_t tdenv_global_get(int devid, void *hostvar)
{
	return ( tdenv_get(&globalenvs[devid], devid, hostvar) );
}


/** 
 * A new hash table (a tree node in the data environment hierarchy). 
 * It starts with a copy of all parent entries.
 * @param tdvars The maximum number of new entries (guaranteed by the compiler)
 * @param parent The parent data environment node
 * @param devid  The device it refers to
 * @return       The new table
 */
tdenv_t tdenv_start(int tdvars, tdenv_t parent, int devid)
{
	tdenv_t t = (tdenv_t) ort_alloc(sizeof(struct tdenv_s));

	if (parent == NULL)
	{
		/* If the global environment was to stick, we would use this instead:
		 * memcpy(&(t->table),&(globalenvs[devid]->table),STSIZE*sizeof(tditem_t));
		 */
		memset(t->table, 0, STSIZE * sizeof(tditem_t)); 
		t->depth = 1;                 /* Keep 0 depth for global env */
	}
	else
	{
		memcpy(&(t->table), &(parent->table), STSIZE * sizeof(tditem_t));
		t->depth = parent->depth + 1;
	}

	/* Allocate space (a pool) for new entries */
	t->entries = (tditem_t) ort_calloc(tdvars * sizeof(struct tditem_s));
	t->num_of_entries = tdvars;     /* max # symbols to be used */
	t->used_entries = 0;            /* actual # symbols used */
	t->devid = devid;
	t->parent = parent;             /* my parent in the tree */
	return (t);
}


/** 
 * Remove the hash table from the data environment tree.
 * @param t        The table
 * @param dev_free The function to free each corresponding table entry 
 *                 in the device
 */
void tdenv_end(tdenv_t t, void (*dev_free)(void *, void *, int))
{
	ort_device_t *d = ort_get_device(t->devid);
	int          i;

	assert(t->depth > 0);
	if (dev_free)
		for (i = 0; i < t->used_entries; i++)
			if (t->entries[i].noalloc == 0)       /* alloced elsewhere */
				;//dev_free(d->device_info, t->entries[i].imedaddr, 0);
	free(t->entries);
	free(t);
}


void tdenv_show(tdenv_t t)
{
	tditem_t e;
	tdenv_t  p;
	int      i;

	for (p = t; p != NULL; p = p->parent)
	{
		if (p->depth == 0) 
			break;            /* No globals for now */
		printf("\nScope (depth %d):\n------------------\n", p->depth);
		for (i = 0, e = p->entries; (i < p->used_entries) && e; i++, e++)
			printf("%*svar = %p, haddr = %p, iaddr = %p, refs=%d (dev %d)\n", 
			       2*(t->depth-p->depth+1), " ", e->hostvar, e->hostaddr, 
			       e->imedaddr, e->refctr, e->de->devid);
	}
}


/*
 * Data environment items
 */


/* Get the next available entry from the preallocated entry pool.
 * The pool has enough space for all variables (guaranteed by the compiler)
 */
static inline tditem_t tditem_new(tdenv_t t)
{
	if (t->depth)
	{
		assert(t->used_entries < t->num_of_entries);   /* sanity */
		return ((tditem_t) &(t->entries[t->used_entries++]));
	}
	else
	{
		t->used_entries++;
		return (tditem_t) ort_calloc(sizeof(struct tditem_s));
	}
}


/**
 * Add a symbol (hostvar) to the table.
 * @param t        The data environment (hash table)
 * @param hostvar  The item (same as hostaddr except for pointers)
 * @param hostaddr The item's base host address
 * @param seclen   The item's section size in bytes
 * @param secoffst Where the section starts from (bytes wrt to hostaddr)
 * @param imedaddr The mediary address hostaddr maps to
 * @return         The new item
 */
tditem_t tdenv_put(tdenv_t t, void *hostvar, void *hostaddr, size_t secoffst, 
                size_t seclen, void *imedaddr, int noalloc)
{
	tditem_t e, *bucket = &(t->table[((size_t) hostvar) % STSIZE]);
	
	e = tditem_new(t);
	e->de         = t;
	e->hostvar    = hostvar;
	e->hostaddr   = hostaddr;
	e->imedaddr   = imedaddr;
	e->secoffset  = secoffst;
	e->seclen     = seclen;
	e->noalloc    = noalloc;
	e->refctr     = 0;         /* better safe than sorry: is e always calloced? */
	e->decltarg   = 0;
	e->bylink     = 0;
	e->bucketnext = *bucket;
	*bucket = e;
	return (e);
}


/**
 * Gets the mapping item of a symbol (hostvar).
 * We return the most recent entry, not necessarily in the current level.
 * @param t       The environment
 * @param devid   The device id
 * @param hostvar The symbol
 * @return        The entry
 */
tditem_t tdenv_get(tdenv_t t, int devid, void *hostvar)
{
	tditem_t e = t->table[((size_t) hostvar) % STSIZE ];
	
	for ( ; e; e = e->bucketnext)
		if (e->hostvar == hostvar && e->de->devid == devid)
			return (e);
	return (NULL);
}


/**
 * Get the mapping item of a symbol (hostvar), ONLY if it exists in my 
 * target data level.
 * @param t       The environment
 * @param devid   The device id
 * @param hostvar The symbol
 * @return        The entry
 */
tditem_t tdenv_get_currlevel(tdenv_t t, int devid, void *hostvar)
{
	tditem_t e = t->table[((size_t) hostvar) % STSIZE ];
	
	for ( ; e; e = e->bucketnext)
	{
		if (e->de->depth < t->depth) 
			break;
		if (e->hostvar == hostvar && e->de->devid == devid)
			return (e); /* found it */
	}
	return (NULL);
}


/**
 * Removes the most recent appearance of an item from the mapping table.
 * @param t       The environment
 * @param devid   The device id
 * @param hostvar The item
 * @return        The removed entry (be careful, non-globals cannot be freed)
 */
tditem_t tdenv_rem(tdenv_t t, int devid, void *hostvar)
{
	tditem_t e = t->table[((size_t) hostvar) % STSIZE ], f = NULL;
	
	for ( ; e; f = e, e = e->bucketnext)
		if (e->hostvar == hostvar && e->de->devid == devid)  /* Found it */
		{
			if (f)   /* get it off */
				f->bucketnext = e->bucketnext;
			else     /* special case: head of bucket list */
				t->table[((size_t) hostvar) % STSIZE ] = e->bucketnext;
			e->bucketnext = NULL;
			t->used_entries--;
			return (e);
		};
	return (NULL);
}


/**
 * Check if the entry includes the given range (array section). Notice that we 
 * are given a host hostaddress, not a hostvar. Let [a,b] be the given range; 
 * i.e. a = haddr+secoff, b = a+seclen-1. Then we need to find if stored range 
 * [A,B] includes [a,b]. In the code below, a is named "lb" and A is "entrylb".
 * @param e      the entry to check
 * @param haddr  the base address
 * @param seclen the length in bytes
 * @param secoff the offset (from the base address) in bytes
 * @return       RANGE_NEW if this is a completely new range; RANGE_SAMEBASE if
 *               this is a new range but shares the same base address with an
 *               existing item; RANGE_OVERLAPS is it overlaps with an existing
 *               item's range; RANGE_INCLUDED if it is a proper subset of an
 *               existing item's range.
 */
static 
int entry_check_range(tditem_t e, void *haddr, size_t seclen, size_t secoff)
{
	void *lb, *entrylb;

	lb = haddr + secoff;                          /* a */
	entrylb = e->hostaddr + e->secoffset;         /* A */
	if (PointInside(lb, entrylb, entrylb + e->seclen-1))   /* a in [A,B] */
	{
		if (seclen == 0 || PointInside(lb+seclen-1, entrylb, entrylb + e->seclen-1))
			return RANGE_INCLUDED;                             /* b in [A,B] */
		else
			return RANGE_OVERLAPS;                             /* b > B */
	}
	else                                                   /* a < A */
	{
		if (lb <= entrylb && lb+seclen-1 >= entrylb)         /* b > A */
			return RANGE_OVERLAPS;
		else                                                 /* b < A */
			if (haddr == e->hostaddr)  /* item exists but this is new range */
				return RANGE_SAMEBASE;
	}
	return RANGE_NEW;
}


/**
 * Search the data environment hierarchy to see the most recent range (array 
 * section) that includes the given one. Notice that we are given a host
 * hostaddress, not a hostvar. Let [a,b] be the given range; i.e.
 * a = haddr+secoff, b = a+seclen-1. Then we need to find a stored range [A,B]
 * which includes [a,b]. In the code below, a is named "lb" and A is "entrylb".
 * @param t      the current dataenv node
 * @param haddr  the base address
 * @param seclen the length in bytes
 * @param secoff the offset (from the base address) in bytes
 * @param tdi    will point to the existing item (if any)
 * @return       RANGE_NEW if this is a completely new range; RANGE_SAMEBASE if
 *               this is a new range but shares the same base address with an
 *               existing item; RANGE_OVERLAPS is it overlaps with an existing
 *               item's range; RANGE_INCLUDED if it is a proper subset of an
 *               existing item's range.
 */
int tdenv_search_range(tdenv_t t, 
                       void *haddr, size_t seclen, size_t secoff, tditem_t *tdi)
{
	tdenv_t  p;
	tditem_t e;
	int      i, res;

	/* Check hierarchy except global one */
	for (p = t; p != NULL && p->depth > 0; p = p->parent)
		for (i = 0, e = p->entries; (i < p->used_entries) && e; i++, e++)
		{
			if (e->de->devid != t->devid)
				continue;
			if ((res = entry_check_range(e, haddr, seclen, secoff)) != RANGE_NEW)
			{
				tdi && (*tdi = e);
				return (res);
			}
		};
	
	/* Last check: global (decltarg) entries of the given device.
	 * Care taken so that link variables are checked only if actively mapped.
	 * FIXME: TO global vars may not have obtained their address yet!!!
	 * 
	 */
	if (t->depth > 0)
		t = &globalenvs[t->devid];
	for (i = 0; i < STSIZE; i++)
		for (e = t->table[i]; e && (!e->bylink || e->imedaddr); e = e->bucketnext)
			if ((res = entry_check_range(e, haddr, seclen, secoff)) != RANGE_NEW)
			{
				tdi && (*tdi = e);
				return (res);
			};
	
	/* It can only be new then */
	tdi && (*tdi = NULL);
	return RANGE_NEW;
}
