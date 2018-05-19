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

/* ort_env.c */

/*
 * 2014/06/13:
 *   First time around.
 */

#include "ort_prive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

/* display env values */
#define DISPLAY_ENV_FALSE   0
#define DISPLAY_ENV_TRUE    1
#define DISPLAY_ENV_VERBOSE 2
int display_env = DISPLAY_ENV_FALSE;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * PLACES                                                            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ISTRUE(s) \
	(strncmp(s,"true", 4) == 0 || strncmp(s,"TRUE", 4) == 0 || strncmp(s,"1", 1) == 0)

#define ISFALSE(s) \
	(strncmp(s,"false", 5) == 0 || strncmp(s,"FALSE", 5) == 0 || strncmp(s,"0", 1) == 0)

jmp_buf j_placeserr;
#define j_places_error(n) longjmp(j_placeserr, n)

static
void addplace(int ***places, int *list, int n)
{
	int p;

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
}


void getinterval(char **s, int lb, int *len, int *stride)
{
	char *t = *s;

	sscanf(++t, "%d", len);      /* Assume we are on a ':' */
	if (*len <= 0)
		j_places_error(1);
	for (; isdigit(*t); t++)
		;
	if (*t != ':')    /* no stride */
		*stride = 1;
	else
	{
		sscanf(++t, "%d", stride);
		if (*stride == 0)
			j_places_error(1);
		if (*stride < 0)
		{
			t++;
			if (lb + (*stride) * (*len - 1) < 0) /* Check numbers are non-negative */
				j_places_error(1);
		}
		for (; isdigit(*t); t++)
			;
	}
	*s = t;
}


void getplace(int ***places, char **s)
{
	char *t = *s;
	int  lb = -1, len = -1, stride = -1, p, n, list[MAXPLACELEN];
	int  pi, pnum = -1, pstr = -1, iscommalist = 0;

	if (*(t++) != '{')
		j_places_error(1);
	sscanf(t, "%d", &lb);
	if (lb < 0)
		j_places_error(1);
	for (; isdigit(*t); t++)
		;
	switch (*t)
	{
		case '}':  /* or just 1 number */
			list[0] = lb;
			n = len = stride = 1;
			break;

		case ',':  /* comma list */
			list[0] = lb; n = 1;
			for (lb = -1; *t == ','; lb = -1)
			{
				sscanf(++t, "%d", &lb);
				if (lb < 0)
					j_places_error(1);
				for (; isdigit(*t); t++)
					;
				list[n++] = lb;
			}
			iscommalist = 1;
			break;

		case ':':  /* interval */
			getinterval(&t, lb, &len, &stride);
			if (len > MAXPLACELEN)
				j_places_error(3);
			for (n = 0; n < len; n++)
				list[n] = lb + n * stride;
			break;

		default:
			j_places_error(1);
	}

	if (*(t++) != '}')
		j_places_error(1);

	if (*t != ':')     /* Just a single place */
	{
		*s = t;
		addplace(places, list, n);
		return;
	}
	else               /* A range of places */
	{
		if (iscommalist)
			j_places_error(1);
		getinterval(&t, lb, &pnum, &pstr);
		for (pi = 0; pi < pnum; pi++)   /* Construct a new place */
		{
			for (n = 0; n < len; n++)
				list[n] = lb + n * stride;
			addplace(places, list, n);
			lb += pstr;
		}
		*s = t;
	}
}


int sys_get_num_threads()
{
	return ort->icvs.ncpus;
}


int sys_get_num_cores()
{
	return ort->icvs.ncpus;
}


int sys_get_num_sockets()
{
	return (1);
}

/* size | sizeB | sizeK | sizeM | sizeG */
static long int _size_bytes(char *str)
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
	ort_warning("incorrect OMP_STACKSIZE specification; using 256K.\n", str);
	return (KBytes(256));
}

