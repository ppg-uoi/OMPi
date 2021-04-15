/*
  OMPi OpenMP Compiler
  == Copyright since 2001, the OMPi Team
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

/* KERNELS.C
 * Module and kernels support for ompicc
 */

/* 
 * May 2019:
 *   Created out of code in ompicc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#include "git_version.h"
#include "str.h"
#include "ompicc.h"
#include "mapper.h"

static
char current_file[PATHSIZE], cwd[PATHSIZE];  /* Current file and working dir */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        MODULES                                            *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char **modulenames;  /* Pointers to module names */
int  nmodules;


void modules_employ(char *modstring)
{
	char *s;
	int  i;

	/* Pass 1: find # modules */
	if (!modstring)
		modstring = strdup(MODULES_CONFIG);
	for (; isspace(*modstring) || *modstring == ','; modstring++)
		;
	if (*modstring == 0)
		return;
	for (nmodules = 1, s = modstring; *s; s++)
	{
		if (isspace(*s) || *s == ',')
		{
			for (*s = 0, s++; isspace(*s) || *s == ','; s++)
				*s = ',';  /* all spaces become commas */
			if (*s)
				nmodules++;
			s--;
		}
	}

	/* Pass 2: fix pointers */
	if ((modulenames = (char **) malloc(nmodules*sizeof(char *))) == NULL)
	{
		fprintf(stderr, "cannot allocate memory");
		exit (1);
	}
	for (i = 0, s = modstring; i < nmodules; i++)
	{
		for (modulenames[i] = s++; *s; s++)
			;
		if (i == nmodules-1)
			break;
		for (; *s == 0 || *s == ','; s++)
			;
	}
}


char *modules_argfor_ompi()
{
	if (nmodules)
	{
		str modstr = Strnew();  /* String for formating module names */
		int i;

		for (i = 0; i < nmodules; i++)
			str_printf(modstr, "%s-usemod=%s", i ? " -" : "-", modulenames[i]);
		return ( str_string(modstr) );
	}
	return ("");
}


#ifdef HAVE_DLOPEN

#include <dlfcn.h>

static void *module_open(char *name)
{
	void *handle;
	str tmp = Strnew();

	/* TODO: maybe need to check other dirs when portable? */
	str_printf(tmp, "%s/devices/%s/hostpart.so", LibDir, name);
	handle = dlopen(str_string(tmp), RTLD_LAZY);
	str_free(tmp);
	return handle;
}


void modules_show_info(int verbose)
{
	int   i, j, md, ndev, (*get_num_devices)();
	void  *modh, (*print_info)(int);
	char  *error;

	for (i = ndev = 0; i < nmodules; i++)
	{
		if (verbose)
			fprintf(stderr, "MODULE [%s]:\n------\n", modulenames[i]);
		else
			fprintf(stderr, "  MODULE [%s] provides device(s) : ", modulenames[i]);
		
		modh = module_open(modulenames[i]);
		if (!modh)
		{
			fprintf(stderr, "module failed to open.\n");
			if (verbose)
				fprintf(stderr, "  [ reported error: %s ]\n", dlerror());
		}
		else
		{
			if (verbose)
			{
				print_info = dlsym(modh, "hm_print_information");
				if ((error = dlerror()) != NULL)
					fprintf(stderr, "%s\n", error);
				else
					print_info(ndev+1);    /* Host is device #0 */
			}

			get_num_devices = dlsym(modh, "hm_get_num_devices");
			if ((error = dlerror()) != NULL)
				fprintf(stderr, "%s\n", error);
			else
				md = get_num_devices();
			
			if (!verbose)
			{
				for (j = 0; j < md; j++)
					fprintf(stderr, "%d ", ndev+j+1);
				fprintf(stderr, "\n");
			}
			ndev += md;
			dlclose(modh);
		}
		if (verbose)
			fprintf(stderr, "------\n\n");
	}
	if (verbose)
		fprintf(stderr, "Total number of available devices: %d\n", ndev);
}

