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

/* places.c -- implement OpenMP places & thread binding */

#include <ctype.h>
#include <setjmp.h>
#include "ort_prive.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * GLOBAL VARIABLES / DEFINITIONS / MACROS                           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static char *pstok;                 /* For parsing the places string */
static int _reslist[MAX_PLACE_LEN]; /* Temporary list to hold a place */
static jmp_buf j_placeserr;         /* Error handling */
#define j_places_error(n) longjmp(j_placeserr, n)

typedef enum {UNKNOWN, THREADS, CORES, SOCKETS, NUMADOMAINS, LLCACHES} absplace_t;


#ifndef HAVE_STRNCASECMP

static int strncasecmp(char *s, char *t, int len)
{
	for (; *s && *t && len; len--, s++, t++)
		if (tolower((int) *s) != tolower((int) *t))
			break;
	return ( len ? tolower((int) *s) - tolower((int) *t) : 0 );
}

#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * HARDWARE LOCALITY                                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifdef HAVE_HWLOC

#include <hwloc.h>

// Handle hwloc compatibility issues
#if HWLOC_API_VERSION < 0x00010b00
	#define HWLOC_OBJ_NUMANODE   HWLOC_OBJ_NODE
	#define HWLOC_OBJ_PACKAGE    HWLOC_OBJ_SOCKET
#elif HWLOC_API_VERSION < 0x00020000
	#define PRINT_HWLOC_OBJ(obj) \
		printf("os index: %3d, type: %10s, depth: %2d, arity: %3u\n",    \
				obj->os_index, hwloc_obj_type_string(obj->type), \
				obj->depth, obj->arity);
#else
	#define PRINT_HWLOC_OBJ(obj) \
		printf("os index: %3d, type: %10s, depth: %2d, arity: %3u, mem_arity: %u\n", \
				obj->os_index, hwloc_obj_type_string(obj->type), obj->depth, \
				obj->arity, obj->memory_arity);
#endif

static int places_add(int ***places, int *list, int n, int plen);
static absplace_t _aptype;
static int _curres;
static hwloc_obj_type_t cachetype = -1;
static unsigned int max_cache_depth = 0; // L1, L2, L3 caches have depth 1, 2, 3 respectively.


/* Recursively traverse assymetric hwloc topology tree
 * that definitely contains objects of type _aptype.
 */
static
void traverse_topology(hwloc_topology_t topology, hwloc_obj_t obj,
		int ***places, int plen)
{
	unsigned int i;

	// PRINT_HWLOC_OBJ(obj);   // Print each hwloc object while traversing topology

	if (hwloc_compare_types(obj->type, HWLOC_OBJ_PU) == 0)
	{
		_reslist[_curres++] = obj->os_index;
		if (_aptype == THREADS)
		{
			if (places_add(places, _reslist, _curres, plen) == -1)
				return;
			_curres = 0;
		}
	}

#if HWLOC_API_VERSION < 0x00020000
	if (_aptype == LLCACHES && hwloc_compare_types(obj->type, cachetype) == 0)
	{
		if (max_cache_depth < obj->attr->cache.depth)
			max_cache_depth = obj->attr->cache.depth;
	}
#endif

	for (i = 0; i < obj->arity; i++)
		traverse_topology(topology, obj->children[i], places, plen);

	if (_aptype == SOCKETS && hwloc_compare_types(obj->type, HWLOC_OBJ_PACKAGE) == 0)
	{
		if (places_add(places, _reslist, _curres, plen) == -1)
			return;
		_curres = 0;
	}
	else if (_aptype == CORES && hwloc_compare_types(obj->type, HWLOC_OBJ_CORE) == 0)
	{
		if (places_add(places, _reslist, _curres, plen) == -1)
			return;
		_curres = 0;
	}
	else if (_aptype == LLCACHES && hwloc_compare_types(obj->type, cachetype) == 0)
	{
#if HWLOC_API_VERSION < 0x00020000
		if (max_cache_depth != obj->attr->cache.depth)
			return;
#endif
		/* IMPORTANT: cachetype is not initialized unless _aptype == LLCACHES */
		if (places_add(places, _reslist, _curres, plen) == -1)
			return;
		_curres = 0;
	}
#if HWLOC_API_VERSION < 0x00020000
	else if (_aptype == NUMADOMAINS && hwloc_compare_types(obj->type, HWLOC_OBJ_NUMANODE) == 0)
	{
		if (places_add(places, _reslist, _curres, plen) == -1)
			return;
		_curres = 0;
	}
#else
	if (obj->memory_arity == 1)
	{
		if (hwloc_compare_types(obj->memory_first_child->type, HWLOC_OBJ_NUMANODE) == 0 \
                        && _aptype == NUMADOMAINS)
		{
			if (places_add(places, _reslist, _curres, plen) == -1)
				return;
			_curres = 0;
		}
	}
#endif
}


