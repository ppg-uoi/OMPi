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

/* This file provides an API so both master and slave processes can
 * allocate and access memory. Each slave process holds an array of
 * allocated items. The master process holds a similar array, but no real
 * allocation is happenning (variables already exist at master process).
 * This is happenning so master and slaves can know which elements are
 * being used without the need to communicate with each other. The index
 * of the array where an item is stored is the mediary address of that
 * item. This means that when you want to transfer an address from master
 * to slave (or from slave to master) you must use the mediary address.
 * When you want to actually read/write to that memory from a slave process
 * you must use the allocated mediary address.
 *
 * ***NOTE***: For this to work, you must ensure that both the master and
 * the slave allocate and deallocate the items they commonly use in the
 * same order.
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>

/* Initialize this module. NOTE that this function should be called only
 * once and before any other function in this file is called. */
void alloc_items_init(int number_of_devices);


/* Add global variables that exist in #omp declare target link clause.
 * Master process must call this function with is_master == 1 and slave
 * processes with is_master == 0. NOTE that this function should be called
 * only once and before any other add function is called. */
void alloc_items_init_global_vars(int devid, int is_master);


/* Return the mediary address of the given host address of a global variable. */
size_t alloc_items_get_global(int devid, void *addr);


/* Register (but do not allocate momory for) a variable and return its
 * mediary address. Should be used only by the host. */
size_t alloc_items_register(int devid);


/* Allocate memory for the given size and mediary address. Return a pointer
 * to the allocated memory. Should be used by slaves. */
void *alloc_items_add(size_t maddr, size_t size);


/* Unregister (but do not deallocate memory for) a variable. Should be
 * used only by the host. */
void alloc_items_unregister(int devid, size_t maddr);


/* Deallocate memory with the given mediary address. Should be used by slaves. */
void alloc_items_remove(size_t maddr);


/* Return the real address of the given mediary address. */
void *alloc_items_get(size_t maddr);


/* Free all allocated memory. Should be called once when finalizing.
 * NOTE: No other functions of this file should be called after that. */
void alloc_items_free_all(void);


#endif /* __MEMORY_H__ */
