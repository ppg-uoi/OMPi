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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "../../../host/targdenv.h"
#include "memory.h"

/* #define DEBUG */

#define ALLOC_ITEMS_CAPACITY 64

typedef struct {
	void *data; /* The allocated memory */
} alloc_item_t;

/* alloc_items is a dynamically allocated array that holds all allocated
 * memory. Only slave processes need to truly allocate memory. Master just
 * registers (marking index as used). This is happening so master and
 * slaves can know which indexes are used without needing to communicate
 * with each onther. The index of an element is its mediary address, while
 * its value is a struct that holds its actual address. At the
 * **BEGINNING** of the array reside variables that exist in #omp declare
 * target link clause(if any). For these global variables we don't allocate
 * new space; we just store their real address.
 */
static alloc_item_t **alloc_items; /* alloc_items PER DEVICE */
static int *alloc_items_capacity;  /* Total allocated space PER DEVICE */
static int *global_vars_index;     /* Where next global var should be stored PER DEVICE */
static int *next_available_index;  /* To reduce searching time when adding PER DEVICE */
static int num_devices;            /* Number of devices (and elements for each array) */
static int is_master;              /* Master only registers; slaves allocate as well */


static
void dbg(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "  DBG(memory@%d): ", getpid());
	vfprintf(stderr, format, ap);
	va_end(ap);
}


/* Make sure there is enough space to store the requested index at the
 * requested device. Allocate more space if necessary.
 */
static
void check_space(int devid, int index)
{
	int new_capacity;

	if (index >= alloc_items_capacity[devid])
	{
		/* new_capacity = MAX(next_addr, current_capacity+ALLOC_ITEMS_CAPACITY) */
		new_capacity = (index >= alloc_items_capacity[devid] + ALLOC_ITEMS_CAPACITY) ?
			index + 1 : alloc_items_capacity[devid] + ALLOC_ITEMS_CAPACITY;
		alloc_items[devid] = realloc(alloc_items[devid], new_capacity * sizeof(alloc_item_t));
		if (!alloc_items[devid])
		{
			perror("check_space");
			exit(EXIT_FAILURE);
		}
		/* zero-out new elements */
		memset(alloc_items[devid] + alloc_items_capacity[devid], 0,
				(new_capacity - alloc_items_capacity[devid]) * sizeof(alloc_item_t));
		alloc_items_capacity[devid] = new_capacity;
	}
}


/* Make sure that next_available_index points to a valid and unused
 * position.
 */
static
void update_available_index(int devid, int start_address)
{
	next_available_index[devid] = start_address;
	while (next_available_index[devid] < alloc_items_capacity[devid] &&
			alloc_items[devid][next_available_index[devid]].data)
	{
		++(next_available_index[devid]);
	}
	check_space(devid, next_available_index[devid]);
}


static
void alloc_items_add_global_var_at(int devid, void *hostvar, int maddr)
{
	check_space(devid, maddr);
	alloc_items[devid][maddr].data = hostvar;
	/* global_vars_index = MAX(global_vars_index, maddr + 1) */
	global_vars_index[devid] = (global_vars_index[devid] > maddr + 1) ?
		global_vars_index[devid] : maddr + 1;
	next_available_index[devid] = global_vars_index[devid];
#ifdef DEBUG
	dbg("ADDED GLOBAL maddr %d hostvar %p to dev %d\n", maddr, hostvar, devid);
#endif
}


void alloc_items_init(int number_of_devices)
{
	int i;
	num_devices = number_of_devices;

	alloc_items = calloc(num_devices, sizeof(alloc_item_t *));
	if (!alloc_items)
	{
		perror("alloc_items_init");
		exit(EXIT_FAILURE);
	}

	alloc_items_capacity = calloc(num_devices, sizeof(int));
	if (!alloc_items_capacity)
	{
		perror("alloc_items_init");
		exit(EXIT_FAILURE);
	}

	global_vars_index = calloc(num_devices, sizeof(int));
	if (!global_vars_index)
	{
		perror("alloc_items_init");
		exit(EXIT_FAILURE);
	}

	next_available_index = malloc(num_devices * sizeof(int));
	if (!next_available_index)
	{
		perror("alloc_items_init");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < num_devices; ++i)
	{
		next_available_index[i] = -1;
	}
}