/* Before hwloc v2.0 there was only HWLOC_OBJ_CACHE. Since v2.0,
 * 8 types were added. L1 to L5 data (or unified) and L1i to L3i
 * (instruction) caches. This function finds the type of the last
 * (lower) level cache object (i.e. HWLOC_OBJ_L3CACHE).
 */
static
hwloc_obj_type_t get_ll_cache_obj_type(hwloc_topology_t topology)
{
#if HWLOC_API_VERSION < 0x00020000
	return HWLOC_OBJ_CACHE;
#else
	hwloc_obj_type_t cachetype;
	for (cachetype = HWLOC_OBJ_L5CACHE; cachetype >= HWLOC_OBJ_L1CACHE; cachetype--)
		if (hwloc_get_nbobjs_by_type(topology, cachetype) != 0)
			return cachetype;
	return HWLOC_OBJ_L1CACHE; // Doesn't exist but use as fallback.
#endif
}


/* Traverse hwloc topology tree as of object type in case all objects
 * (of the same type) exist in the same level, or invoke traverse_topology()
 * to handle assymetric topologies.
 */
static
void traverse_topology_by_type(hwloc_topology_t topology, int ***places, int plen)
{
	hwloc_obj_type_t type;
	hwloc_obj_t currobj;
	int num_objs;
	unsigned int os_index;

	switch (_aptype)
	{
		case THREADS:
			type = HWLOC_OBJ_PU;
			break;
		case CORES:
			type = HWLOC_OBJ_CORE;
			break;
		case SOCKETS:
			type = HWLOC_OBJ_SOCKET;
			break;
		case NUMADOMAINS:
			type = HWLOC_OBJ_NUMANODE;
			break;
		case LLCACHES:
			type = (cachetype = get_ll_cache_obj_type(topology));
			break;
	}

	num_objs = hwloc_get_nbobjs_by_type(topology, type);
	if (num_objs == -1) // Objects of this type exist but in different levels
	{
		traverse_topology(topology, hwloc_get_root_obj(topology),
				places, plen);
		return;
	}
	else if (num_objs == 0) // No objects of this type exist
	{
		_aptype = THREADS; // HWLOC_OBJ_PUs always exist
		traverse_topology_by_type(topology, places, plen);
		return;
	}

	/* Iterate all objects of _aptype */
	currobj = hwloc_get_next_obj_by_type(topology, type, NULL);
	for (; currobj != NULL; currobj = currobj->next_cousin)
	{
		// PRINT_HWLOC_OBJ(currobj);

		/* Iterate the cpuset of current object */
		hwloc_bitmap_foreach_begin(os_index, currobj->cpuset)
		{
			_reslist[_curres++] = os_index;
		}
		hwloc_bitmap_foreach_end();
		if (places_add(places, _reslist, _curres, plen) == -1)
			return;
		_curres = 0;
	}

}


/* Initialize place_partition using abstract place names
 * and hwloc topology tree.
 */
static
void places_aname_from_topology(int ***places, absplace_t aptype, int plen)
{
	hwloc_topology_t topology;

	if (hwloc_topology_init(&topology) < 0)
		j_places_error(5);

#if HWLOC_API_VERSION >= 0x00020000
	/* Root, PU and NUMA objects cannot be fitlered-out.
	 * Cores, Packages (ex Sockets) and Caches should be explicitly filtered-in.
	 */
	hwloc_topology_set_all_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_NONE);
	hwloc_topology_set_type_filter(topology, HWLOC_OBJ_PACKAGE, HWLOC_TYPE_FILTER_KEEP_ALL);
	hwloc_topology_set_type_filter(topology, HWLOC_OBJ_CORE, HWLOC_TYPE_FILTER_KEEP_ALL);
	hwloc_topology_set_cache_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_ALL);
