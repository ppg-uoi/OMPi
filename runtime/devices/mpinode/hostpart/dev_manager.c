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
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "dev_manager.h"
#include "slave.h"
#include "../../../host/ort_prive.h"


#define MPI_DEV_NAME_CAPACITY 16

/* #define DEBUG */


static int *argc;
static char ***argv;
static char **device_names;
static int num_mpi_devices;


static
void dbg(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "  DBG(dev_mngr): ");
	vfprintf(stderr, format, ap);
	va_end(ap);
}


static
void add_dev_name(char *dev_name, size_t len)
{
	static int device_names_capacity = 0;

	if (num_mpi_devices + 1 >= device_names_capacity)
	{
		device_names_capacity += MPI_DEV_NAME_CAPACITY;
		device_names = realloc(device_names, device_names_capacity * sizeof(char *));
		if (!device_names)
		{
			perror("add_dev_name");
			exit(EXIT_FAILURE);
		}
	}
	device_names[num_mpi_devices++] = strndup(dev_name, len);
}


static
void process_line(char *line)
{
	char *c, *start, *end;

	/* get rid of initial whitespace and detect if comment */
	for (c = line; *c; ++c)
	{
		if (*c == '#')
			return;
		else if (!(isspace(*c)))
			break;
	}
	if (!(*c)) return; /* no more characters */

	/* retrieve node name */
	start = c;
	while (*c && !isspace(*c) && (*c != '#'))
		++c;
	end = c;

	add_dev_name(start, end - start);
}


static
char *get_all_dev_names(void)
{
	char *names;
	int i;
	size_t length = 0, used_length = 0;

	/* calculate length */
	for (i = 0; i < num_mpi_devices; ++i)
	{
		length += strlen(device_names[i]);
	}
	length += num_mpi_devices - 1; /* for , between device names */
	++length; /* for '\0' */

	/* allocate memory for the result */
	names = calloc(length, 1);
	if (!names)
	{
		perror("get_all_dev_names");
		exit(1);
	}

	/* create result */
	for (i = 0; i < num_mpi_devices; ++i)
	{
		strcpy(names + used_length, device_names[i]);
		used_length += strlen(device_names[i]);
		if (i != num_mpi_devices - 1)
		{
			strcpy(names + used_length, ",");
			++used_length;
		}
	}

	return names; /* NOTE: you must free that yourself to avoid memory leaks */
}


static
MPI_Comm start_all_devices(void)
{
	MPI_Info info;
	MPI_Comm spawned_comm, merged_comm;
	int omp_get_num_devices(void);
	int num_total_devices;
	char *all_devs_string = get_all_dev_names();

	MPI_Info_create(&info);
	MPI_Info_set(info, "host", all_devs_string);
#ifdef DEBUG
	char *tmp_argv[] = {"-e", "gdb", *argv[0], NULL};
	MPI_Comm_spawn("/bin/konsole", tmp_argv, get_num_mpi_devices(), info, 0,
			MPI_COMM_WORLD, &spawned_comm, MPI_ERRCODES_IGNORE);
#else
	MPI_Comm_spawn(*argv[0], *argv + 1, get_num_mpi_devices(), info, 0,
			MPI_COMM_WORLD, &spawned_comm, MPI_ERRCODES_IGNORE);
#endif
	MPI_Info_free(&info);
	free(all_devs_string);

	/* Master sets high = 0 (second arg) so the master process gets rank = 0 in
	 * the merged communicator. */
	MPI_Intercomm_merge(spawned_comm, 0, &merged_comm);

	/* Slave processes need to know the corrent number of devices to finish
	 * their initialization.
	 */
	num_total_devices = omp_get_num_devices();
	MPI_Bcast(&num_total_devices, 1, MPI_INT, 0, merged_comm);

	return merged_comm;
}