#else

void modules_show_info()
{
	fprintf(stderr, "Unfortunately, there is no support for modules.\n");
}

#endif



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        CREATING & EXECUTING MAKEFILES FOR KERNELS         *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
void km_substitute(char *var, int maxlen, str s, char *modulename, int kernid)
{
	// FIXME: is goutfile set here?
	if (strncmp("@@OMPI_OUTPUT@@", var, maxlen) == 0)
		str_printf(s, "%s", (user_outfile.head && strlen(user_outfile.head->val) != 0) ?
		                    user_outfile.head->val : "a.out");
	else if (strncmp("@@OMPI_LIBDIR@@", var, maxlen) == 0)
		str_printf(s, LibDir);
	else if (strncmp("@@OMPI_KERNELMAKEFILE@@", var, maxlen) == 0)
		str_printf(s, "%s-makefile", modulename);
	else if (strncmp("@@OMPI_MODULE@@", var, maxlen) == 0)
		str_printf(s, "%s", modulename);
	else if (strncmp("@@OMPI_CC@@", var, maxlen) == 0)
		str_printf(s, "%s", COMPILER);
	else if (strncmp("@@OMPI_CFLAGS@@", var, maxlen) == 0)
		str_printf(s, "%s", CFLAGS);
	else if (strncmp("@@OMPI_LDFLAGS@@", var, maxlen) == 0)
		str_printf(s, "%s", LDFLAGS);
	else if (strncmp("@@OMPI_KERNELID@@", var, maxlen) == 0)
		str_printf(s, "%02d", kernid);
	else if (strncmp("@@OMPI_KERNELFILE@@", var, maxlen) == 0)
		str_printf(s, "%s", current_file);
	else
		str_printf(s, "%*.*s", maxlen, maxlen, var);
}


static
void km_substitute_vars(char *line, str s, char *modulename, int kernid)
{
	char *p;

	for (; isspace(*line); line++)  /* skip spaces */
		if (*line == 0)
			return;
		else
			str_putc(s, *line);

	if (*line == '#')
	{
		str_printf(s, "%s", line);    /* comment line */
		return;
	}

	while (1)
	{
		for (; *line != '@'; line++)
			if (*line == 0)
				return;
			else
				str_putc(s, *line);

		p = line+1;
		if (*p != '@')
			str_putc(s, '@');
		else
		{
			for (p++; *p != '@'; p++)
				if (*p == 0)
				{
					str_printf(s, "%s", line);
					return;
				}
				else
					if (isspace(*p))
					{
						str_printf(s, "%*.*s", p-line, p-line, line);
						break;
					};

			if (*p == '@')
			{
				if (*(p++) == '@')
				{
					km_substitute(line, p-line+1, s, modulename, kernid);
					p++;
				}
			}
		}
		line = p;
	}
}


static
int kernel_makefile_run(char *path, char *modulename)
{
	char cmd[SLEN];
	int  res, eflag;

	if (strcmp(path, ".") != 0)
	{
		if (*cwd == 0)
		{
			perror("getcwd():");
			return (1);
		}
		if (verbose)
			fprintf(stderr, "  ===> changing to directory %s\n", path);
		if (chdir(path) < 0)
		{
			perror("chdir():");
			return (1);
		}
	}

	if (verbose)
		fprintf(stderr, "  ===> make -f %s-makefile\n", modulename);
	snprintf(cmd, SLEN, "make -f %s-makefile", modulename);
	res = sysexec(cmd, &eflag);

	if (strcmp(path, ".") != 0)
	{
		if (verbose)
			fprintf(stderr, "  ===> returning to directory %s\n", cwd);
		if (chdir(cwd) < 0)
		{
			perror("chdir():");
			return (1);
		}
	}
	if (eflag || res)
		return (1);
}