static
void ort_getenv_places(char *s)
{
	int  n, i;
	int **places = NULL;// = &(ort->icvs.place_partition);

	if ((n = setjmp(j_placeserr)) != 0)
	{
		switch (n)
		{
			case 1:
				ort_warning("incorrect value of OMP_PLACES e.v.; ignoring.\n");
				break;
			case 2:
				ort_warning("out of memory while parsing OMP_PLACES e.v.; ignoring\n");
				break;
			case 3:
				ort_warning("cannot support more than %d numbers for a place.\n",
				            MAXPLACELEN);
		}
		if (places)  /* Cleanup */
			places_free(places);
		return;
	}

	if (*s != '{')   /* abstract places */
	{
		if (strncmp(s, "threads", 7) == 0)
		{
			n = sys_get_num_threads();
			s += 7;
		}
		else
			if (strncmp(s, "cores", 5) == 0)
			{
				n = sys_get_num_cores();
				s += 5;
			}
			else
				if (strncmp(s, "sockets", 7) == 0)
				{
					n = sys_get_num_sockets();
					s += 7;
				}
				else
					j_places_error(1);

		if (*s == '(')
		{
			sscanf(s, "(%d)", &i);
			if (i <= 0)
				j_places_error(1);
			if (i < n)            /* Request for more places: nope */
				n = i;              /* Request for fewer places: 0, 1, ... i */
		}
		else
		{
			if (*s != 0)
				j_places_error(1);
		}

		for (i = 0; i < n; i++)               /* Create the places */
			addplace(&places, &i, 1);
	}
	else             /* numeric places */
		for (getplace(&places, &s); *s == ','; getplace(&places, &s))
			s++;
		
	if(places)
	{
		places_free(ort->icvs.place_partition);
		ort->icvs.place_partition = places;
	}
}

/* Default placement is to use one core per OpenMP thread */
void ort_get_default_places(void)
{
	int  n, i;
	int ***places = &(ort->icvs.place_partition);

	n = sys_get_num_cores();
	for (i = 0; i < n; i++)               /* Create the places */
		addplace(places, &i, 1);
}