#else
	hwloc_topology_ignore_type(topology, HWLOC_OBJ_GROUP);
	hwloc_topology_ignore_type(topology, HWLOC_OBJ_MISC);
#endif

	if (hwloc_topology_load(topology) < 0)
	{
		hwloc_topology_destroy(topology);
		j_places_error(5);
	}

	_aptype = aptype;
	_curres = 0;
	traverse_topology_by_type(topology, places, plen);

	hwloc_topology_destroy(topology);
}

#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * OMP_PLACES PARSER -- see rtcommon.h                               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Add the processor IDs located in 'list' of length 'n'
 * to a new place, with respect to the maximum number of
 * places allowed by the user ('plen').
 */
static
int places_add(int ***places, int *list, int n, int plen)
{
	int p;

	if (numplaces((*places)) == plen)
		return -1; // No more places to add

	if (numplaces((*places)) == 0)
	{
		(*places) = (int **) malloc(2 * sizeof(int *));
		if (!(*places))
			j_places_error(2);
		(*places)[0] = (int *) malloc(1 * sizeof(int));
		if (!(*places)[0])
			j_places_error(2);
		(*places)[0][0] = 0;
	}
	else
	{
		(*places) = (int **) realloc((*places), ((*places)[0][0] + 2) * sizeof(int *));
		if (!(*places))
			j_places_error(2);
	}
	p = ++(*places)[0][0];
	(*places)[p] = malloc((n + 1) * sizeof(int));
	if (!(*places)[p])
		j_places_error(2);
	(*places)[p][0] = n;
	memcpy(&(*places)[p][1], list, n * sizeof(int));

	return 0;
}


static
void places_remove(int ***places, int *list, int n)
{
	int i, j;

	if (numplaces(*places) == 0)
		return;
	for (i = 0; i < numplaces(*places); i++)
		if (placelen(*places, i) == n)          /* Compare place with list */
		{
			for (j = 0; j < placelen(*places, i); j++)
				if ((*places)[i + 1][j + 1] != list[j])
					break;
			if (j == placelen(*places, i))        /* Found a match; remove place. */
			{
				--(*places)[0][0];                  /* 1 less place */
				free((*places)[i+1]);               /* Free row */
				for (; i < numplaces(*places); i++) /* Push back */
					(*places)[i+1] = (*places)[i+2];
				if (numplaces(*places) == 0)
				{
					free(*places);
					*places = NULL;
				}
			}
		};
}


/* Require a positive (sign>0), non-negative (sign=0) or any (sign<0) integer.
 */
static
int places_integer(int sign)
{
	int p;

	if (sscanf(pstok, "%d", &p) != 1)
		j_places_error(4);
	if ((sign > 0 && p <= 0) || (sign == 0 && p < 0))
		j_places_error(1);
	if (p < 0)
		pstok++;                       /* skip the sign */
	for (; isdigit(*pstok); pstok++)   /* advance pstok ptr */
		;
	return (p);
}


/* res-interval := res ':' num ':' stride | res ':' num | res | !res
 */
static
void places_res_interval(int *res, int *num, int *stride, int *exclude)
{
	if (*pstok == '!')
	{
		pstok++;
		*exclude = 1;
		*res = places_integer(0);
		return;
	}

	*exclude = 0;
	*num = 1;
	*stride = 1;
	*res = places_integer(0);
	if (*pstok == ':')
	{
		pstok++;
		*num = places_integer(1);
		if (*pstok == ':')
		{
			pstok++;
			*stride = places_integer(-1);
		}
	}
	/* All numbers in the interval should be >= 0 */
	if (*res + *stride * (*num - 1) < 0)
		j_places_error(1);
}


/* res-list := res-interval | res-list ',' res-interval
 */