static
int initialize_thread(void)
{
	extern void ort_init_after_modules(void);
	extern ort_vars_t *ort; /* Pointer to ort_globals */

	MPI_Comm parent_comm, merged_comm;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int is_initialized, provided, name_len, rank, size;

	/* Initialize MPI */
	MPI_Initialized(&is_initialized);
	if (!is_initialized)
	{
		MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
		if (provided != MPI_THREAD_MULTIPLE)
		{
			fprintf(stderr, "Your MPI library was not configured with "
					"MPI_THREAD_MULTIPLE support. Aborting...\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}

	/* All processes (the original and the spawned ones) will eventually
	 * execute this. We want spawned processes (slaves) to wait for
	 * commands from their master instead of continuing to execute user's code.
	 */
	MPI_Comm_get_parent(&parent_comm);
	if (parent_comm != MPI_COMM_NULL)
	{
		/* Slaves set high = 1 (second arg) so the master process gets rank = 0 in
		 * the merged communicator. */
		MPI_Intercomm_merge(parent_comm, 1, &merged_comm);

#ifdef DEBUG
		MPI_Comm_rank(merged_comm, &rank);
		MPI_Comm_size(merged_comm, &size);
		MPI_Get_processor_name(processor_name, &name_len);
		dbg("I'm a SLAVE @ %s: rank = %d/%d, pid = %d\n", processor_name, rank,
				size, getpid());
#endif

		override_devpart_med2dev_addr();
		override_omp_is_initial_device();

		/* Slave processes should continue ort initialization; now they
		 * are stuck at ort_discover_modules(). For the global variables
		 * registration to work, we also need to know the correct number
		 * of devices; host provides us with it.
		 */
		MPI_Bcast(&(ort->num_devices), 1, MPI_INT, 0, merged_comm);
		ort_init_after_modules();
		wait_and_execute(merged_comm);
	}
	else
	{
#ifdef DEBUG
		MPI_Get_processor_name(processor_name, &name_len);
		dbg("I'm the MASTER @ %s: pid = %d\n", processor_name, getpid());
#endif
	}
	return (provided == MPI_THREAD_MULTIPLE);
}


MPI_Comm dev_manager_initialize(int *user_argc, char ***user_argv)
{
	FILE *infile;
	MPI_Comm merged_comm = MPI_COMM_NULL;
	char *line = NULL, *home_dir, filepath[256];
	size_t line_len = 0, home_dir_len;

	if (user_argc && user_argv) /* from host on first device initialization */
	{
		argc = user_argc;
		argv = user_argv;
	}
	else /* host and device initialization */
	{
		initialize_thread();
	}
	if (num_mpi_devices != 0)
	{
		return NULL; /* initialization already done */
	}

	/* create filename */
	home_dir = getenv("HOME");
	home_dir_len = strlen(home_dir);
	strncpy(filepath, home_dir, 255);
	strncpy(filepath + home_dir_len, MPI_CONF_FILE, 255 - home_dir_len);

	/* open configuration file */
	infile = fopen(filepath, "r");
	if (!infile)
	{
		perror("[ORT warning]: Cannot open mpinode configuration file " MPI_CONF_FILE);
		return 0;
	}

	/* process configuration file */
	while (getline(&line, &line_len, infile) != -1)
	{
		process_line(line);
	}

	/* close and free */
	fclose(infile);
	if (line)
	{
		free(line);
	}

	if (user_argc && user_argv) /* from host on first device initialization */
	{
		merged_comm = start_all_devices();
	}

	return merged_comm;
}


int get_num_mpi_devices()
{
	return num_mpi_devices;
}


char *get_device_name(int devid)
{
	return device_names[devid];
}


void dev_manager_finalize()
{
	int i;

	for (i = 0; i < num_mpi_devices; ++i)
	{
		free(device_names[i]);
	}
	free(device_names);
	device_names = NULL;
#ifdef DEBUG
	dbg("CALLED dev_manager_finalize.\n");
#endif
}


#ifdef DEBUG
#undef DEBUG
#endif
