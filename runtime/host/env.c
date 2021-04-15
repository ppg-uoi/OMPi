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

/* env.c -- OpenMP environmental variables */

#include "ort_prive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#ifdef HAVE_HWLOC
	#include <hwloc.h>
#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * GLOBAL VARIABLES / DEFINITIONS / MACROS                           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* display env values */
#define DISPLAY_ENV_FALSE   0
#define DISPLAY_ENV_TRUE    1
#define DISPLAY_ENV_VERBOSE 2
int display_env = DISPLAY_ENV_FALSE;

#define ISTRUE(s) \
	(strncasecmp(s, "true", 4) == 0 || strncmp(s, "1", 1) == 0)
#define ISFALSE(s) \
	(strncasecmp(s, "false", 5) == 0 || strncmp(s, "0", 1) == 0)


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
 * OMP_STACKSIZE                                                     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* size | sizeB | sizeK | sizeM | sizeG */
static
long int _size_bytes(char *str)
{
	long int size;
	char *s, mod;

	for (s = str; isspace(*s) || isdigit(*s); s++)
		;
	switch (*s)  /* specifier */
	{
		case   0:
		case 'k': case 'K': mod = 10; break;  /* default */
		case 'b': case 'B': mod = 0;  break;
		case 'm': case 'M': mod = 20; break;
		case 'g': case 'G': mod = 30; break;
		default:
			goto WRONG_STSIZE;
	};
	if (*s)      /* past the specifier */
	{
		for (*s = 0, s++; isspace(*s); s++) /* */ ;
		if (*s) goto WRONG_STSIZE;
	}

	if ((size = atoi(str)) < 0) goto WRONG_STSIZE;
	if (mod) size <<= mod;
	if (size < 128)
	{
		ort_warning("illegal OMP_STACKSIZE value (%ld); using 256K instead\n", size);
		size = KBytes(256);
	}
	return (size);

WRONG_STSIZE:
	ort_warning("incorrect OMP_STACKSIZE specification; using system default.\n");
	return (-1);
}


