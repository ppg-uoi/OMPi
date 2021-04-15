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

/* taskdeps.c -- implementation of task dependencies using a list scheme */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include "ort.h"
#include "ort_prive.h"
#include "set.h"


/**
 * td_metadata_t is a type that is used to store dependency-tracking 
 * information at runtime for some tag. 
 * The metadata needed (for the List Scheme algorithm) is:
 *	- A lock to protect access to the metadata structure (see othr_lock_t)
 *	- A list of nodes (used for the tasks waiting) (see tdep_node_t)
 *	- Number of tasks in the oldest generation (int)
 *	- Annotation of the tasks in the youngest generation (see tdep_annotation_t)
 *	- The number of generations (int)
 */

typedef enum
{
	annotate_none_first = -1, // used as an "empty" value
	annotate_out,
	annotate_in,
	annotate_inout
} annotation_t;

typedef struct tdepnode_s
{
	void *task; // pointer to the task
	int last_in_generation; // end-of-generation marker
	struct tdepnode_s *next; // pointer to next node
} tdepnode_t;

/* Each metadata structure must lie in its own cache-line to avoid 
 * false-sharing between different tags. This requires that a metadata be
 * allocated on cache-line boundaries (i.e aligned_alloc()) and that it 
 * occupies a multiple of cache lines (and thus padded allocations are needed). 
 */
typedef struct
{
	othr_lock_t  mutex;
	tdepnode_t  *head;
	tdepnode_t  *tail;
	int          oldest_num_tasks;
	annotation_t youngest_annot;
	int          num_gens;
	void        *actual_addr;       /* real start of aligned allocation */
} metadata_t;

SET_TYPE_DEFINE(metadata_set,void*, metadata_t*, 1031)
SET_TYPE_IMPLEMENT_NR(metadata_set)


// All a task needs for task dependencies
typedef struct
{
	// Both depend_count_mutex must lie on their own cache-line (in the same 
	// for better results) because i do not want false-sharing for the mutex 
	// and the depend-count between different "task-dependency-graphs", i.e 
	// different tasks performing their own issue and release operations
	othr_lock_t       depcount_mutex;
	int               depcount;           // do i have dependencies?
	char              pad2[CACHE_LINE];
	metadata_t      **depsdata;       // list of tags this task has registered
	int               depsdata_count; // size of depsdata array
	set(metadata_set) depend_table;    // used to find the metadata for each tag
	void             *ready_list_next; // used during the release operation to 
	                                   // gather all tasks that become "ready"
} tdeps_t;


/* The dep_array has the following: first are the ouput annotations, then the 
 * input and lastly the inout ones. The index i (starting with 0) is used to 
 * traverse the dep_array. This macro returns the annotation type of 
 * dep_array[i].
 * NOTE: all the output annotations are considered as inout annotations. Thus, 
 * in the code below i expect only inout annotations. For this reason, this 
 * macro outputs an inout annotation for the first part where the output 
 * annotations are 
 */
#define GET_ANNOTATION_TYPE(i,num_output,num_input,num_inout) \
  (i < num_output ? annotate_inout \
                  : (i < num_output + num_input ? annotate_in \
                                                : annotate_inout))

#define INIT_TASK_DEPEND_METADATA(tmd) do { \
						othr_init_lock(&(tmd)->mutex, ORT_LOCK_NORMAL); \
						othr_set_lock(&(tmd)->mutex); \
						(tmd)->head = NULL; \
						(tmd)->tail = NULL; \
						(tmd)->oldest_num_tasks = 0; \
						(tmd)->youngest_annot = annotate_none_first; \
						(tmd)->num_gens = 0; \
						(tmd)->actual_addr = (tmd)->actual_addr; /* actual_addr is set in the same thread before using this macro */ \
						othr_unset_lock(&(tmd)->mutex); \
					} while (0)