static
int places_res_list()
{
	int n, i, j;
	int res, num, stride, exclude;

	for (n = 0; ; pstok++ /* Skip comma */)
	{
		places_res_interval(&res, &num, &stride, &exclude);
		if (exclude)
		{
			/* Search and remove res */
			for (i = n-1; i >= 0; i--)
				if (_reslist[i] == res)
					for (--n, j = i; j < n; j++)   /* push back */
						_reslist[j] = _reslist[j+1];
		}
		else
		{
			/* Add interval to the list */
			if (n + num > MAX_PLACE_LEN)
				j_places_error(3);
			for (i = 0; i < num; i++)
				_reslist[n++] = res + i*stride;
		}
		if (*pstok != ',')
			break;
	}
	return (n);
}


/* place := '{' res-list '}'
 */
static
int places_place()
{
	int n;

	if (*pstok != '{')
		j_places_error(1);
	pstok++;
	n = places_res_list();
	if (*pstok != '}')
		j_places_error(1);
	pstok++;
	return (n);
}


/* p-interval := place ':' len ':' stride | place ':' len | place | '!' place
 */
static
void places_p_interval(int ***places)
{
	int n, len = 1, stride = 1, p, i;

	if (*pstok == '!')   /* exclude a place */
	{
		pstok++;
		if ( (n = places_place()) > 0 )
			places_remove(places, _reslist, n);
		return;
	}

	n = places_place();

	if (*pstok == ':')
	{
		pstok++;
		len = places_integer(1);
		if (*pstok == ':')
		{
			pstok++;
			stride = places_integer(-1);
		}
	}

	if (n)
		for (p = 0; p < len; p++)         /* Construct len places */
		{
			if (p)
				for (i = 0; i < n; i++)       /* Add stride to previous place elems */
				{
					_reslist[i] += stride;
					if (_reslist[i] < 0)
						j_places_error(1);
				};
			places_add(places, _reslist, n, -1);
		};
}


/* p-list := p-interval | p-list ',' p-interval
 */
static
void places_p_list(int ***places)
{
	for ( ; ; pstok++ /* Skip comma */)
	{
		places_p_interval(places);
		if (*pstok != ',')
			break;
	}
	if (*pstok)
		j_places_error(1);
}


static
void places_aname_default(int ***places, absplace_t aptype, int plen)
{
	int n, i;

	n = ort->icvs.ncpus;
	if (aptype == SOCKETS)        /* assume 1 socket all in all */
	{
		for (i = 0; i < n; i++)
			_reslist[i] = i;
		places_add(places, _reslist, n, plen); /* 1 place for all */
	}
	else
	{
		if (plen > 0 && plen < n)
			n = plen;
		for (i = 0; i < n; i++)                      /* n places */
			places_add(places, &i, 1, plen);
	}
}


/* aname := word '(' len ')' | word
 * word  := "sockets" | "cores" | "threads" | "numa_domains"
 */
static
void places_aname(int ***places)
{
	int        i, n, len = -1;
	absplace_t aptype;

	if (strncasecmp(pstok, "threads", 7) == 0)
	{
		aptype = THREADS;
		pstok += 7;
	}
	else if (strncasecmp(pstok, "cores", 5) == 0)
	{
		aptype = CORES;
		pstok += 5;
	}
	else if (strncasecmp(pstok, "sockets", 7) == 0)
	{
		aptype = SOCKETS;
		pstok += 7;
	}
	else if (strncasecmp(pstok, "numa_domains", 12) == 0)
	{
		aptype = NUMADOMAINS;
		pstok += 12;
	}
	else if (strncasecmp(pstok, "ll_caches", 9) == 0)
	{
		aptype = LLCACHES;
		pstok += 9;
	}
	else
	{
		j_places_error(1);
	}

	if (*pstok == '(')
	{
		pstok++;
		len = places_integer(1);
		if (*pstok != ')')
			j_places_error(1);
		pstok++;
	}

	if (*pstok)
		j_places_error(1);

#ifdef HAVE_HWLOC
	places_aname_from_topology(places, aptype, len);
#else
	places_aname_default(places, aptype, len);
#endif
}


/* list := p-list | aname
 */
