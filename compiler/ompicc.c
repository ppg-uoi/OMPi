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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* OMPICC
 * A driver for the OMPi compiler.
 */

/*
 * Oct 2010:
 *   Improvements; correct argument quoting.
 * Apr 2010:
 *   Some improvements
 * Aug 2007:
 *   Major simplifications due to new threadprivate implementation.
 *   Removed POMP support.
 * May 2007:
 *   Fixed some definitions for compiling/linking/preprocessing.
 *   -lrt now determined at configuration time.
 *   Some less important bug fixes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#if defined(HAVE_REALPATH)
	#include <limits.h>
#endif

#include "config.h"
#include "boolean.h"
#include "git_version.h"
#include "str.h"
#include "keyval.h"

#define LEN  4096
#define SLEN 1024

/*
 * Modules
 */

char *modulestring = NULL;
char **modulenames;  /* Pointers to module names */
int  nmodules;


void modulestring_to_modulenames()
{
	char *s;
	int  i;

	/* Pass 1: find # modules */
	if (!modulestring)
		modulestring = strdup(MODULES_CONFIG);
	for (; isspace(*modulestring) || *modulestring == ','; modulestring++)
		;
	if (*modulestring == 0)
		return;
	for (nmodules = 1, s = modulestring; *s; s++)
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
	for (i = 0, s = modulestring; i < nmodules; i++)
	{
		for (modulenames[i] = s++; *s; s++)
			;
		if (i == nmodules-1)
			break;
		for (; *s == 0 || *s == ','; s++)
			;
	}
}


char *modules_to_ompi(char *s)
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
	str_printf(tmp, "%s/devices/%s/hostmodule.so", LibDir, name);
	handle = dlopen(str_string(tmp), RTLD_LAZY);
	str_free(tmp);
	return handle;
}


void modules_show_info()
{
	int   i, ndev, (*get_num_devices)();
	void  *modh, (*print_info)(int);
	char  *error;

	for (i = ndev = 0; i < nmodules; i++)
	{
		fprintf(stderr, "MODULE [%s]:\n------\n", modulenames[i]);
		modh = module_open(modulenames[i]);
		dlerror();
		if (!modh)
			fprintf(stderr, "module failed to open.\n");
		else
		{
			print_info = dlsym(modh, "hm_print_information");
			if ((error = dlerror()) != NULL)
				fprintf(stderr, "%s\n", error);
			else
				print_info(i+1);    /* Host is device #0 */

			get_num_devices = dlsym(modh, "hm_get_num_devices");
			if ((error = dlerror()) != NULL)
				fprintf(stderr, "%s\n", error);
			else
				ndev += get_num_devices();
			dlclose(modh);
		}
		fprintf(stderr, "------\n\n");
	}
	fprintf(stderr, "Total number of available devices: %d\n", ndev);
}

#else

void modules_show_info()
{
	fprintf(stderr, "Unfortunately, there is no support for modules.\n");
}

#endif

/* Definitions (macros) provided extrenally (see Makefile.am)
 *
 * OmpiName
 * CPPcmd
 * CCcmd
 * PreprocFlags
 * CompileFlags
 * LinkFlags
 * IncludeDir
 * LibDir
 */

/* Flags collected from the OMPI_CPP, OMPI_CPPFLAGS,
 * OMPI_CC, OMPI_CFLAGS and OMPI_LDFLAGS, from the ./configure info and the
 * library-specific configuration file.
 */
#ifdef PATH_MAX
	#define PATHSIZE PATH_MAX
	#define FLAGSIZE PATH_MAX
#else
	#define PATHSIZE 4096
	#define FLAGSIZE 4096
#endif
char PREPROCESSOR[PATHSIZE], COMPILER[PATHSIZE],
     CPPFLAGS[FLAGSIZE], CFLAGS[FLAGSIZE], LDFLAGS[FLAGSIZE],
     ORTINFO[FLAGSIZE];
char ortlibname[PATHSIZE],           /* The runtime library needed */
     RealOmpiName[PATHSIZE];

#ifdef PORTABLE
	#undef LibDir
	#undef IncludeDir
	char CompilerPath[PATHSIZE], LibDir[PATHSIZE], IncludeDir[PATHSIZE];
#endif

