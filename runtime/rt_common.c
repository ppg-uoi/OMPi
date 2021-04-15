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

/*
 * This file contains common functionalities between
 * the host and and the hostparts
 */

#include "rt_common.h"

#define KERNELTABLE_CAPACITY 10

/* kerneltable holds information about every kernel function and its
 * corresponding function pointer. The array is dynamically allocated;
 * every time we run out of space, more KERNELTABLE_CAPACITY positions are
 * allocated.
 */

typedef struct {
	char *kernel_name;
	void *(*kernel_function)(void *);
} kerneltable_t;

kerneltable_t *kerneltable = NULL;
int kerneltable_size = 0;     /* used space */
int kerneltable_capacity = 0; /* total allocated space */

int  **places_dup(int **from)
{
	int i;
	int num_of_places = numplaces(from);
	int **c_places;


	if(num_of_places <=0)
		return NULL;

	c_places = malloc(num_of_places*sizeof(int **));

	c_places[0] = malloc(sizeof(int));
	c_places[0][0] = from[0][0];

	for(i=1; i<=num_of_places; i++)
	{
		c_places[i] = malloc((placelen(from,i-1)+1)*sizeof(int));
		memcpy(c_places[i], from[i], (placelen(from,i-1)+1)*sizeof(int));
	}

	return c_places;
}

void  places_free(int **place)
{
	int n = numplaces(place) - 1;

	if(place == NULL)
		return;
	for (; n >=0; n--)
		if (placelen(place, n))
			free(place[n + 1]);
	free(place);
}

void ort_kerneltable_add(char *name, void *(*kernel_function)(void *))
{
	/* allocate more space if necessary */
	if (kerneltable_size >= kerneltable_capacity)
	{
		kerneltable_capacity += KERNELTABLE_CAPACITY;
		kerneltable = realloc(kerneltable, kerneltable_capacity * sizeof(kerneltable_t));
		if (!kerneltable)
		{
			fprintf(stderr, "Error resizing kernel table. Exiting...\n");
			exit(EXIT_FAILURE);
		}
	}

	/* add new kernel to kerneltable */
	/* name should be global string literal; no need for strcpy */
	kerneltable[kerneltable_size].kernel_name = name;
	kerneltable[kerneltable_size++].kernel_function = kernel_function;
}

int get_kernel_id_from_name(char *name)
{
	int i;

	for (i = 0; i < kerneltable_size; ++i)
	{
		if (!strcmp(kerneltable[i].kernel_name, name))
		{
			return i;
		}
	}

	fprintf(stderr, "Error: Kernel with name \"%s\" does not exist in kernel "
			"table. Exiting...\n", name);
	exit(EXIT_FAILURE);
}

void *(*get_kernel_function_from_id(int id))(void *)
{
	return kerneltable[id].kernel_function;
}

void free_kerneltable(void)
{
	free(kerneltable);
	kerneltable = NULL;
}