static
void _show_size_bytes(FILE *fp, size_t n)
{
	if (n < 1024)
		fprintf(fp, "%ldB", n);
	else 
		if (n < 1024*1024)
		{
			if (n & 1023)
				fprintf(fp, "%ldB", n);
			else 
				fprintf(fp, "%ldK", n >> 10);
		}
		else
			if (n < 1024*1024*1024)
			{
				if (n & (1024*1024 - 1))
					fprintf(fp, "%ldB", n);
				else 
					fprintf(fp, "%ldM", n >> 20);
			}
			else
			{
				if (n & (1024*1024*1024 - 1))
					fprintf(fp, "%ldB", n);
				else 
					fprintf(fp, "%ldG", n >> 30);
			}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * RETRIEVE AND DISPLAY ENVIRONMENTAL VARIABLES                      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Environmental variables */
void ort_get_environment(void)
{
	char *s, *t;
	int  n;

	if ((s = getenv("OMP_DYNAMIC")) != NULL)
	{
		if (ISTRUE(s))
			ort->icvs.dynamic = 1;
		else
			ort->icvs.dynamic = 0;
	}

	if ((s = getenv("OMP_NESTED")) != NULL)
	{
		if (ISTRUE(s))
			ort->icvs.nested = 1;
		else
			ort->icvs.nested = 0;
	}

	if ((s = getenv("OMP_SCHEDULE")) != NULL)
	{
		if ((t = strchr(s, ',')) != NULL)   /* Chunksize specified. */
		{
			sscanf(t + 1, "%d", &n);
			if (n > 0)
				ort->icvs.rtchunk = (u_long) n;
			else
				ort_warning("illegal chunksize in environment "
				            "variable OMP_SCHEDULE; using default value\n");
		}
		if (strncasecmp(s, "static", 6) == 0)
			ort->icvs.rtschedule = omp_sched_static;
		else
			if (strncasecmp(s, "dynamic", 7) == 0)
				ort->icvs.rtschedule = omp_sched_dynamic;
			else
				if (strncasecmp(s, "guided", 6) == 0)
					ort->icvs.rtschedule = omp_sched_guided;
				else
					if (strncasecmp(s, "auto", 4) == 0)
						ort->icvs.rtschedule = omp_sched_auto;
					else
					{
						ort_warning("incorrect schedule type of environment "
						            "variable OMP_SCHEDULE; 'auto' assumed\n");
						ort->icvs.rtschedule = omp_sched_auto;
					}
	}

	/* OpenMP 3.0 */
	if ((s = getenv("OMP_STACKSIZE")) != NULL)
		if ((n = _size_bytes(s)) > 0)
			ort->icvs.stacksize = n;
	/* OpenMP 3.0 */
	if ((s = getenv("OMP_THREAD_LIMIT")) != NULL)
		if (sscanf(s, "%d", &n) == 1 && n > 0)
			ort->icvs.threadlimit = n;
	/* OpenMP 3.0 */
	if ((s = getenv("OMP_MAX_ACTIVE_LEVELS")) != NULL)
		if (sscanf(s, "%d", &n) == 1 && n >= 0)
			ort->icvs.levellimit = n;
	/* OpenMP 3.0 */
	if ((s = getenv("OMP_WAIT_POLICY")) != NULL)
	{
		if (strncasecmp(s, "active", 6) == 0)
			ort->icvs.waitpolicy = _OMP_ACTIVE;
		else
			if (strncasecmp(s, "passive", 7) == 0)
				ort->icvs.waitpolicy = _OMP_PASSIVE;
			else
				ort_warning("incorrect value of environment "
				            "variable OMP_WAIT_POLICY; ignoring\n");
	}

	/* Modified in OpenMP 3.1 */
	if ((s = getenv("OMP_NUM_THREADS")) != NULL)
	{
		int  l = 0;
		char *t = s - 1;

		do
		{
			if (sscanf(t + 1, "%d", &n) == 1 && n > 0)
			{
				if (l < MAX_NUMTHR_LEVELS)
					ort->nthr_per_level[l++] = n;
				else
				{
					ort_warning("too many levels in OMP_NUM_THREADS environment "
					            "variable;\n  using the first %d.\n", MAX_NUMTHR_LEVELS);
					break;
				}
			}
			else
			{
				ort_warning("illegal value in environment variable "
				            "OMP_NUM_THREADS; using default values\n");
				l = 0;
				break;
			}
		}
		while ((t = strchr(t + 1, ',')) != NULL);   /* One more level */

		ort->set_nthrlevs = l;
		if (l > 0)
			ort->icvs.nthreads = ort->nthr_per_level[0];  /* Force this initialy */
	}

	/* Modified in OpenMP 4.0 */
	if ((s = getenv("OMP_PROC_BIND")) != NULL)
	{
		int  l = 0;
		char *t = s - 1;
		char bind[256];
		omp_proc_bind_t bind_choice;

		do
		{
			if (sscanf(t + 1, "%s", bind) == 1)
			{
				if (ISFALSE(bind))
				{
					if (l != 0)
					{
						ort_warning("illegal value in environment variable "
						            "OMP_PROC_BIND; using default values\n");
						l = 0;
					}
					else
						ort->icvs.proc_bind = omp_proc_bind_false;

					break;
				}
				else
					if (ISTRUE(bind))
					{
						if (l != 0)
						{
							ort_warning("illegal value in environment variable "
							            "OMP_PROC_BIND; using default values\n");
							l = 0;
						}
						else
							ort->icvs.proc_bind = omp_proc_bind_true;

						break;
					}
					else
					{
						if (strncasecmp(bind, "primary", 7) == 0)
							bind_choice = omp_proc_bind_primary;
						else if (strncasecmp(bind, "master", 6) == 0)
							bind_choice = omp_proc_bind_master;
						else if (strncasecmp(bind, "close", 5)  == 0)
							bind_choice = omp_proc_bind_close;
						else if (strncasecmp(bind, "spread", 6) == 0)
							bind_choice = omp_proc_bind_spread;
						else
						{
							ort_warning("illegal value in environment variable "
							            "OMP_PROC_BIND; using default values\n");
							l = 0;
							break;
						}
					}
				if (l < MAX_BIND_LEVELS)
					ort->bind_per_level[l++] = bind_choice;
				else
				{
					ort_warning("too many levels in OMP_PROC_BIND environment "
					            "variable;\n  using the first %d.\n", MAX_BIND_LEVELS);
					break;
				}
			}
			else
			{
				ort_warning("illegal value in environment variable "
				            "OMP_PROC_BIND; using default values\n");
				l = 0;
				break;
			}
		}
		while ((t = strchr(t + 1, ',')) != NULL);   /* One more level */

		ort->set_bindlevs = l;
		if (l > 0)
			ort->icvs.proc_bind = ort->bind_per_level[0];  /* Force this initialy */
	}

	/* OpenMP 4.0 */
	if ((s = getenv("OMP_CANCELLATION")) != NULL)
	{
		if (ISTRUE(s))
			ort->icvs.cancel = 1;
	}

	/* OpenMP 4.0 */
	if ((s = getenv("OMP_DISPLAY_ENV")) != NULL)
	{
		if (ISTRUE(s))
			display_env = DISPLAY_ENV_TRUE;
		else
			if (strncasecmp(s, "VERBOSE", 7) == 0)
				display_env = DISPLAY_ENV_VERBOSE;
	}

	/* OpenMP 4.0 */
	if ((s = getenv("OMP_PLACES")) != NULL)
		ort_getenv_places(s);
	ort_get_default_places(); /* Fallback to default places if required */

	/* OpenMP 4.0 */
	if (((s = getenv("OMP_DEFAULT_DEVICE")) != NULL) && (sscanf(s, "%d", &n)==1))
	{
		if(n >= 0 && n < ort->num_devices)
			ort->icvs.def_device = n;
		else
			if (!ort->embedmode)
				ort->icvs.def_device = ort_illegal_device("OMP_DEFAULT_DEVICE", n);
	}

	/* OpenMP 4.5, section 4.14, p. 303 */
	if ((s = getenv("OMP_MAX_TASK_PRIORITY")) != NULL)
		if (sscanf(s, "%d", &n) == 1 && n >= 0)
			ort->icvs.max_task_prio = n;

	/* OpenMP 5.0 */
	if ((s = getenv("OMP_DISPLAY_AFFINITY")) != NULL)
	{
		if (ISTRUE(s))
			ort_warning("OMP_DISPLAY_AFFINITY: currently ignoring any non-FALSE value\n");
			// ort->icvs.display_affinity = 1;
	}

	/* OpenMP 5.0 */
	if ((s = getenv("OMP_AFFINITY_FORMAT")) != NULL)
		ort_set_affinity_format(s);

	/* OpenMP 5.0 */
	if ((s = getenv("OMP_TARGET_OFFLOAD")) != NULL)     /* Ignored for now */
	{
		if (strncasecmp(s, "mandatory", 9) == 0)
			ort->icvs.targetoffload = OFFLOAD_MANDATORY;
		else
			if (strncasecmp(s, "disabled", 8) == 0)
				ort->icvs.targetoffload = OFFLOAD_DISABLED;
			else
				if (strncasecmp(s, "default", 7) == 0)
					ort->icvs.targetoffload = OFFLOAD_DEFAULT;
				else
					ort_warning("incorrect value of environment "
					            "variable OMP_TARGET_OFFLOAD; ignoring\n");
	}

	/* 
	 * OMPi-specific below
	 */
	
	if (((s = getenv("OMPI_TASKQUEUESIZE")) != NULL) && (sscanf(s, "%d", &n) == 1)
	    && n > 0)
		ort->taskqueuesize = n;
	else
		ort->taskqueuesize = TASK_QUEUE_SIZE;

	if (((s = getenv("OMPI_DYNAMIC_TASKQUEUESIZE")) != NULL) && ISTRUE(s))
		ort->dynamic_taskqueuesize = 1;
	else
		ort->dynamic_taskqueuesize = 0;

	if ((s = getenv("OMPI_STEAL_POLICY")) != NULL)
	{
		if (strncasecmp(s, "FIFO", 4) == 0)
			ort->ompi_steal_policy = FIFO;
		else
			if (strncasecmp(s, "LIFO", 4) == 0)
				ort->ompi_steal_policy = LIFO;
	}
	else
		ort->ompi_steal_policy = FIFO;

	if ((s = getenv("OMPI_PAR2TASK_POLICY")) != NULL)
	{
		if (ISFALSE(s))
			ort->partotask_policy = FALSE;
		else if (ISTRUE(s))
			ort->partotask_policy = TRUE;
		else if (strncasecmp(s, "AUTO", 4) == 0)
			ort->partotask_policy = AUTO;
	}
	else
		ort->partotask_policy = AUTO;
	
	if (((s = getenv("OMPI_HOSTTARGET_SHARE")) != NULL) && ISFALSE(s))
		ort->module_host.sharedspace = 0;
}


static
void show_places(int beautify)
{
	int i, j, k, ub, plen;
	int **places = ort->place_partition;

	printf("\t[host] OMP_PLACES='");
	if (numplaces(places) <= 0)
		printf("' (no places defined)\n");
	else
	{
		for (i = 0; i < numplaces(places); i++)
		{
			printf("{");
			plen = placelen(places, i);
			for (j = 0; j < plen; j++)
			{
				for (ub = -1, k = j; beautify && k+1 < plen; k++)
				{
					if (places[i+1][k+1] + 1 == places[i+1][k+2])
						ub = k + 1;
					else
						break;
				}
				/*
				 * No need to check the value of beautify here.
				 * When beautify is false, ub equals -1 due to
				 * the for-loop initialization above.
				 */
				if (ub == -1 || ub == j + 1)
				{
					printf("%d%c", places[i + 1][j + 1],
							(j == plen - 1) ? '}' : ',');
				}
				else
				{
					printf("%d-%d%c", places[i + 1][j + 1],
							places[i + 1][ub + 1],
							(ub == plen - 1) ? '}' : ',');
					j = ub;
				}
			}
			printf("%s", (i == numplaces(places) - 1) ? "'" : ",");
		}
		printf(" (%d places defined)\n", numplaces(places));
	}
}


#define DISPVAR(v,val) printf("\t[host] " #v "='" #val "'\n")
#define DISPTFVAR(v,flag,t,f) \
          printf("\t[host] " #v "='%s'\n", (flag) ? #t : #f)

          
/* OpenMP 4.0 */
void display_env_vars()
{
	char *bind_choice[5] = {"FALSE", "TRUE", "PRIMARY", "CLOSE", "SPREAD"};
	int  i;

	printf("OPENMP DISPLAY ENVIRONMENT BEGIN\n");
	printf("\t_OPENMP='%d'\n", _OPENMP);

	/* 
	 * OpenMP 1.0 - OpenMP 2.5 
	 */
	
	DISPTFVAR(OMP_DYNAMIC, ort->icvs.dynamic, TRUE, FALSE);
	DISPTFVAR(OMP_NESTED, ort->icvs.nested, TRUE, FALSE);
	if (ort->icvs.rtchunk == 0)
		printf("\t[host] OMP_SCHEDULE='AUTO'\n");
	else
		printf("\t[host] OMP_SCHEDULE='%s, %d'\n", 
		       ort->icvs.rtschedule == omp_sched_static ? "STATIC" :
		       ort->icvs.rtschedule == omp_sched_dynamic ? "DYNAMIC" :
		       ort->icvs.rtschedule == omp_sched_guided ? "GUIDED" :
		       ort->icvs.rtschedule == omp_sched_auto ? "AUTO" : "STATIC",
		       ort->icvs.rtchunk);

	/* 
	 * OpenMP 3.0
	 */
	
	printf("\t[host] OMP_STACKSIZE='");
	if (ort->icvs.stacksize == -1)
	{
		pthread_attr_t tattr;
		size_t         size, ret = pthread_attr_init(&tattr);
		
		ret = pthread_attr_getstacksize(&tattr, &size);
		_show_size_bytes(stdout, size);
		printf("' (system default)\n");
	}
	else
	{
		_show_size_bytes(stdout, ort->icvs.stacksize);
		printf("'\n");
	}
	printf("\t[host] OMP_THREAD_LIMIT='%d'\n", ort->icvs.threadlimit);
	printf("\t[host] OMP_MAX_ACTIVE_LEVELS='%d'\n", ort->icvs.levellimit);
	DISPTFVAR(OMP_WAIT_POLICY,ort->icvs.waitpolicy==_OMP_PASSIVE,PASSIVE,ACTVE);


	/* 
	 * OpenMP 3.1
	 */
	
	printf("\t[host] OMP_NUM_THREADS='");
	if (ort->set_nthrlevs > 0)
		for (i = 0; i < ort->set_nthrlevs; i++)
			printf("%d%s", ort->nthr_per_level[i], 
			               (i != ort->set_nthrlevs-1) ? "," : "'\n");
	else
		printf("%d'\n", (ort->icvs.nthreads == -1) ? 
		                ort->icvs.ncpus : ort->icvs.nthreads);

	/* 
	 * OpenMP 4.0
	 */
	
	printf("\t[host] OMP_PROC_BIND='");
	if (ort->set_bindlevs > 0)
		for (i = 0; i < ort->set_bindlevs; i++)
			printf("%s%s,", bind_choice[ort->bind_per_level[i]],
			                (i != ort->set_bindlevs-1) ? "," : "'\n");
	else
		if (ort->icvs.proc_bind == omp_proc_bind_true)
			printf("TRUE'\n");
		else
			if (ort->icvs.proc_bind == omp_proc_bind_false)
				printf("FALSE'\n");

	DISPTFVAR(OMP_CANCELLATION, ort->icvs.cancel, ENABLED, DISABLED);
	DISPTFVAR(OMP_DISPLAY_ENV, display_env == DISPLAY_ENV_TRUE, TRUE, VERBOSE);
	printf("\t[host] OMP_DEFAULT_DEVICE='%d'\n", ort->icvs.def_device);

	show_places(1);

	/* 
	 * OpenMP 4.5
	 */
	
	printf("\t[host] OMP_MAX_TASK_PRIORITY='%d'\n", ort->icvs.max_task_prio);

	/*
	 * OpenMP 5.0
	 */
	DISPTFVAR(OMP_DISPLAY_AFFINITY, ort->icvs.display_affinity, TRUE, FALSE);
	printf("\t[host] OMP_AFFINITY_FORMAT='%s'\n", ort->icvs.affinity_format);
	
	printf("\t[host] OMP_TARGET_OFFLOAD='%s'\n", 
	      ort->icvs.targetoffload == OFFLOAD_DEFAULT ? "DEFAULT" :
	      ort->icvs.targetoffload == OFFLOAD_DISABLED ? "DISABLED" : "MANDATORY");

	/* 
	 * OMPi-specific stuff
	 */
	
	if (display_env == DISPLAY_ENV_VERBOSE)
	{
		printf("\n");
		printf("\t[host] OMPI_TASKQUEUESIZE='%d'\n", ort->taskqueuesize);

		DISPTFVAR(OMPI_DYNAMIC_TASKQUEUESIZE, ort->dynamic_taskqueuesize, 
		          ENABLED, DISABLED);
		DISPTFVAR(OMPI_STEAL_POLICY, ort->ompi_steal_policy == FIFO, FIFO, LIFO);

		if (ort->partotask_policy == FALSE)
			DISPVAR(OMPI_PAR2TASK_POLICY, FALSE);
		else if (ort->partotask_policy == TRUE)
			DISPVAR(OMPI_PAR2TASK_POLICY, TRUE);
		else
			DISPVAR(OMPI_PAR2TASK_POLICY, AUTO);
		
		DISPTFVAR(OMPI_HOSTTARGET_SHARE, ort->module_host.sharedspace, TRUE, FALSE);
		
		/* TODO: display eelib env */
	}

	printf("OPENMP DISPLAY ENVIRONMENT END\n");
}