static
int kernel_makefile_create(char *path, char *modulename, int kernid, 
                           mapmod_t mmod)
{
	static str s = NULL;
	char   line[SLEN], filepath[SLEN], *flavor, *ext;
	FILE   *fpin, *fpout;
	
	if (!usecarstats || !mmod)
		snprintf(filepath, SLEN, "%s/devices/%s/MakeKernel.%s",
		                   LibDir, modulename, modulename);
	else
	{
		if ((ext = strrchr(current_file, '.')) != NULL)
			*ext = 0; /* remove extension */
		else
			ext = current_file + strlen(current_file);
		snprintf(ext, PATHSIZE-strlen(current_file), "_d%02d.c", kernid);

		/* Mapper arguments */
		snprintf(filepath, SLEN, "%s/%s", cwd, current_file);
		flavor = mapper_select_flavor(mmod, filepath);
		
		/* Check for actual flavor and form the kernel makefile installation path */
		if (flavor == NULL || strcmp(flavor, "devpart") == 0)
			snprintf(filepath, SLEN, "%s/devices/%s/MakeKernel.%s",
			         LibDir, modulename, modulename);
		else
		{
			char *flv = flavor;    /* skip "devpart" from returned flavor name */
			if (strlen(flavor) >= 7)
				flv = (flavor[7] == 0) ? flavor+7 : flavor+8;
			snprintf(filepath, SLEN, "%s/devices/%s/MakeKernel-%s.%s",
			         LibDir, modulename, flv, modulename);
		}
		if (verbose)
			fprintf(stderr, "  kernel makefile selected (mapper):\n\t%s\n", filepath);
	}

	if ((fpin = fopen(filepath, "r")) == NULL)
	{
		fprintf(stderr, "[***] cannot find 'MakeKernel.%s' recipe for creating "
		                "%s kernels.\n", modulename, modulename);
		return (1);
	}

	snprintf(filepath, SLEN, "%s/%s-makefile", path, modulename);
	if ((fpout = fopen(filepath, "w")) == NULL)
	{
		fprintf(stderr, "[***] cannot generate '%s/%s-makefile' to create "
		                "kernels for device %s\n.", path, modulename, modulename);
		fclose(fpin);
		return (1);
	}

	if (s)
		str_truncate(s);
	else
		s = Strnew();

	while (fgets(line, SLEN, fpin) != NULL)
		km_substitute_vars(line, s, modulename, kernid);
	fprintf(fpout, "%s", str_string(s));

	fclose(fpin);
	fclose(fpout);

	if (verbose)
		fprintf(stderr, "  ( %s )\n", filepath);
	kernel_makefile_run(path, modulename);

	if (keep < 2)    /* Remove makefile */
		unlink(filepath);
	
	return (0);
}


/* Generate a makefile for each requested module @ the given path */
void kernel_makefiles(char *fname, int nkernels)
{
	int      i, j;
	char     filepath[PATHSIZE], *s;
	mapmod_t mmod;
		
	if ((s = strrchr(fname, '/')) == NULL)
		strcpy(filepath, ".");
	else
	{
		strncpy(filepath, fname, s-fname); /* Strip filename from the path */
		filepath[s-fname] = 0;
	}
	
	if (!getcwd(cwd, PATHSIZE))
		*cwd = 0;
	
	for (i = 0; i < nmodules; i++)
	{
		mmod = usecarstats ? mapper_load_module(modulenames[i]) : NULL;
		for (j = 0; j < nkernels; j++)
		{
			sprintf(current_file, "%s\n", fname);
			kernel_makefile_create(filepath, modulenames[i], j, mmod);
		}
		if (mmod)
			mapper_free_module(mmod);
	}

	if (!keep)       /* TODO: Remove kernels!! */
	{
		strcpy(filepath, fname);
		if ((s = strrchr(filepath, '.')) != NULL)
			*s = 0; /* remove extension */
		else
			s = filepath + strlen(filepath);
		for (i = 0; i < nkernels; i++)
		{
			snprintf(s, PATHSIZE-strlen(filepath), "_d%02d.c", i);
			unlink(filepath);
		}
	}
}