#define INIT_TASK_DEPEND_NODE(node,task) do { \
						(node)->task = (task); \
						(node)->last_in_generation = 0; \
						(node)->next = NULL; \
					} while (0)

#define TDEPINFO(t) ((tdeps_t *) (t)->dependencies)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *  MEMORY ROUTINES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


tdepnode_t *tdep_node_alloc()
{
	return (tdepnode_t*) ort_alloc(sizeof(tdepnode_t));
}


void tdep_node_free(tdepnode_t *par)
{
	if (par)
		free(par);
}


metadata_t *padded_metadata_alloc()
{
  /* A metadata structure needs a) to be aligned to cache-line boundaries 
	 * and b) have a size that is a multiple of the cache line size
	 */
	static int paddedsize = (sizeof(metadata_t)/CACHE_LINE + 
	                         (sizeof(metadata_t)%CACHE_LINE ? 1 : 0))*CACHE_LINE;
	void *actual_addr = NULL;
	metadata_t *md;
	
	/* Ask for the nearest multiple of cache line size */
	md = ort_alloc_aligned(paddedsize, &actual_addr);
	md->actual_addr = actual_addr;
	return md;
}


void padded_metadata_free(metadata_t *par)
{
	/* Free the actual memory (not the memory-aligned subset).
	 * Note: The list of nodes does not have to be deallocated because 
	 * these nodes are reclaimed by the release operations 
	 */
	if (par)
		free(par->actual_addr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *  MAIN ROUTINES                                                *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


typedef void * dtag_t;


static int rmduplicates(dtag_t *from, dtag_t *to)
{
	int    n = 0;
	dtag_t *e;
	
	for (; from < to; from++)
		if (*from == NULL)
			continue;
		else
			for (e = from+1; e < to; e++)
				if (*e == *from)
				{
					n++;
					*e = NULL;
				};
	return n;
}


static int rminother(dtag_t *from, dtag_t *to, dtag_t *othfrom, dtag_t *othto)
{
	int    n = 0;
	dtag_t *e;
	
	for (; from < to; from++)
		if (*from == NULL)
			continue;
		else
			for (e = othfrom; e < othto; e++)
				if (*e == *from)
				{
					n++;
					*from = NULL;
					break;
				};
	return n;
}


static int rminboth(dtag_t *from, dtag_t *to, dtag_t *othfrom, dtag_t *othto)
{
	int    n = 0;
	dtag_t *e;
	
	for (; from < to; from++)
		if (*from == NULL)
			continue;
		else
			for (e = othfrom; e < othto; e++)
				if (*e == *from)
				{
					if (e < othto-1)         /* swap with last one */
					{
						dtag_t tmp = *(othto-1);
						*(othto-1) = *e;
						*e = tmp;
					}
					n++;
					*from = NULL;
					othto--;
					e--;
					break;
				};
	return n;
}


/* Mark duplicate and redundant entries (by making them NULL) and return 
 * actual # dependencies.
 */
static 
int fix_dep_array(dtag_t *arr, int *nout, int *nin, int *ninout)
{
	dtag_t *cur = arr;
	int    mi = 0, mo = 0, mio = 0, nb, nactual;

	/* Step #1: remove duplicates in each dependence category */
	if (*nout)
		mo += rmduplicates(cur, cur + *nout);
	cur += *nout;
	if (*nin)
		mi += rmduplicates(cur, cur + *nin);
	cur += *nin;
	if (*ninout)
		mio += rmduplicates(cur, cur + *ninout);
	
	/* Step #2: remove anything from input or output that appears in inout */
	cur = arr + *nout + *nin;
	if (*nout && *ninout)
		mo += rminother(arr, arr + *nout, cur, cur + *ninout);
	if (*nin && *ninout)
		mi += rminother(arr + *nout, arr + *nout + *nin, cur, cur + *ninout);
	
	/* Step #3: remove dependencies that are in both input and output and
	 * add them to the inout set. In the output set they are marked as NULL.
	 * In the input set, they are shifted to its upper end, and the start 
	 * of the inout set is lowered so as to include them,
	 */
	cur = arr + *nout;
	nb = rminboth(arr, arr + *nout, cur, cur + *nin);
	mi += nb;
	mo += nb;

	nactual = *nout + *nin + *ninout - mo - mi - mio + nb;

	*nout -= mo; 
	*nin -= mi;
	*ninout -= mio;
	*ninout += nb;
	
	return (nactual);
}


/**
 * This function is called to issue (i.e register) a task for a single tag. 
 * The tag is specified through the metadata argument. 
 * The annotation and the task itself are passed as well (see last two 
 * arguments). 
 * The function returns 1 if a dependency is found and 0 otherwise. 
 * If a memory allocation failure occurs (for a task node) -1 is returned 
 * and the metadata is left unchanged.
 *
 * Note 1: All parameters must be non-null pointers. 
 * Note 2: This is called for non-output tasks.
 * Note 3: When this function is called the lock on the task is held.
 */
static 
int issue_task_single_tag(metadata_t *metadata, annotation_t annot, 
                          ort_task_node_t *task)
{
	int status;
	tdepnode_t *node;

	assert(metadata != NULL && annot != annotate_out && task != NULL);

	othr_set_lock(&metadata->mutex);
	
	/* First case: No generations active */
	if (metadata->num_gens == 0)
	{
		// The task creates a new generation which serves as both the youngest 
		// and oldest generation.
		// Thus, the task has no dependencies and is not inserted in the list 
		// and parameter node is left unused
		metadata->num_gens = 1;
		metadata->oldest_num_tasks = 1;
		metadata->youngest_annot = annot;
		
		status = 0;
	}
	else 
		if ((annot == annotate_inout || metadata->youngest_annot != annot) 
				         && (metadata->youngest_annot != annotate_none_first))
		{
			/* Second case: a generation must be created; get a new task node */
			if ((node = tdep_node_alloc()) == NULL)
				status = -1;
			else
			{
				metadata->youngest_annot = annot;  // remember annot since it changed
				++metadata->num_gens; 
				// A new generation is created; the number of generations increases. 
				// Since there exists a previous generation, this task has a 
				// dependency and the depcount of the task must be increased;
				// the increase is safe since the lock on the task is assumed held.
				++TDEPINFO(task)->depcount;
			
				INIT_TASK_DEPEND_NODE(node,task);
				
				if (metadata->tail != NULL)
				{
					// The current last node in the list is now also the last node 
					// in its generation so we must set the end-of-generation marker
					metadata->tail->last_in_generation = 1;
					metadata->tail->next = node;
				}
				else
					metadata->head = node;
				metadata->tail = node;

				status = 1;
			}
		}
		else
		{
			// The task is inserted in the current generation. If num_gens == 1 
			// however the task is inserted in the oldest generation and thus it is
			// not explicitly inserted in the list because it has no dependencies.
			if (metadata->num_gens == 1)
			{
				++metadata->oldest_num_tasks;
				status = 0;
			}
			else
			{
				if ((node = tdep_node_alloc()) == NULL)
					status = -1;
				else
				{
					// the task has a dependency
					++TDEPINFO(task)->depcount;
				
					INIT_TASK_DEPEND_NODE(node,task);

					metadata->tail->next = node;
					metadata->tail = node;

					status = 1;
				}
			}
		}

	othr_unset_lock(&metadata->mutex);

	return status;
}


static 
void issue_task_nodeps_case(ort_eecb_t *me, ort_task_node_t *tnode)
{
	/* I must offload the task immediately (to my queue)
	 * A Note: in the cases of memory failure i couldn't do this because the task 
	 * wasn't registered and thus could not do a release operation later. If i 
	 * offloaded to my task-queue and went on to create a new task
	 * with same dependencies i would have a problem. But, if i get here then i 
	 * have successfully made the issue operations with the exception that the 
	 * task had no dependencies after all (at runtime). However, the task has 
	 * been registered and will perform the release operations as it must.
	 */

	if (ort_task_throttling())   /* No space in my own queue */
		ort_task_execute_this(me, tnode);
	else
	{
		int worker_id = me->thread_num;
		ort_eecb_t *my_parent = me->sdn;
		int old_bottom = atomic_read
		                 (&(my_parent->tasking.queue_table[worker_id].bottom));

		my_parent->tasking.queue_table[worker_id].tasks[old_bottom % TASKQUEUESIZE] 
		  = tnode;
		my_parent->tasking.queue_table[worker_id].bottom++;
	}
}


int tdeps_init(void *_tnode, int numdeps)
{
	ort_task_node_t *tnode = (ort_task_node_t *) _tnode;
	tdeps_t         *deps;
	
	assert(tnode != NULL && numdeps >= 0);

	tnode->dependencies = deps = ort_alloc(sizeof(tdeps_t));
	
	othr_init_lock(&deps->depcount_mutex, ORT_LOCK_NORMAL);
	// FIXME: Do we need the lock here? (VVD)
	othr_set_lock(&deps->depcount_mutex);
		deps->depcount = 0;
	othr_unset_lock(&deps->depcount_mutex);
	
	/* this task may not create any task with dependencies. */
	deps->depend_table = NULL;
	deps->ready_list_next = NULL;
	deps->depsdata_count = numdeps;
	if (numdeps == 0)
		deps->depsdata = NULL;
	else
		if ((deps->depsdata = ort_alloc(numdeps*sizeof(metadata_t*))) == NULL)
			return 1;
	return 0;
}


void tdeps_issue_task(ort_task_node_t *tnode, 
                      dtag_t *deparray, int num_out, int num_in, int num_inout)
{
	ort_eecb_t *me = __MYCB;
	ort_task_node_t *parent = NULL;
	int deparraysize = num_out + num_in + num_inout, uniquedeps, truedeps = 0;
	int i, depid, memory_failure = 0, issue_status;
	dtag_t tag;
	tdeps_t *depinfo;
	metadata_t *metadata;
	setelem(metadata_set) datum_location = NULL;

	assert(deparray != NULL && (num_out > 0 || num_in > 0 || num_inout>0));
	assert(tnode->parent != NULL);

	uniquedeps = fix_dep_array(deparray, &num_out, &num_in, &num_inout);
	tdeps_init(tnode, uniquedeps);
	depinfo = TDEPINFO(tnode);
	
	/* Issue this new node for each tag (in dep_array) to parent task */
	parent = tnode->parent;

	if (parent->dependencies == NULL)
		tdeps_init(parent, 0);
	// Lazy allocation of depend_table in parent
	if (TDEPINFO(parent)->depend_table == NULL)
		TDEPINFO(parent)->depend_table = set_new(metadata_set);
	
	/* First i lock the task. I do this in order to avoid release operations
	 * while i make issue operations for multiple tags. I also increase the 
	 * number of children for my parent
	 * Note: if i did not lock the node i could initialize the depcount to 
	 *       1 and decrement it by one at the end
	 * NOTE: CHANGE:: this version uses initialization by 1
	 */
	othr_set_lock(&depinfo->depcount_mutex);

#if defined(HAVE_ATOMIC_FAA)
	_faa(&((tnode->parent)->num_children), 1);
#else
	ee_set_lock(&((tnode->parent)->lock));
	(tnode->parent)->num_children++;
	ee_unset_lock(&((tnode->parent)->lock));
#endif

	for (i = depid = 0; i < deparraysize; ++i)    /* Go through each tag */
		if (deparray[i] == NULL)                   /* Skip removed ones */ 
			continue;
		else
		{
			tag = deparray[i];

			/* Find the metadata for this tag */
			datum_location = set_get(TDEPINFO(parent)->depend_table, tag);
			if (datum_location == NULL)
			{
				// first time i encounter this tag so i must insert it
				datum_location = set_put(TDEPINFO(parent)->depend_table, tag);
				datum_location->value = NULL;
			}

			if (datum_location->value == NULL)
			{
				// NOTE: In the case inserted == false, which means that the key was 
				// found, i may still not have allocated the metadata (in the value 
				// member of *datum_location) because of this scenario:
				// Search for key. Entry is not found and created. Here i try to 
				// allocate metadata but if fail. The next time i search for the same 
				// key. Now the function sets inserted to false, but i didn't allocate
				// the metadata the previous time.
				datum_location->value = padded_metadata_alloc();
				
				// initialize the metadata structure
				INIT_TASK_DEPEND_METADATA(datum_location->value);
			}

			metadata = datum_location->value;
			depinfo->depsdata[depid] = metadata;

			issue_status = issue_task_single_tag(metadata,
			                   GET_ANNOTATION_TYPE(depid,num_out,num_in,num_inout), 
			                   tnode);

			if (issue_status == -1)
			{
				// In this simple implementation just crash
				ort_error(1, "[ort_issue_task_with_depend]: memory allocation failed\n");
				// Handle memory failure 5
				depinfo->depsdata_count = depid;
				memory_failure = 1;
				break;
			}

			truedeps += issue_status;
			depid++;
		};
	
	othr_unset_lock(&depinfo->depcount_mutex);
	// Change in this version i decrement by one the depcount of the task
	//othr_set_lock(&tnode->task_depend_info_var.depcount_mutex);
	//depcount_last = --tnode->task_depend_info_var.depcount;	
	//othr_unset_lock(&tnode->task_depend_info_var.depcount_mutex);

	// Check if i had no dependencies
	if (truedeps == 0)
		issue_task_nodeps_case(me, tnode);

	testnotset(me->sdn->tasking.never_task);
}


/**
 * This function is called to release (i.e deregister) a task for a single tag. 
 * The tag is specified through the metadata argument.
 * 
 * Usage of parameters reclaim_begin and reclaim_end:
 * If after the release some portion in the beginning of the list becomes 
 * now free to reclaim, *reclaim_begin points to the first node and 
 * *reclaim_end points to one-past-the-last node (i.e has the value of the 
 * next pointer of the last node). Otherwise, both *reclaim_begin and 
 * *reclaim_end are set to NULL.
 *
 * Usage of parameter ready_list:
 * If after the release some tasks become ready, *ready_list points to the 
 * beginning of a list of tasks that can be traversed using the ready_list_next 
 * pointers inside each task (in the depend var). Otherwise, *ready_list is 
 * set to NULL.
 *
 * Note: All parameters must be non-null pointers.
 */
static void release_task_single_tag(metadata_t *metadata, 
        tdepnode_t **reclaim_begin, tdepnode_t **reclaim_end, void **ready_list)
{
	assert(metadata != NULL && reclaim_begin != NULL && reclaim_end != NULL 
	        && ready_list != NULL);

	*reclaim_begin = NULL;
	*reclaim_end = NULL;
	*ready_list = NULL;

	othr_set_lock(&metadata->mutex);

	if (--metadata->oldest_num_tasks == 0)
	{
		if (metadata->head != NULL)
		{
			tdepnode_t *t;
			ort_task_node_t *task;

			*reclaim_begin = metadata->head;

			do
			{
				t = metadata->head;
				task = (ort_task_node_t*)(t->task);

				// reduce depend count of task in t
				othr_set_lock(&TDEPINFO(task)->depcount_mutex);
					
				if (--TDEPINFO(task)->depcount == 0)
				{
					// this task must now be scheduled; inserted it in the ready_list
					TDEPINFO(task)->ready_list_next = *ready_list;
					*ready_list = t->task;
				}

				othr_unset_lock(&TDEPINFO(task)->depcount_mutex);

				++metadata->oldest_num_tasks;
				metadata->head = t->next;
			} while (t->last_in_generation == 0 && metadata->head != NULL);

			*reclaim_end = metadata->head;
		}
			
		--metadata->num_gens;

		if (metadata->head == NULL)
		{
			metadata->tail = NULL;
			if (metadata->num_gens == 0)
				metadata->youngest_annot = annotate_none_first; /* empty */
		}
	}

	othr_unset_lock(&metadata->mutex);
}


void tdeps_after_execution(ort_task_node_t *tnode, void *me)
{
	ort_task_node_t *ready_node;
	tdeps_t         *dinfo;
	int             i, numdeps;
	tdepnode_t      *reclaim_from, *reclaim_to, *rn;
	void            *ready_list;
	metadata_t      *metadata;
	
	assert(tnode != NULL && me != NULL);
	if ((dinfo = TDEPINFO(tnode)) == NULL)
		return;
	
	numdeps = dinfo->depsdata_count;

	// Release the task from all the registered tags (in depsdata)
	for (i = 0; i < numdeps; ++i)
	{
		metadata = dinfo->depsdata[i];

		// Release task from given metadata
		release_task_single_tag(metadata, &reclaim_from, &reclaim_to, &ready_list);

		// Free any task nodes
		for (; reclaim_from != reclaim_to; reclaim_from = rn)
		{
			rn = reclaim_from->next;
			tdep_node_free(reclaim_from);
		}
		
		// Schedule tasks in the ready-list; we do it one-by-one here.
		// Another option would be to insert all the tasks at once and thus 
		// have only 1 memory barrier at the end.
		// Locality Heuristic: I place the tasks in my queue. This helps locality 
		// because if i was a writer task and i activated some reader tasks
		// i have the data that they will read.
		while (ready_list)
		{
			ready_node = (ort_task_node_t*) ready_list;
			
			// i must do this before executing the task, because if the task is 
			// executed then it may get freed and i will not access to *ready_node
			ready_list = TDEPINFO(ready_node)->ready_list_next;

			// Another subtle note here: Because the task was issued with the 
			// lock held, i know that when the release operation above made the 
			// depend_cout of ready_node zero this can only happen when the issue 
			// operation is done with the task. So i am the only one which now has 
			// access to this task and i can proceed on to scheduling it
			issue_task_nodeps_case(me, ready_node);
		}
	}
}


void tdeps_free_tdepinfo(void *_dinfo)
{
	tdeps_t *dinfo = (tdeps_t *) _dinfo;
	setelem(metadata_set) elem;
	metadata_t *m;
	
	if (!dinfo) return;

	// NOTE: in order to access the fields of dinfo it means that i have memory 
	// visibility of *dinfo. This is true in OMPi's current scheduler
	// because tasks are tied and hence one thread executes the task from start 
	// to finish. BUT if that wasn't the case, in any scheduler the assumption
	// that when a thread steals another task it also obtains memory visibility 
	// using some kind of memory fence is sound, hence i do not need some kind of
	// fence here.
 
	othr_destroy_lock(&dinfo->depcount_mutex);
	if (dinfo->depsdata)
		free(dinfo->depsdata);
	if (dinfo->depend_table)
	{
		// Release memory for each of the metadata structures
		for (elem = dinfo->depend_table->first; elem; elem = elem->next)
		{
			m = elem->value;
			
			// to acquire memory visibility of the task_depend_metadata i do an
			// acquire/release on the internal mutex
			othr_set_lock(&(m->mutex));
			othr_unset_lock(&(m->mutex));
			othr_destroy_lock(&(m->mutex));
			         padded_metadata_free(m);
		}
		set_free(dinfo->depend_table); // release memory for the metadata set itself
		dinfo->depend_table = NULL;
	}
}