static
void places_list(int ***places)
{
	if (*pstok != '{' && *pstok != '!')
		places_aname(places);
	else
		places_p_list(places);
}


/* strdup with dumb space stripping */
static
char *_strip_copy(char *s)
{
	int  newlen;
	char *new, *t;

	/* Count length without spaces */
	for (t = s, newlen = 0; *t; t++)
		if (!isspace(*t))
			newlen++;
	/* Allocate and copy non-space characters */
	for (t = new = ort_alloc(newlen+1); *s; s++)
		if (!isspace(*s))
			*(t++) = *s;
	*t = 0;
	return (new);
}


static
void places_error_handle(int n, int **places) {
	switch (n)
	{
		case 1:
			ort_warning("incorrect value of OMP_PLACES e.v.; ignoring.\n");
			break;
		case 2:
			ort_warning("ort_getenv_places(): out of memory while adding a place.\n");
			break;
		case 3:
			ort_warning("cannot support a place of length %d or more.\n",
					MAX_PLACE_LEN);
			break;
		case 4:
			ort_warning("syntax error in OMP_PLACE e.v (expected a number); "
					"ignoring.\n");
			break;
		case 5:
			ort_warning("hwloc topology detection failed; ignoring.\n");
			break;
		case 6:
			ort_warning("ort_get_default_places(): out of memory while adding a place.\n");
			break;
	}

	if (places)  /* Cleanup */
		places_free(places);
}


void ort_getenv_places(char *s)
{
	int  n, i;
	int  **places = NULL;
	char *tmps = NULL;

	if ((n = setjmp(j_placeserr)) != 0)
	{
		places_error_handle(n, places);
		if (tmps)
			free(tmps);
		return;
	}

	pstok = tmps = _strip_copy(s);
	places_list(&places);
	free(tmps);

	ort->place_partition = places;
}


/* Default placement is to use one "processor" per OpenMP thread.
 * If hwloc is available, "processor" = core. Otherwise a "processor"
 * is whatever sysconf() returns.
 */
void ort_get_default_places(void)
{
	int **places = NULL;
	int n;

	if (ort->place_partition != NULL) /* Places already initialized from env */
		return;

	/* If execution reaches this point, OMP_PLACES e.v. was not set
	 * or ort_getenv_places() failed at some point.
	 */

	if ((n = setjmp(j_placeserr)) != 0)
	{
		places_error_handle((n == 5) ? 5 : 6, places);
		return;
	}

#ifdef HAVE_HWLOC
	places_aname_from_topology(&places, CORES, -1);
#else
	places_aname_default(&places, CORES, -1);
#endif

	ort->place_partition = places;
}


void places_show()
{
	int i, j;
	int **places = ort->place_partition;

	printf("\t[host] OMP_PLACES='");
	if (numplaces(places) <= 0)
		printf("' (no places defined)\n");
	else
	{
		for (i = 0; i < numplaces(places); i++)
		{
			printf("{");
			for (j = 0; j < placelen(places, i); j++)
				printf("%d%c", places[i + 1][j + 1],
				       (j == placelen(places, i) - 1) ? '}' : ',');
			printf("%s", (i == numplaces(places) - 1) ? "'" : ",");
		}
		printf(" (%d places defined)\n", numplaces(places));
	}
}