typedef struct arg_s
{
	char opt;
	char val[SLEN];
	struct arg_s *next;
} arg_t;

typedef struct
{
	arg_t *head, *tail;
} arglist_t;


#define ompi_info() \
	fprintf(stderr,\
	 "This is %s %s using\n  >> system compiler: %s\n  >> runtime library: %s\n"\
	                      "  >> config. devices: %s\n",\
	  PACKAGE_NAME, GIT_VERSION, COMPILER, *ORTINFO ? ORTINFO : ortlibname,\
		MODULES_CONFIG)


static void ompicc_error(int exitcode, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "[ompicc error]: ");
	vfprintf(stderr, format, ap);
	va_end(ap);

	exit(exitcode);
}


static char *get_basename(char *path)
{
	char *s;

	if (path == NULL || *path == 0)
		return ".";
	else
		if (path[0] == '/' && path[1] == 0)
			return path;
		else
		{
			s = path;
			while (*s)
				s++;
			s--;
			while (*s == '/' && s != path)   /* destructive */
				*s-- = 0;
			if (s == path)
				return path;
			while (*s != '/' && s != path)
				s--;
			if (*s == '/')
				return s + 1;
			else return path;
		}
}


static arg_t *new_arg(char opt, char *val)
{
	arg_t *p;

	if ((p = (arg_t *) malloc(sizeof(arg_t))) == NULL)
		ompicc_error(-1, "malloc() failed\n");
	p->opt = opt;
	if (val != NULL)
		strcpy(p->val, val);
	else p->val[0] = 0;
	p->next = NULL;
	return p;
}


void arglist_add(arglist_t *l, arg_t *arg)
{
	if (l->head == NULL)
		l->head = l->tail = arg;
	else
	{
		l->tail->next = arg;
		l->tail = arg;
	}
}


int append_arg(arglist_t *l, int argc, char **argv, int proceed)
{
	char opt, val[SLEN];
	arg_t *p;

	val[0] = 0;
	if (argv[0][0] == 0)
		return 0;
	if (argv[0][0] != '-')
	{
		p = new_arg(0, *argv);
		arglist_add(l, p);
		return 0;
	}
	opt = argv[0][1];

	if (argv[0][2] != 0)
	{
		strcpy(val, &argv[0][2]);
		p = new_arg(opt, val);
		arglist_add(l, p);
		return 0;
	}
	else
	{
		if (proceed && argc > 1)
			strcpy(val, &argv[1][0]);
		p = new_arg(opt, val);
		arglist_add(l, p);
		return proceed && argc > 1;
	}
}


static int quotedlen(char *s)
{
	int len;
	for (len = 0; *s != 0; s++)
		len += (*s == '"' ? 2 : 1);
	return (len);
}


/* Prepare a string from an argument list; all args are quoted */
static void strarglist(char *dest, arglist_t *l, int maxlen)
{
	arg_t *p;
	char  *c, *d = dest;

	for (*d = 0, p = l->head; p != NULL; p = p->next)
	{
		if ((d - dest) + quotedlen(p->val) + 6 >= maxlen)
			ompicc_error(1, "argument(s) too long; rebuild OMPi with larger LEN.\n");
		if (p->opt)
		{
			sprintf(dest, "-%c ", p->opt);
			dest += ((p->opt == 'o') ? 3 : 2);
		}
		*(dest++) = '"';
		for (c = p->val; *c != 0; c++)
		{
			if (*c == '"') *(dest++) = '\\';
			*(dest++) = *c;
		}
		*(dest++) = '"';
		*(dest++) = ' ';
	}
	*dest = 0;
}


int fok(char *fname)
{
	struct stat buf;

	return (stat(fname, &buf) == 0);
}


bool disableOpenMP = false,
     disableOmpix = false,
     disableCodeDuplication = false,
     cppLineNo = false;
bool mustlink = true;     /* Becomes 0 if -c parameter is specified */
bool makefile = true;     /* Becomes 0 if --nomakefile parameter is specified */
int  keep = 0;            /* Becomes 1/2 if -k/K parameter is specified */
bool verbose = false;
bool showdbginfo = false; /* Force the compiler to show some debugging info */
bool showdevinfo = false; /* Display info about configured devices */
bool usegdb = false;      /* If true _ompi will be run through gdb. For optimal
                           * results ompi must be configured with CFLAGS=-Og */