void alloc_items_init_global_vars(int devid, int is_master_)
{
	tdenv_t env = tdenv_global_env(devid);
	tditem_t item;
	int i, index;

	is_master = is_master_; /* we need this for free_all function */

	/* Master process has an alloc_items array per device; it must add the
	 * global vars at the correct position. Slave processes have a single
	 * array (since they care only about themselves); they must add the
	 * global vars at position 0.
	 */
	index = (is_master_) ? devid : 0;

	/* When traversing the hash table, different processes are not
	 * guaranteed to visit the items in the same order. That is why in
	 * ort_decltarg_register() we assign each variable a unique
	 * init_imedaddr. All processes call ort_decltarg_register() at the
	 * beginning of their execution in the same order (since all processes
	 * run the same executable), so variables get the same init_imedaddr
	 * among processes. Then here we use init_imedaddr as an index to
	 * alloc_items table.
	 */
	for (i = 0; i < STSIZE; i++)
		for (item = env->table[i]; item; item = item->bucketnext)
			alloc_items_add_global_var_at(index, item->hostvar, item->init_imedaddr);
}


size_t alloc_items_get_global(int devid, void *addr)
{
	int i;

	for (i = 0; i < global_vars_index[devid]; ++i)
	{
		if (alloc_items[devid][i].data == addr)
		{
			return i;
		}
	}
#ifdef DEBUG
	dbg("alloc_items_get_global var %p does not exist.\n", addr);
	abort(); /* abort() crashes with a usefull core dump */
#endif
}


void *alloc_items_add_helper(int devid, size_t maddr, size_t size, int do_alloc)
{
	check_space(devid, maddr);
	alloc_items[devid][maddr].data = (do_alloc) ? calloc(size, 1) : (void *)0x999;
	if (!alloc_items[devid][maddr].data)
	{
		perror("alloc_items_add");
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	dbg("%s maddr %d of size %d to %p to dev %d\n",
			(do_alloc) ? "ADDED" : "REGISTERED", maddr, size,
			alloc_items[devid][maddr].data, devid);
#endif
	return alloc_items[devid][maddr].data;
}


size_t alloc_items_register(int devid)
{
	size_t maddr = next_available_index[devid];

	if (next_available_index[devid] == -1) /* first time initialization */
	{
		update_available_index(devid, 0);
		maddr = next_available_index[devid] = 1; /* leave 0 empty for NULL address */
	}
	alloc_items_add_helper(devid, maddr, 0, 0);
	update_available_index(devid, maddr + 1);

	return maddr;
}


void *alloc_items_add(size_t maddr, size_t size)
{
	return alloc_items_add_helper(0, maddr, size, 1);
}


void alloc_items_remove_helper(int devid, int maddr, int do_remove)
{
	if (maddr >= global_vars_index[devid]) /* cannot free a global variable */
	{
		if (do_remove)
		{
			free(alloc_items[devid][maddr].data);
		}
		alloc_items[devid][maddr].data = NULL;
		next_available_index[devid] = (next_available_index[devid] < maddr) ?
			next_available_index[devid] : maddr;
#ifdef DEBUG
		dbg("%s maddr %d, next_available_index is %d on dev %d\n",
				(do_remove) ? "DELETED" : "UNREGISTERED", maddr,
				next_available_index[devid], devid);
#endif
	}
}


void alloc_items_unregister(int devid, size_t maddr)
{
	alloc_items_remove_helper(devid, maddr, 0);
}


void alloc_items_remove(size_t maddr)
{
	alloc_items_remove_helper(0, maddr, 1);
}


void *alloc_items_get(size_t maddr)
{
#ifdef DEBUG
	if ((int)maddr >= alloc_items_capacity[0])
	{
		dbg("alloc_items_get maddr %d is bigger than %d on dev %d\n",
				maddr, alloc_items_capacity[0], 0);
		abort();
	}
#endif

	return alloc_items[0][maddr].data;
}


void alloc_items_free_all(void)
{
	int i, devid, total = 0;

	for (devid = 0; devid < num_devices; devid++)
	{
		/* don't try to free a global variable (because it was never malloced) */
		for (i = global_vars_index[devid]; i < alloc_items_capacity[devid]; ++i)
		{
			if (alloc_items[devid][i].data)
			{
				if (!is_master)
				{
					free(alloc_items[devid][i].data); /* only slave does alloc */
				}
				++total;
			}
		}
		free(alloc_items[devid]);
	}

	free(alloc_items);
	alloc_items = NULL;
	free(alloc_items_capacity);
	alloc_items_capacity = NULL;
	free(global_vars_index);
	global_vars_index = NULL;
	free(next_available_index);
	next_available_index = NULL;
#ifdef DEBUG
	dbg("CALLED alloc_items_free_all, freed %d items\n", total);
#endif
}