// Implemented for displaying affinity using affinity format.
char *places_get_list_str(int pfrom, int pto)
{
	int i, j, ub, k, plen,
	    bi = 0, buffsize, freespace; // For output buffer
	int **places = ort->place_partition;
	char *outbuff, *newbuff;

	buffsize = freespace = 128;

	if (numplaces(places) <= 0 || pfrom < 0 || pto >= numplaces(places))
		return NULL;

	outbuff = (char *) ort_alloc(buffsize * sizeof(char));

	for (i = pfrom; i <= pto; i++)
	{
		plen = placelen(places, i);
		for (j = 0; j < plen; j++)
		{
			for (ub = -1, k = j; k+1 < plen; k++)
				if (places[i+1][k+1] + 1 == places[i+1][k+2])
					ub = k+1;
				else
					break;
			if (ub == -1 || ub == j + 1)
				bi += snprintf(outbuff + bi, freespace, "%d,",
						places[i+1][j+1]);
			else
			{
				bi += snprintf(outbuff + bi, freespace, "%d-%d,",
						places[i+1][j+1], places[i+1][ub+1]);
				j = ub;
			}
			freespace = buffsize - bi;
			if (freespace < 16)
			{
				freespace += buffsize;
				buffsize <<= 1;
				outbuff = ort_realloc(outbuff, buffsize);
			}
		}
	}
	outbuff[bi-1] = '\0'; // Delete dangling ','
	return ort_realloc(outbuff, bi); // Shrink allocated space
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * BINDING THREADS TO PLACES                                         *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Compute the place (sub)partition that a thread should be bound to with
 * regard to the OpenMP processor binding policy and then place a request
 * to the ee_lib to bind that thread to the computed place (sub)partition.
 */
void bindme(int eeid, ort_eecb_t *t, ort_eecb_t *parent)
{
	int placenum, p, s, plb, prevp, pi;

	/* Master thread executes on parent's place.
	 * There is no need for binding; only currplace should be updated.
	 */
	if (eeid == 0)
		t->currplace = parent->currplace;

	/* In case binding parent thread has previously failed,
	 * disable binding for it's child threads.
	 */
	if (parent->currplace < 0)
		return;

	prevp = t->pfrom;
	placenum = t->pto - prevp + 1;
	/* proc_bind clause overrides current policy */
	if (parent->mf->bind_override != omp_proc_bind_false)
		__CURRTASK(t)->icvs.proc_bind = parent->mf->bind_override;

	switch (__CURRTASK(t)->icvs.proc_bind)
	{
		case omp_proc_bind_false:
			break;
		case omp_proc_bind_master:
			t->currplace = ee_bindme(ort->place_partition, parent->currplace);
			break;
		case omp_proc_bind_true:
		case omp_proc_bind_close:
			if (t->num_siblings <= placenum) /* making the common (?) case fast */
			{
				pi = prevp + (parent->currplace + eeid) % placenum;
				t->currplace = ee_bindme(ort->place_partition, pi);
				break;
			}
			p = t->num_siblings / placenum;
			s = t->num_siblings % placenum;
			if (eeid < s * (p + 1))
				pi = eeid / (p + 1);
			else
				pi = (eeid - s) / p;
			pi = prevp + (parent->currplace - prevp + pi) % placenum;
			t->currplace = ee_bindme(ort->place_partition, pi);
			break;
		case omp_proc_bind_spread:
			if (t->num_siblings > placenum)
			{
				p = t->num_siblings / placenum;
				s = t->num_siblings % placenum;
				if (eeid < s * (p + 1))
					pi = eeid / (p + 1);
				else
					pi = (eeid - s) / p;
				pi = prevp + (parent->currplace - prevp + pi) % placenum;
				t->currplace = ee_bindme(ort->place_partition, pi);
				// Only these 2 lines differ from the 'close' policy
				t->pfrom = pi;
				t->pto   = pi;
			}
			else
			{
				p = placenum / t->num_siblings;
				s = placenum % t->num_siblings;

				if (parent->currplace - prevp < s * (p + 1))
					plb = parent->currplace - (parent->currplace - prevp) % (p + 1);
				else
					plb = parent->currplace - (parent->currplace - prevp - s * (p + 1)) % p;

				if (eeid == 0)
				{
					t->pfrom = plb;
					if (parent->currplace - prevp >= s * (p + 1))
						p--;
					t->pto = plb + p;
					break; /* exit case */
				}

				if (plb < prevp + s * (p + 1))
					pi = ((plb - prevp) / (p + 1));
				else
					pi = s + (plb - prevp - s * (p + 1)) / p;

				pi = (pi + eeid) % t->num_siblings;

				if (pi < s)
				{
					t->pfrom = prevp + (prevp + pi * (p + 1)) % placenum;
				}
				else
				{
					t->pfrom = prevp + (prevp + s * (p + 1) + (pi - s) * p) % placenum;
					p--;
				}
				t->pto = t->pfrom + p;
				t->currplace = ee_bindme(ort->place_partition, t->pfrom);
			}
			break;
	}
}