arglist_t files     = { NULL, NULL };      /* Files to be compiled/linked */
arglist_t goutfile  = { NULL, NULL };      /* Output, -o XXX */
arglist_t prep_args = { NULL, NULL };      /* Preprocessor args */
arglist_t link_args = { NULL, NULL };      /* Linker args */
arglist_t scc_flags = { NULL, NULL };      /* Remaining system compiler args */


/*
 * Options
 */


#define OPTNAME(opt)   "--" #opt
#define OPTNAME_V(opt) "V--" #opt "="
#define OPTION(opt)    OPT_##opt

typedef enum {
	OPTION(unknown) = -1, /* unknown option */
	OPTION(ort) = 0,  OPTION(nomp),        OPTION(nox),
	OPTION(nocodup),  OPTION(nomakefile),  OPTION(nolineno),
	OPTION(gdb),      OPTION(dbg),         OPTION(devs),
	OPTION(devinfo),
	OPTION(lastoption)    /* dummy */
} option_t;

char *optnames[] = {
	OPTNAME_V(ort),    OPTNAME(nomp),        OPTNAME(nox),
	OPTNAME(nocodup),  OPTNAME(nomakefile),  OPTNAME(nolineno),
	OPTNAME(gdb),      OPTNAME(dbg),         OPTNAME_V(devs),
	OPTNAME(devinfo)
};

option_t optid(char *arg, char **val)
{
	int i;

	for (i = 0; i < OPTION(lastoption); i++)
		if (optnames[i][0] == 'V')     /* Option with value */
		{
			if (strncmp(optnames[i]+1, arg, strlen(optnames[i])-1) == 0)
			{
				*val = arg + strlen(optnames[i]) - 1;
				return ((option_t) i);
			}
		}
		else
			if (strcmp(optnames[i], arg) == 0)
				return ((option_t) i);
	return ( OPTION(unknown) );
}