static
void show_places()
{
	int i, j;
	int **places = ort->icvs.place_partition;

	if (numplaces(places) > 0)
	{
		printf("\t[host] OMP_PLACES (%d places defined) = ", numplaces(places));
		for (i = 0; i < numplaces(places); i++)
		{
			printf("{");
			for (j = 0; j < placelen(places, i); j++)
				printf("%d%c", places[i + 1][j + 1],
				       (j == placelen(places, i) - 1) ? '}' : ',');
			printf("%s", (i == numplaces(places) - 1) ? "\n" : ", ");
		}
	}
	else
		printf("\t[host] OMP_PLACES (0 places defined)\n");
}

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
				ort->icvs.rtchunk = n;
			else
				ort_warning("illegal chunksize in environment "
				            "variable OMP_SCHEDULE; using default value\n");
		}
		if (strncmp(s, "static", 6) == 0)
			ort->icvs.rtschedule = omp_sched_static;
		else
			if (strncmp(s, "dynamic", 7) == 0)
				ort->icvs.rtschedule = omp_sched_dynamic;
			else
				if (strncmp(s, "guided", 6) == 0)
					ort->icvs.rtschedule = omp_sched_guided;
				else
					if (strncmp(s, "auto", 4) == 0)
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
		ort->icvs.stacksize = _size_bytes(s);
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
		if (strncmp(s, "active", 6) == 0 || strncmp(s, "ACTIVE", 6) == 0)
			ort->icvs.waitpolicy = _OMP_ACTIVE;
		else
			if (strncmp(s, "passive", 7) == 0 || strncmp(s, "PASSIVE", 7) == 0)
				ort->icvs.waitpolicy = _OMP_PASSIVE;
			else
				ort_warning("incorrect value of environment "
				            "variable OMP_WAIT_POLICY; ignoring\n");
	}

	/* OpenMP 4.0.0 */
	if ((s = getenv("OMP_CANCELLATION")) != NULL)
	{
		if (ISTRUE(s))
			ort->icvs.cancel = 1;
	}

	/* OpenMP 4.0.0 */
	if ((s = getenv("OMP_DISPLAY_ENV")) != NULL)
	{
		if (ISTRUE(s))
			display_env = DISPLAY_ENV_TRUE;
		else
			if (strncmp(s, "VERBOSE", 7) == 0)
				display_env = DISPLAY_ENV_VERBOSE;
	}

	/* OpenMP 4.0.0 */
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
						if (strncmp(bind, "master", 6) == 0)
							bind_choice = omp_proc_bind_master;
						else
							if (strncmp(bind, "close", 5)  == 0)
								bind_choice = omp_proc_bind_close;
							else
								if (strncmp(bind, "spread", 6) == 0)
									bind_choice = omp_proc_bind_spread;
								else
								{
									ort_warning("illegal value in environment variable "
									            "OMP_PROC_BIND; using default values\n");
									l = 0;
									break;
								};

				if (l < MAXBINDLEVS)
					ort->bind_per_level[l++] = bind_choice;
				else
				{
					ort_warning("too many levels in OMP_PROC_BIND environment "
					            "variable;\n  using the first %d.\n", MAXBINDLEVS);
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

	if ((s = getenv("OMP_NUM_THREADS")) != NULL)
	{
		int  l = 0;
		char *t = s - 1;

		do
		{
			if (sscanf(t + 1, "%d", &n) == 1 && n > 0)
			{
				if (l < MAXNTHRLEVS)
					ort->nthr_per_level[l++] = n;
				else
				{
					ort_warning("too many levels in OMP_NUM_THREADS environment "
					            "variable;\n  using the first %d.\n", MAXNTHRLEVS);
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

	if ((s = getenv("OMP_PLACES")) != NULL)
		ort_getenv_places(s);

	if (((s = getenv("OMP_DEFAULT_DEVICE")) != NULL) && (sscanf(s, "%d", &n)==1))
	{
		if(n >= 0 && n < ort->added_devices)
			ort->icvs.def_device = n;
		else
			ort_warning("Invalid OMP_DEFAULT_DEVICE value; the first available device"
			            "will be used as default.\n");
	}


	/* Ompi specific */
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
		if (strncmp(s, "FIFO", 4) == 0)
			ort->ompi_steal_policy = FIFO;
		else
			if (strncmp(s, "LIFO", 4) == 0)
				ort->ompi_steal_policy = LIFO;
	}
	else
		ort->ompi_steal_policy = FIFO;
}

/* OpenMP 4.0.0 */
void display_env_vars()
{
	char *rtschedule;
	char *rtchunk;
	int i;
	char *bind_choice[5] = {"FALSE", "TRUE", "MASTER", "CLOSE", "SPREAD"};
	size_t size;
	int ret;

	switch (ort->icvs.rtschedule)
	{
		case omp_sched_static:
			rtschedule = "STATIC";
			break;
		case omp_sched_dynamic:
			rtschedule = "DYANAMIC";
			break;
		case omp_sched_guided:
			rtschedule = "GUIDED";
			break;
		case omp_sched_auto:
			rtschedule = "AUTO";
			break;
		default:
			rtschedule = "STATIC";
			break;
	}

	fprintf(stderr, "OPENMP DISPLAY ENVIRONMENT BEGIN\n");

	fprintf(stderr, "\t_OPENMP='%d'\n", _OPENMP);

	/* OMP_SCHEDULE */
	if (ort->icvs.rtchunk == -1)
		fprintf(stderr, "\t[host] OMP_SCHEDULE='%s, AUTO'\n", rtschedule);
	else
		fprintf(stderr, "\t[host] OMP_SCHEDULE='%s, %d'\n", rtschedule,
		        ort->icvs.rtchunk);

	/* OMP_NUM_THREADS */
	fprintf(stderr, "\t[host] OMP_NUM_THREADS='");
	if (ort->set_nthrlevs > 0)
	{
		for (i = 0; i < ort->set_nthrlevs; i++)
		{
			fprintf(stderr, "%d", ort->nthr_per_level[i]);
			if (i != ort->set_nthrlevs - 1)
				fprintf(stderr, ",");
			else
				fprintf(stderr, "'\n");
		}
	}
	else
	{
		if (ort->icvs.nthreads == -1)
			fprintf(stderr, "%d'\n", ort->icvs.ncpus);
		else
			fprintf(stderr, "%d'\n", ort->icvs.nthreads);
	}

	/* OMP_DYNAMIC */
	if (ort->icvs.dynamic == 1)
		fprintf(stderr, "\t[host] OMP_DYNAMIC='TRUE'\n");
	else
		fprintf(stderr, "\t[host] OMP_DYNAMIC='FALSE'\n");

	/* OMP_PROC_BIND */
	fprintf(stderr, "\t[host] OMP_PROC_BIND='");
	if (ort->set_bindlevs > 0)
	{
		for (i = 0; i < ort->set_bindlevs; i++)
		{
			fprintf(stderr, "%s", bind_choice[ort->bind_per_level[i]]);
			if (i != ort->set_bindlevs - 1)
				fprintf(stderr, ",");
			else
				fprintf(stderr, "'\n");
		}
	}
	else
		if (ort->icvs.proc_bind == omp_proc_bind_true)
			fprintf(stderr, "TRUE'\n");
		else
			if (ort->icvs.proc_bind == omp_proc_bind_false)
				fprintf(stderr, "FALSE'\n");

	/* OMP_PLACES */
	show_places();

	/* OMP_NESTED */
	if (ort->icvs.nested == 1)
		fprintf(stderr, "\t[host] OMP_NESTED='TRUE'\n");
	else
		fprintf(stderr, "\t[host] OMP_NESTED='FALSE'\n");

	/* OMP_STACKSIZE */
	if (ort->icvs.stacksize == -1)
	{
		pthread_attr_t tattr;
		int ret = pthread_attr_init(&tattr);
		ret = pthread_attr_getstacksize(&tattr, &size);
		fprintf(stderr, "\t[host] OMP_STACKSIZE='%d bytes'\n", (int)size);
	}
	else
		fprintf(stderr, "\t[host] OMP_STACKSIZE='%d bytes'\n", (int)ort->icvs.stacksize);

	/* OMP_WAIT_POLICY */
	if (ort->icvs.waitpolicy == _OMP_PASSIVE)
		fprintf(stderr, "\t[host] OMP_WAIT_POLICY='PASSIVE'\n");
	else
		fprintf(stderr, "\t[host] OMP_WAIT_POLICY='ACTIVE'\n");

	/* OMP_MAX_ACTIVE_LEVELS */
	if (ort->icvs.levellimit == -1)
		fprintf(stderr, "\t[host] OMP_MAX_ACTIVE_LEVELS='NO LIMIT'\n");
	else
		fprintf(stderr, "\t[host] OMP_MAX_ACTIVE_LEVELS='%d'\n", ort->icvs.levellimit);

	/* OMP_THREAD_LIMIT */
	if (ort->icvs.threadlimit == -1)
		fprintf(stderr, "\t[host] OMP_THREAD_LIMIT='NO LIMIT'\n");
	else
		fprintf(stderr, "\t[host] OMP_THREAD_LIMIT='%d'\n", ort->icvs.threadlimit);

	/* OMP_CANCELLATION */
	if (ort->icvs.cancel)
		fprintf(stderr, "\t[host] OMP_CANCELLATION='ENABLED'\n");
	else
		fprintf(stderr, "\t[host] OMP_CANCELLATION='DISABLED'\n");

	/* OMP_DISPLAY_ENV */
	if (display_env == DISPLAY_ENV_TRUE)
		fprintf(stderr, "\t[host] OMP_DISPLAY_ENV='TRUE'\n");
	else
		fprintf(stderr, "\t[host] OMP_DISPLAY_ENV='VERBOSE'\n");

	/* OMP_DEFAULT_DEVICE */
	if (ort->icvs.def_device == HOST_ID)
		fprintf(stderr, "\t[host] OMP_DEFAULT_DEVICE='HOST CPU'\n");
	else
		fprintf(stderr, "\t[host] OMP_DEFAULT_DEVICE='%d'\n", ort->icvs.def_device);

	if (display_env == DISPLAY_ENV_VERBOSE)
	{
		/* OMPI_TASKQUEUESIZE */
		fprintf(stderr, "\t[host] OMPI_TASKQUEUESIZE='%d'\n", ort->taskqueuesize);

		/* OMPI_DYNAMIC_TASKQUEUESIZE */
		if (ort->dynamic_taskqueuesize == 1)
			fprintf(stderr, "\t[host] OMPI_DYNAMIC_TASKQUEUESIZE='ENABLED'\n");
		else
			fprintf(stderr, "\t[host] OMPI_DYNAMIC_TASKQUEUESIZE='DISABLED'\n");

		/* OMPI_STEAL_POLICY */
		if (ort->ompi_steal_policy == FIFO)
			fprintf(stderr, "\t[host] OMPI_STEAL_POLICY='FIFO'\n");
		else
			fprintf(stderr, "\t[host] OMPI_STEAL_POLICY='LIFO'\n");
	}

	fprintf(stderr, "OPENMP DISPLAY ENVIRONMENT END\n");
}