void parse_args(int argc, char **argv)
{
	int  d, ortlib = 0;
	char *parameter, *val;

	argv++;
	argc--;

	while (argc)
	{
		d = 0;
		switch ( optid(parameter = argv[0], &val) )
		{
			case OPTION(ort):
				strncpy(ortlibname, val, 511);
				ortlib = 1;
				break;
			case OPTION(nomp):       disableOpenMP = true; break;
			case OPTION(nox):        disableOmpix = true; break;
			case OPTION(nocodup):    disableCodeDuplication = true; break;
			case OPTION(nomakefile): makefile = false; break;
			case OPTION(nolineno):   cppLineNo = true; break;
			case OPTION(gdb):        usegdb = true; break;
			case OPTION(dbg):        showdbginfo = true; break;
			case OPTION(devinfo):    showdevinfo = true; break;
			case OPTION(devs):
				modulestring = val;
				break;
			default:
				if (parameter[0] == '-')              /* option */
					switch (parameter[1])
					{
						case 'c':
							mustlink = false;
							break;
						case 'l':
							d = append_arg(&link_args, argc, argv, 1);
							break;
						case 'L':
							d = append_arg(&link_args, argc, argv, 1);
							break;
						case 'I':
							d = append_arg(&prep_args, argc, argv, 1);
							break;
						case 'D':
							d = append_arg(&prep_args, argc, argv, 1);
							break;
						case 'U':
							d = append_arg(&prep_args, argc, argv, 1);
							break;
						case 'o':
							d = append_arg(&goutfile, argc, argv, 1);
							break;
						case 'k':
							keep = 1; d = 0;
							break;
						case 'K':
							keep = 2; d = 0;
							break;
						case 'v':
							verbose = true; d = 0;
							break;
						default:
							d = append_arg(&scc_flags, argc, argv, 0);
							if (strcmp(argv[0], "--version") == 0)
							{
								printf("%s\n", GIT_VERSION);
								_exit(0);
							}
					}
				else
				{
					d = append_arg(&files, argc, argv, 0);
					if (!fok(files.tail->val))
						ompicc_error(1, "file %s does not exist\n", files.tail->val);
				};
		}
		argc = argc - 1 - d;
		argv = argv + 1 + d;
	}

	if (!ortlib)
		strcpy(ortlibname, "default");    /* Default lib to use */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        COMPILING                                          *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int nkernels = 0;   /* # kernels in user code */


void ompicc_compile(char *fname)
{
	char *s, preflags[SLEN], noext[PATHSIZE], outfile[PATHSIZE];
	char cmd[LEN], strscc_flags[LEN], strgoutfile[LEN];
	int  res;

	if ((s = strrchr(fname, '.')) != NULL)
		if (strcmp(s, ".o") == 0) return;     /* skip .o files */

	strcpy(noext, fname);
	if ((s = strrchr(noext, '.')) != NULL) *s = 0; /* remove ext */
	sprintf(outfile, "%s_ompi.c", noext);

	/* Preprocess
	 */
	strarglist(preflags, &prep_args, SLEN);
#if defined(__SYSOS_cygwin) && defined(__SYSCOMPILER_cygwin)
	/* Hack for CYGWIN gcc */
	sprintf(cmd, "%s -U__CYGWIN__ -D__extension__=  -U__GNUC__ -D_OPENMP=200805"
	        " %s -I%s %s \"%s\" > \"%s.pc\"",
	        PREPROCESSOR, CPPFLAGS, IncludeDir, preflags, fname, noext);
#else
	sprintf(cmd, "%s -U__GNUC__ -D_OPENMP=200805 %s -I%s %s \"%s\" > \"%s.pc\"",
	        PREPROCESSOR, CPPFLAGS, IncludeDir, preflags, fname, noext);
#endif
	if (verbose)
		fprintf(stderr, "====> Preprocessing file (%s.c)\n  [ %s ]\n", noext, cmd);
	if ((res = system(cmd)) != 0)
		_exit(res);

	/* Transform
	 */
	sprintf(cmd, "%s%s%s \"%s.pc\" __ompi__%s%s%s%s%s%s%s %s > \"%s\"%s",
	        usegdb ? "gdb " : "", /* Run gdb instead of running _ompi directly */
	        RealOmpiName,
	        usegdb ? " -ex 'set args" : "", /* Pass the arguments */
	        noext,
	        strstr(CFLAGS, "OMPI_MAIN=LIB") ? " -nomain " : "",
	        strstr(CFLAGS, "OMPI_MEMMODEL=PROC") == NULL ? "" :
	        strstr(CFLAGS, "OMPI_MEMMODEL=THR") ? " -procs -threads " :
	        " -procs ",
	        disableOpenMP ? " -nomp " : "",
	        disableOmpix ? " -nox " : "",
	        disableCodeDuplication ? " -nocodedup " : "",
	        cppLineNo ? " -nolineno " : "",
	        showdbginfo ? " -showdbginfo" : "",
	        modules_to_ompi(modulestring),
	        outfile,
	        usegdb ? "'" : "");
	if (verbose)
		fprintf(stderr, "====> Transforming file (%s.c)\n  [ %s ]\n", noext, cmd);
	if ((res = system(cmd)) > 0)
		res = WEXITSTATUS(res);
	if (keep < 2)
	{
		sprintf(cmd, "%s.pc", noext);               /* remove preprocessed file */
		unlink(cmd);
	}
	if (res == 33)                                /* no pragma omp directives */
	{
		FILE *of = fopen(outfile, "w");
		if (of == NULL)
		{
			fprintf(stderr, "Cannot write to intermediate file.\n");
			_exit(1);
		}
		if (cppLineNo)
			fprintf(of, "# 1 \"%s.c\"\n", noext); /* Identify the original file */
		fclose(of);
		sprintf(cmd, "cat \"%s.c\" >> \"%s\"", noext, outfile);
		if (system(cmd) != 0)
		{
			unlink(outfile);
			_exit(res);
		}
	}
	else
	{
		if (res != 0)
		{
			if (!keep)
				unlink(outfile);
			_exit(res);
		}
		else   /* All OK - parse the 2nd line to get needed info */
		{
			FILE *of = fopen(outfile, "r");
			char ch = 0;

		  if (of != NULL)
			{
				for (ch = fgetc(of); ch!=EOF && ch!='\n'; ch = fgetc(of))
					;
				if (ch == '\n')
					0 || fscanf(of, "$OMPi__nfo:%d", &nkernels);
				fclose(of);
			}
		}
	}

	/* Compile the transformed file
	 */
	strarglist(strscc_flags, &scc_flags, LEN);
	sprintf(cmd, "%s \"%s\" -c %s -I%s %s %s",
	        COMPILER, outfile, CFLAGS, IncludeDir, preflags, strscc_flags);
	if (verbose)
		fprintf(stderr, "====> Compiling file (%s):\n  [ %s ]\n", outfile, cmd);
	res = system(cmd);
	if (!keep)
		unlink(outfile);
	if (res != 0)
		_exit(res);

	/* Settle the output file name
	 */
	strarglist(strgoutfile, &goutfile, LEN);
	strcpy(noext, get_basename(fname));
	if ((s = strrchr(noext, '.')) != NULL) * s = 0; /* remove ext */
	if (goutfile.head != NULL && !mustlink)
		strcpy(outfile, goutfile.head->val);
	else
		sprintf(outfile, "%s.o", noext);
	strcat(noext, "_ompi.o");
	if (verbose)
		fprintf(stderr, "====> Renaming file \"%s\" to \"%s\"\n",
		        noext, outfile);
	rename(noext, outfile);

	/* Kernels must be created within the directory of the source file.
	 * If all source files are in the same directory we are doing
	 * redundant copies here.
	 */
	if (nkernels && nmodules)
	{
		void kernel_makefiles(char *);

		if (verbose)
			fprintf(stderr, "====> Generating kernel makefiles for %d module(s)\n",
			                nmodules);
		kernel_makefiles(fname);  /* 99.9% get_basename() did not touch it */
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        LINKING                                            *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ompicc_link()
{
	arg_t *p;
	char cur_obj[PATHSIZE], tmp[PATHSIZE], cmd[LEN];
	char objects[LEN], *obj;
	char strsccargs[LEN], strlinkargs[LEN], strgoutfile[LEN], strprepargs[LEN];
	int len, is_tmp, ignore;
	char rm_obj[LEN];

	obj = objects;
	*obj = 0;
	strcpy(rm_obj, "rm -f ");
	for (p = files.head; p != NULL; p = p->next)
	{
		strcpy(cur_obj, p->val);
		is_tmp = 0;
		len = strlen(cur_obj);
		if (cur_obj[len - 1] == 'c')
			is_tmp = 1;
		if (is_tmp)
		{
			cur_obj[len - 2] = 0;
			strcpy(tmp, cur_obj);
			strcpy(cur_obj, get_basename(tmp));
			strcat(cur_obj, ".o");
			strcat(rm_obj, cur_obj);
			strcat(rm_obj, " ");
		}
		sprintf(obj, "\"%s\" ", cur_obj);
		obj += strlen(cur_obj) + 3;
	}

	strarglist(strsccargs,  &scc_flags, LEN);
	strarglist(strlinkargs, &link_args, LEN);
	strarglist(strgoutfile, &goutfile, LEN);
	strarglist(strprepargs, &prep_args, LEN);

	/* We have to include -lort 2 times due to circular dependencies
	 * with the threading libraries.
	 */
	sprintf(cmd,
	        "%s %s %s -I%s %s %s %s -L%s -L%s/%s -lort %s %s -lort",
	        COMPILER, objects, CFLAGS, IncludeDir, strprepargs, strsccargs,
	        strgoutfile, LibDir, LibDir, ortlibname, LDFLAGS, strlinkargs);
	if (verbose)
		fprintf(stderr, "====> Linking:\n  [ %s ]\n", cmd);
	if (system(cmd) != 0)
		fprintf(stderr, "Error: could not perform linking.\n");
	ignore = system(rm_obj);   /* Remove unnecessary files */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *                  Generate Makefile                        *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void replace_variable(char *line, char *variable, char *replace_with)
{
	char *p, tmp[SLEN];

	while (p = strstr(line, variable))
	{
		sprintf(tmp, "%s%s", replace_with, p + strlen(variable));
		//Should check if p - line + strlen(tmp) > sizeof(line)
		strcpy(p, tmp);
	}
}


//COPY PASTE FROM strarglist()
/* Prepare a string from an argument list; all args are quoted */
static void getargs(char *dest, arglist_t *l, int maxlen)
{
	arg_t *p;
	char  *c, *d = dest;

	for (*d = 0, p = l->head; p != NULL; p = p->next)
	{
		if ((d - dest) + quotedlen(p->val) + 6 >= maxlen)
			ompicc_error(1, "argument(s) too long; rebuild OMPi with larger LEN.\n");
		//    if (p->opt)
		//    {
		//      sprintf(dest, "-%c ", p->opt);
		//      dest += ( (p->opt == 'o') ? 3 : 2 );
		//    }
		//    *(dest++) = '"';
		for (c = p->val; *c != 0; c++)
		{
			if (*c == '"') *(dest++) = '\\';
			*(dest++) = *c;
		}
		//    *(dest++) = '"';
		if (p->next)
			*(dest++) = ' ';
	}
	*dest = 0;
}

void ompicc_makefile(char *compilerPath)
{
	char strfiles[LEN], strgoutfile[LEN];
	char *s, preflags[SLEN], noext[PATHSIZE], infile[PATHSIZE],
	     outfile[PATHSIZE], line[SLEN];
	FILE *fp, *infp;

	/* Check if the library has a template Makefile
	 */
	sprintf(infile, "%s/%s/MakefileTemplate", LibDir, ortlibname);
	if (!(infp = fopen(infile, "r")))
	{
		makefile = false;
		return;
	}

	getargs(strfiles, &files, LEN);

	//The file ompi will output
	if (goutfile.head && strlen(goutfile.head->val) != 0)
		sprintf(strgoutfile, "%s", goutfile.head->val);
	else
		sprintf(strgoutfile, "a.out");

	if (verbose)
		fprintf(stderr, "====> Outputing Makefile\n");
	if ((fp = fopen("Makefile", "w")) == NULL)
		fprintf(stderr, "Error: could not generate Makefile\n");
	else
	{
		fprintf(fp, "# Makefile generated by %s\n", PACKAGE_NAME);

		while (fgets(line, sizeof line, infp) != NULL)
		{

			replace_variable(line, "@OMPICC@", compilerPath);
			replace_variable(line, "@OMPI_INPUT@", strfiles);
			replace_variable(line, "@OMPI_OUTPUT@", strgoutfile);
			replace_variable(line, "@OMPI_ORTLIB@", ortlibname);

			fputs(line, fp);  /* write the line */
		}

		fclose(fp);
	}
}

void ompicc_makefile_compile()
{
	char cmd[LEN];
	int res;

	/* Run the compilation target
	 */
	//Should find the make executable
	sprintf(cmd, "make compile");

	if (verbose)
		fprintf(stderr, "====> Running target compile on generated Makefile\n");

	res = system(cmd);

	if (res != 0)
		_exit(res);
}

void ompicc_makefile_link()
{
	char cmd[LEN];
	int res;

	/* Run the link target
	 */
	//Should find the make executable
	sprintf(cmd, "make link");

	if (verbose)
		fprintf(stderr, "====> Running target link on generated Makefile\n");

	res = system(cmd);

	if (res != 0)
		_exit(res);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        CREATING & EXECUTING MAKEFILES FOR KERNELS         *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* TODO: Is it OK that makefiles discover all kernels by themselves?
 * Also, we need to see what @variables@ are universally needed.
 */


void km_substitute(char *var, int maxlen, str s, char *modulename)
{
	// FIXME: is goutfile set here?
	if (strncmp("@@OMPI_OUTPUT@@", var, maxlen) == 0)
		str_printf(s, "%s", (goutfile.head && strlen(goutfile.head->val) != 0) ?
		                    goutfile.head->val : "a.out");
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
	else
		str_printf(s, "%*.*s", maxlen, maxlen, var);
}


void km_substitute_vars(char *line, str s, char *modulename)
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
					km_substitute(line, p-line+1, s, modulename);
					p++;
				}
			}
		}
		line = p;
	}
}


int kernel_makefile_run(char *path, char *modulename)
{
	char cwd[PATHSIZE], cmd[SLEN];
	int  res;

	if (strcmp(path, ".") != 0)
	{
		if (getcwd(cwd, PATHSIZE) == NULL)
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
	sprintf(cmd, "make -f %s-makefile", modulename);
	res = system(cmd);

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
	if (res)
		return (1);
}


int kernel_makefile_create(char *path, char *modulename)
{
	static str s = NULL;
	char       line[SLEN], filepath[SLEN];
	FILE       *fpin, *fpout;

	/* Kernel makefile intallation path */
	snprintf(filepath, SLEN, "%s/devices/%s/MakeKernel.%s",
	         LibDir, modulename, modulename);
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
	  km_substitute_vars(line, s, modulename);
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
void kernel_makefiles(char *fname)
{
	int  i;
	char filepath[PATHSIZE], *s;

	if ((s = strrchr(fname, '/')) == NULL)
		strcpy(filepath, ".");
	else
	{
		strncpy(filepath, fname, s-fname);
		filepath[s-fname] = 0;
	}

	for (i = 0; i < nmodules; i++)
		kernel_makefile_create(filepath, modulenames[i]);

	if (!keep)       /* TODO: Remove kernels!! */
	{
		strcpy(filepath, fname);
	  if ((s = strrchr(filepath, '.')) != NULL)
			*s = 0; /* remove extension */
		else
			s = filepath + strlen(filepath);
		for (i = 0; i < nkernels; i++)
		{
			sprintf(s, "_d%02d.c", i);   // FIXME: maybe 3 digits?
			unlink(filepath);
		}
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        THE MAIN() PART                                    *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Get values from environmental variables if they exist,
 * otherwise grab the configuration values (from Makefile.am)
 */
void ompicc_get_envvars()
{
	char *t;

	if ((t = getenv("OMPI_CPP")) == NULL)
		strncpy(PREPROCESSOR, CPPcmd, 511);
	else
		strncpy(PREPROCESSOR, t, 511);

	if ((t = getenv("OMPI_CPPFLAGS")) == NULL)
		strncpy(CPPFLAGS, PreprocFlags, 511);
	else
		strncpy(CPPFLAGS, t, 511);

	if ((t = getenv("OMPI_CC")) == NULL)
		strncpy(COMPILER, CCcmd, 511);
	else
		strncpy(COMPILER, t, 511);

	if ((t = getenv("OMPI_CFLAGS")) == NULL)
		strncpy(CFLAGS, CompileFlags, 511);
	else
		strncpy(CFLAGS, t, 511);

	if ((t = getenv("OMPI_LDFLAGS")) == NULL)
		strncpy(LDFLAGS, LinkFlags, 511);
	else
		strncpy(LDFLAGS, t, 511);
}


/* Read the runtime library's configuration file
 */
void get_ort_flags()
{
	char confpath[PATHSIZE];
	FILE *fp;
	void setflag(char *, char *);

	snprintf(confpath, PATHSIZE - 1, "%s/%s/ortconf.%s", LibDir, ortlibname,
	         ortlibname);
	if ((fp = fopen(confpath, "r")) == NULL)
		ompicc_error(1, "library `%s' cannot be found\n  (%s is missing)\n",
		             ortlibname, confpath);
	keyval_read(fp, setflag);
	fclose(fp);
}


void setflag(char *key, char *value)
{
	if (strcmp(key, "ORTINFO") == 0 && strlen(value) + strlen(ORTINFO) < FLAGSIZE)
		strcat(*ORTINFO ? strcat(ORTINFO, " ") : ORTINFO, value);
	if (strcmp(key, "CPPFLAGS") == 0 && strlen(value) + strlen(CPPFLAGS) < FLAGSIZE)
		strcat(strcat(CPPFLAGS, " "), value);
	if (strcmp(key, "CFLAGS") == 0 && strlen(value) + strlen(CFLAGS) < FLAGSIZE)
		strcat(strcat(CFLAGS, " "), value);
	if (strcmp(key, "LDFLAGS") == 0 && strlen(value) + strlen(LDFLAGS) < FLAGSIZE)
		strcat(strcat(LDFLAGS, " "), value);
	if (strcmp(key, "CC") == 0 && strlen(COMPILER) < FLAGSIZE)
		strcpy(COMPILER, value);
	if (strcmp(key, "CPP") == 0 && strlen(PREPROCESSOR) < FLAGSIZE)
		strcpy(PREPROCESSOR, value);
}


void get_path(char *argv0, char *path)
{
	int i;

	memset(path, '\0', PATHSIZE);

	for (i = strlen(argv0); i >= 0; i--)
	{
		if (argv0[i] == '/')
		{
			strncpy(path, argv0, i + 1);
			path[i + 1] = '\0';
			break;
		}
	}

}

int main(int argc, char **argv)
{
	arg_t *p;
#if defined(HAVE_REALPATH)
	char  argv0[PATHSIZE];
	char  path[PATHSIZE];
	char  *res;

	strcpy(argv0, "");
	res = realpath(argv[0], argv0);
	if (res == NULL)
		strcpy(RealOmpiName, OmpiName);
	else
	{
		get_path(argv0, path);         /* path before ompicc */
		strcpy(RealOmpiName, path);
		strcat(RealOmpiName, OmpiName);
	}
#else
	strcpy(RealOmpiName, OmpiName);
#endif

#ifdef PORTABLE
	/* Code for making ompi portable in Linux. This wont work on other OSes.
	 */
	char buffer[PATHSIZE];
	ssize_t len = readlink("/proc/self/exe", buffer, PATHSIZE - 1);
	if (len == -1)
	{
		fprintf(stderr, "Error: couldn't retrieve library path using readlink.\n");
		exit(1);
	}
	else
		if (len == PATHSIZE - 1)
		{
			fprintf(stderr, "Error: path to %s too long.\n", PACKAGE_TARNAME);
			exit(1);
		}
		else
			buffer[len] = '\0';

	get_path(buffer, CompilerPath);

	if (strlen(CompilerPath) + 4 + strlen(PACKAGE_TARNAME) + 1 > PATHSIZE)
	{
		fprintf(stderr, "Error: path to %s too long.\n", PACKAGE_TARNAME);
		exit(1);
	}

	strcpy(LibDir, CompilerPath);
	strcat(LibDir, "../");
	strcpy(IncludeDir, LibDir);
	strcat(LibDir, "lib/");
	strcat(LibDir, PACKAGE_TARNAME);
	strcat(IncludeDir, "include/");
	strcat(IncludeDir, PACKAGE_TARNAME);
#endif

	ompicc_get_envvars();
	parse_args(argc, argv);
	get_ort_flags();
	modulestring_to_modulenames();

	if (argc == 1)
	{
		ompi_info();
		fprintf(stderr,
		        "\nUsage: %s [ompi options] [system compiler options] programfile(s)\n",
		        argv[0]);
		fprintf(stderr,
		        "\n"
		        "   OMPi options:\n"
		        "                  -k: keep intermediate file\n"
		        "                  -v: be verbose (show the actual steps)\n"
		        "        --ort=<name>: use a specific OMPi runtime library\n"
		        "              --nomp: ignore OpenMP constructs\n"
		        "               --nox: ignore OMPi extensions\n"
		        "    --devs=<devices>: target the given devices\n"
		        "           --devinfo: show information about configured devices\n"
		        "\n"
		        "Use environmental variables OMPI_CPP, OMPI_CC, OMPI_CPPFLAGS,\n"
		        "OMPI_CFLAGS, OMPI_LDFLAGS to have OMPi use a particular base\n"
		        "preprocessor and compiler, along with specific flags.\n");
		exit(0);
	}

	if (verbose)
	{
		ompi_info();
		fprintf(stderr, "----\n");
	}

	if (showdevinfo)
	{
		fprintf(stderr, "%d configured device module(s): %s\n\n",
		        nmodules, MODULES_CONFIG);
		modules_show_info();
		if (files.head == NULL)
			exit(0);
	}

	if (files.head == NULL)
	{
		fprintf(stderr, "No input file specified; "
		                "run %s with no arguments for help.\n", argv[0]);
		exit(0);
	}

	if (makefile)
		ompicc_makefile(argv[0]);

	/* If makefile exists
	 */
	if (makefile)
	{
		ompicc_makefile_compile();

		if (mustlink)
			ompicc_makefile_link();
	}
	else
	{
		for (p = files.head; p != NULL; p = p->next)
			ompicc_compile(p->val);
		if (mustlink)
			ompicc_link();
	}

	return (0);
}
