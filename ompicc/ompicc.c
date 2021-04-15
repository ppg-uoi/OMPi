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

/* OMPICC
 * A driver for the OMPi compiler.
 */

/*
 * May 2019:
 *   Code related to modules/devices and kernels moved to kernels.c.
 * July 2016:
 *   Small refactoring and more documentation.
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
#include "stddefs.h"
#include "git_version.h"
#include "str.h"
#include "keyval.h"
#include "ompicc.h"
#include "kernels.h"


/* The following two symbols must exist (i.e. these variables must be
 * defined, their type or value does not really matter) in order to be able
 * to load the mpinode shared library when running 'ompicc --devinfo'. */
int myeecb, ort;


/* This function uses system(3) to execute the shell command pointed by 'cmd'.
 * On success, the termination status of the child process which is retrieved
 * using the macros WIFEXITED and WEXITSTATUS described in waitpid(2) is returned.
 * If the child process could not be created or its status could not be retrieved:
 * 1) if 'exit_flag' is NULL -1 is returned, 2) if 'exit_flag' is not NULL
 * *exit_flag is set to -1. If system(3) is successful and 'exit_flag' is not NULL
 * *exit_flag is set to 0.
 */
int sysexec(char *cmd, int *exit_flag)
{
	int ret;

	/* If the child process could not be created or its status
	 * could not be retrieved set exit_flag to 1.
	 */
	if ((ret = system(cmd)) == -1)
	{
		if (exit_flag)
			return (*exit_flag = -1);
		_exit(-1);
	}
	exit_flag && (*exit_flag = 0);
	return (WIFEXITED(ret) ? WEXITSTATUS(ret) : ret);
}


/* How does ompicc decide on the flags to use when processing user programs?
 *
 * First, ompicc knows the flags used to build OMPi. These are hard-coded
 * at the time ompicc is built and include all the flags required to build
 * the last threading library. These are provided in Makefile.am and include
 * CompileFlags, LinkFlags, IncludeDir, LibDir etc. See comment below for
 * a full list.
 *
 * Second, ompicc reads the OMPI_XXX environmental variables. If any of
 * them exists, it overrides the corresponding hard-coded flags above.
 * In the end, the values of PREPROCESSOR, COMPILER, CPPFLAGS, CFLAGS,
 * LDFLAGS and ORTINFO variables below are either set to the hard-coded flags
 * or the corresponding environmental variable. These are the ones used to
 * build the user program.
 *
 * Finally, the user may pass additional flags when running ompicc. These
 * are stored in the variables named "user_xxx" (a bit below) and are
 * used as *additional* flags.
 *
 * Esoteric note:
 *   Because the hard-coded flags are the ones used to build
 *   the last threading library, there may be linker flags needed if the user
 *   utilizes another library with --ort.
 *   Should fix some day :-(
 */

/* Definitions (macros) provided extrernally (see Makefile.am).
 * These are hard-coded when making ompicc and are the defaults.
 * They can be user-overriden through the OMPI_XXX environmental variables.
 * The last two are overriden if a portable build of OMPi is in effect.
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
char PREPROCESSOR[PATHSIZE], COMPILER[PATHSIZE],
     CPPFLAGS[FLAGSIZE], CFLAGS[FLAGSIZE], LDFLAGS[FLAGSIZE],
     ORTINFO[FLAGSIZE];
char ortlibname[PATHSIZE],           /* The runtime library needed */
     RealOmpiName[PATHSIZE];

#ifdef PORTABLE_BUILD
	char InstallPath[PATHSIZE], LibDir[PATHSIZE], IncludeDir[PATHSIZE];
#endif

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
			snprintf(dest, maxlen, "-%c ", p->opt);
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
     cppLineNo = false;
bool mustlink = true;     /* Becomes 0 if -c parameter is specified */
bool makefile = true;     /* Becomes 0 if --nomakefile parameter is specified */
int  keep = 0;            /* Becomes 1/2 if -k/K parameter is specified */
bool verbose = false;
bool showdbginfo = false; /* Force the compiler to show some debugging info */
bool showdevinfo = false; /* Display info about configured devices */
bool longdevinfo = false; /* short by default */
bool usegdb = false;      /* If true _ompi will be run through gdb. For optimal
                           * results ompi must be configured with CFLAGS=-Og */
bool usecarstats = false;
bool reductionOld = false; /* Make _ompi produce newer style reduction code */
bool autoscope = false;    /* Enable autoscoping dfa */
int  taskoptlevel = -1;    /* default */
char *reqmodules = NULL;   /* Requested modules by the user */

arglist_t user_files     = { NULL, NULL };  /* Files to be compiled/linked */
arglist_t user_outfile   = { NULL, NULL };  /* Output, -o XXX */
arglist_t user_prep_args = { NULL, NULL };  /* Preprocessor args */
arglist_t user_link_args = { NULL, NULL };  /* Linker args given by the user */
arglist_t user_scc_flags = { NULL, NULL };  /* Remaining system compiler args */


/*
 * Options
 */


#define OPTNAME(opt)   "--" #opt
#define OPTNAME_V(opt) "V--" #opt "="
#define OPTION(opt)    OPT_##opt

typedef enum {
	OPTION(unknown) = -1, /* unknown option */
	OPTION(version) = 0, OPTION(options),     OPTION(envs),
	OPTION(ort),         OPTION(nomp),        OPTION(nox),
	OPTION(taskopt),     OPTION(nomakefile),  OPTION(nolineno),
	OPTION(gdb),         OPTION(dbg),         OPTION(devs),
	OPTION(devinfo),     OPTION(devvinfo),    OPTION(reduction),
	OPTION(autoscope),   OPTION(carstats),
	OPTION(lastoption)    /* dummy */
} option_t;

char *optnames[] = {
	OPTNAME(version),    OPTNAME(options),     OPTNAME(envs),
	OPTNAME_V(ort),      OPTNAME(nomp),        OPTNAME(nox),
	OPTNAME_V(taskopt),  OPTNAME(nomakefile),  OPTNAME(nolineno),
	OPTNAME(gdb),        OPTNAME(dbg),         OPTNAME_V(devs),
	OPTNAME(devinfo),    OPTNAME(devvinfo),    OPTNAME_V(reduction),
	OPTNAME(autoscope),  OPTNAME(carstats)
};

char *optinfo[] = {
	"", "    (show all OMPi options)", "    (show all available env vars)",
	"<eelib>",   "", "",
	"[size|speed|speed+]    (default: speed+)", "", "    (produces no # <line>)",
	"    (runs _ompi from within gdb)", "", "<deviceid,deviceid,...>",
	"     (show short devices info)", "    (show long devices info)", "[old|new]",
	"   (enable autoscoping analysis)", "    (turn on C.A.R.S. analysis)"
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


void showallopts()
{
	int i;

	printf("all available options\n---------------------\n");
	for (i = 0; i < OPTION(lastoption); i++)
		printf("  %s%s\n", optnames[i]+(optnames[i][0] == 'V' ? 1 : 0), optinfo[i]);
}


void helpOpenMPenv()
{
	int i, max = 0;
	char *ompenv[][2] = {
		{ "  OMP_DYNAMIC",           "boolean" },
		{ "  OMP_NESTED",            "boolean" },
		{ "  OMP_SCHEDULE",          "policy[,int]" },
		{ "",                        "     policy=static|dynamic|guided|auto" },
		{ "  OMP_STACKSIZE",         "int[C]     C=B|K|M|G (default:K)" },
		{ "  OMP_THREAD_LIMIT",      "int" },
		{ "  OMP_MAX_ACTIVE_LEVELS", "int" },
		{ "  OMP_WAIT_POLICY",       "active|passive" },
		{ "  OMP_NUM_THREADS",       "int[,int[,int ...]]" },
		{ "  OMP_PROC_BIND",         "true|false|<list of types>" },
		{ "",                        "     types=master|close|spread" },
		{ "  OMP_CANCELLATION",      "boolean" },
		{ "  OMP_DISPLAY_ENV",       "true|false|verbose" },
		{ "  OMP_PLACES",            "symbolic[(int)]|<list of places>" },
		{ "",                        "     symbolic=thread|cores|sockets" },
		{ "  OMP_DEFAULT_DEVICE",    "int" },
		{ "  OMP_MAX_TASK_PRIORITY", "int" },
		{ "  OMP_DISPLAY_AFFINITY",  "boolean" },
		{ "  OMP_AFFINITY_FORMAT",   "string" },
		{ "  OMP_TARGET_OFFLOAD",    "MANDATORY|DISABLED|DEFAULT" },
		{ "  OMPI_DYNAMIC_TASKQUEUESIZE", "boolean     (defualt:false)" },
		{ "  OMPI_STEAL_POLICY",     "FIFO|LIFO   (default:FIFO)" },
		{ "  OMPI_PAR2TASK_POLICY",  "true|false|auto  (default:auto)" },
		{ "  OMPI_HOSTTARGET_SHARE", "boolean     (default:false)" },
		{ NULL, NULL }
	};
	
	printf("all runtime env. variables\n--------------------------\n");
	for (i = 0; ompenv[i][0] != NULL; i++)
		if (strlen(ompenv[i][0]) > max)
			max = strlen(ompenv[i][0]);
	for (i = 0; ompenv[i][0] != NULL; i++)
		printf("%-*s  %s\n", max, ompenv[i][0], ompenv[i][1]);
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
			case OPTION(version):
				printf("%s\n", GIT_VERSION);
				_exit(0);
				break;
			case OPTION(options):
				showallopts();
				_exit(0);
				break;
			case OPTION(envs):
				helpOpenMPenv();
				_exit(0);
				break;
			case OPTION(ort):
				strncpy(ortlibname, val, 511);
				ortlib = 1;
				break;
			case OPTION(nomp):       disableOpenMP = true; break;
			case OPTION(nox):        disableOmpix = true; break;
			case OPTION(nomakefile): makefile = false; break;
			case OPTION(nolineno):   cppLineNo = true; break;
			case OPTION(gdb):        usegdb = true; break;
			case OPTION(dbg):        showdbginfo = true; break;
			case OPTION(carstats):   usecarstats = true; break;
			case OPTION(devinfo):    showdevinfo = true; longdevinfo = false; break;
			case OPTION(devvinfo):   showdevinfo = true; longdevinfo = true; break;
			case OPTION(autoscope):  autoscope = true; break;
			case OPTION(taskopt):
				if (strcmp(val, "size") == 0)
					taskoptlevel = 0;
				else
					if (strcmp(val, "speed") == 0)
						taskoptlevel = 1;
					else
						if (strcmp(val, "speed+") == 0)
							taskoptlevel = 2;
				break;
			case OPTION(devs):
				reqmodules = val;
				break;
			case OPTION(reduction):
				if (strcmp(val, "old") == 0)
					reductionOld = true;
				else
					if (strcmp(val, "new"))
						ompicc_error(1,"unknown reduction request (try 'new' or 'old').\n");
				break;
			default:
				if (parameter[0] == '-')              /* option */
					switch (parameter[1])
					{
						case 'c':
							mustlink = false;
							break;
						case 'l':
							d = append_arg(&user_link_args, argc, argv, 1);
							break;
						case 'L':
							d = append_arg(&user_link_args, argc, argv, 1);
							break;
						case 'I':
							d = append_arg(&user_prep_args, argc, argv, 1);
							break;
						case 'D':
							d = append_arg(&user_prep_args, argc, argv, 1);
							break;
						case 'U':
							d = append_arg(&user_prep_args, argc, argv, 1);
							break;
						case 'o':
							d = append_arg(&user_outfile, argc, argv, 1);
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
							d = append_arg(&user_scc_flags, argc, argv, 0);
							break;
					}
				else
				{
					d = append_arg(&user_files, argc, argv, 0);
					if (!fok(user_files.tail->val))
						ompicc_error(1, "file %s does not exist\n", user_files.tail->val);
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


void ompicc_compile(char *fname)
{
	char *s, *compfmt, preflags[SLEN], noext[PATHSIZE], outfile[PATHSIZE];
	char cmd[LEN], strscc_flags[LEN], strgoutfile[LEN];
	int  res, eflag;
	int  nkernels = 0;  /* # kernels in user code */

	if ((s = strrchr(fname, '.')) != NULL)
		if (strcmp(s, ".o") == 0) return;     /* skip .o files */

	strcpy(noext, fname);
	if ((s = strrchr(noext, '.')) != NULL) *s = 0; /* remove ext */
	snprintf(outfile, PATHSIZE, "%s_ompi.c", noext);

	/* Preprocess
	 */
	strarglist(preflags, &user_prep_args, SLEN);
#if defined(__SYSOS_cygwin) && defined(__SYSCOMPILER_cygwin)
	/* Hack for CYGWIN gcc */
	snprintf(cmd, LEN, "%s -U__CYGWIN__ -D__extension__=  -U__GNUC__ "
	        " %s -I%s %s \"%s\" > \"%s.pc\"",
	        PREPROCESSOR, CPPFLAGS, IncludeDir, preflags, fname, noext);
#else
	snprintf(cmd, LEN, "%s -U__GNUC__ %s -I%s %s \"%s\" > \"%s.pc\"",
	        PREPROCESSOR, CPPFLAGS, IncludeDir, preflags, fname, noext);
#endif
	if (verbose)
		fprintf(stderr, "====> Preprocessing file (%s.c)\n  [ %s ]\n", noext, cmd);
	if ((res = sysexec(cmd, NULL)) != 0)
		_exit(res);

	/* Transform
	 */
#ifdef PORTABLE_BUILD
	compfmt = "%s%s%s \"%s.pc\" __ompi__ \"%s\"%s%s%s%s%s%s%s%s%s%s %s > \"%s\"%s";
#else
	compfmt = "%s%s%s \"%s.pc\" __ompi__%s%s%s%s%s%s%s%s%s%s %s > \"%s\"%s";
#endif
	snprintf(cmd, LEN, compfmt,
	        usegdb ? "gdb " : "", /* Run gdb instead of running _ompi directly */
	        RealOmpiName,
	        usegdb ? " -ex 'set args" : "", /* Pass the arguments */
	        noext,
#ifdef PORTABLE_BUILD
	        InstallPath,             /* Use as the first argument */
#endif
	        strstr(CFLAGS, "OMPI_MAIN=LIB") ? " -nomain " : "",
	        strstr(CFLAGS, "OMPI_MEMMODEL=PROC") == NULL ? "" :
	          strstr(CFLAGS, "OMPI_MEMMODEL=THR") ? " -procs -threads " :
	            " -procs ",
	        disableOpenMP ? " -nomp " : "",
	        disableOmpix ? " -nox " : "",
	        taskoptlevel == 0 ? " -taskopt0 " : 
	          taskoptlevel == 2 ? " -taskopt2 " : " -taskopt1 ",
	        cppLineNo ? " -nolineno " : "",
	        showdbginfo ? " -showdbginfo " : "",
	        usecarstats ? " -drivecar " : "",
	        reductionOld ? " -oldred " : "",
	        autoscope ? " -autoscope" : "",
	        modules_argfor_ompi(),
	        outfile,
	        usegdb ? "'" : "");
	if (verbose)
		fprintf(stderr, "====> Transforming file (%s.c)\n  [ %s ]\n", noext, cmd);
	res = sysexec(cmd, NULL);
	if (keep < 2)
	{
		snprintf(cmd, LEN, "%s.pc", noext);               /* remove preprocessed file */
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
		snprintf(cmd, LEN, "cat \"%s.c\" >> \"%s\"", noext, outfile);
		if (sysexec(cmd, &eflag))
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
					fscanf(of, "$OMPi__nfo:%d", &nkernels);
				fclose(of);
			}
		}
	}

	/* Compile the transformed file
	 */
	strarglist(strscc_flags, &user_scc_flags, LEN);
	snprintf(cmd, LEN, "%s \"%s\" -c %s -I%s %s %s",
	        COMPILER, outfile, CFLAGS, IncludeDir, preflags, strscc_flags);
	if (verbose)
		fprintf(stderr, "====> Compiling file (%s):\n  [ %s ]\n", outfile, cmd);
	res = sysexec(cmd, &eflag);
	if (!keep)
		unlink(outfile);
	if (eflag || res)
		_exit(res);

	/* Settle the output file name
	 */
	strarglist(strgoutfile, &user_outfile, LEN);
	strcpy(noext, get_basename(fname));
	if ((s = strrchr(noext, '.')) != NULL) * s = 0; /* remove ext */
	if (user_outfile.head != NULL && !mustlink)
		strcpy(outfile, user_outfile.head->val);
	else
		snprintf(outfile, PATHSIZE, "%s.o", noext);
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
		if (verbose)
			fprintf(stderr, "====> Generating kernel makefiles for %d module(s)\n",
			                nmodules);
		kernel_makefiles(fname, nkernels); /* 99.9% get_basename didn't touch it */
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
	int len, is_tmp, eflag;
	char rm_obj[LEN];

	obj = objects;
	*obj = 0;
	strcpy(rm_obj, "rm -f ");
	for (p = user_files.head; p != NULL; p = p->next)
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
		snprintf(obj, LEN, "\"%s\" ", cur_obj);
		obj += strlen(cur_obj) + 3;
	}

	strarglist(strsccargs,  &user_scc_flags, LEN);
	strarglist(strlinkargs, &user_link_args, LEN);
	strarglist(strgoutfile, &user_outfile, LEN);
	strarglist(strprepargs, &user_prep_args, LEN);

	/* We have to include -lort 2 times due to circular dependencies
	 * with the threading libraries.
	 */
	snprintf(cmd, LEN,
	        "%s %s %s -I%s %s %s %s -L%s -L%s/%s -lort %s %s -lort",
	        COMPILER, objects, CFLAGS, IncludeDir, strprepargs, strsccargs,
	        strgoutfile, LibDir, LibDir, ortlibname, LDFLAGS, strlinkargs);
	if (verbose)
		fprintf(stderr, "====> Linking:\n  [ %s ]\n", cmd);
	if (sysexec(cmd, &eflag))
		fprintf(stderr, "Error: could not perform linking.\n");
	sysexec(rm_obj, &eflag);   /* Remove unnecessary files */
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
		snprintf(tmp, SLEN, "%s%s", replace_with, p + strlen(variable));
		//Should check if p - line + strlen(tmp) > sizeof(line)
		strcpy(p, tmp);
	}
}


/* Prepare a string from an argument list; all args are quoted;
 * copied from strarglilst()
 */
static void getargs(char *dest, arglist_t *l, int maxlen)
{
	arg_t *p;
	char  *c, *d = dest;

	for (*d = 0, p = l->head; p != NULL; p = p->next)
	{
		if ((d - dest) + quotedlen(p->val) + 6 >= maxlen)
			ompicc_error(1, "argument(s) too long; rebuild OMPi with larger LEN.\n");
		for (c = p->val; *c != 0; c++)
		{
			if (*c == '"') *(dest++) = '\\';
			*(dest++) = *c;
		}
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
	snprintf(infile, PATHSIZE, "%s/%s/MakefileTemplate", LibDir, ortlibname);
	if (!(infp = fopen(infile, "r")))
	{
		makefile = false;
		return;
	}

	getargs(strfiles, &user_files, LEN);

	//The file ompi will output
	if (user_outfile.head && strlen(user_outfile.head->val) != 0)
		snprintf(strgoutfile, LEN, "%s", user_outfile.head->val);
	else
		snprintf(strgoutfile, LEN, "a.out");

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

	/* Run the compile target */
	snprintf(cmd, LEN, "make compile");  /* make should be on the user PATH */

	if (verbose)
		fprintf(stderr, "====> Running target compile on generated Makefile\n");

	if ((res = sysexec(cmd, NULL)) != 0)
		_exit(res);
}


void ompicc_makefile_link()
{
	char cmd[LEN];
	int res;

	/* Run the link target */
	snprintf(cmd, LEN, "make link");  /* make should be on the user PATH */

	if (verbose)
		fprintf(stderr, "====> Running target link on generated Makefile\n");

	if ((res = sysexec(cmd, NULL)) != 0)
		_exit(res);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                           *
 *        THE MAIN() PART                                    *
 *                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Get values from environmental variables if they exist,
 * otherwise grab the default configuration values (as hard-coded
 * from Makefile.am)
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
	void setflag(char *, char *, void *);

	snprintf(confpath, PATHSIZE - 1, "%s/%s/ortconf.%s", LibDir, ortlibname,
	         ortlibname);
	if ((fp = fopen(confpath, "r")) == NULL)
		ompicc_error(1, "library `%s' cannot be found\n  (%s is missing)\n",
		             ortlibname, confpath);
	keyval_read(fp, setflag, NULL);
	fclose(fp);
}


void setflag(char *key, char *value, void *ignore)
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

#ifdef PORTABLE_BUILD
	/* Code for making ompi portable in Linux. This wont work on other OSes.
	 */
	char buffer[PATHSIZE];
	ssize_t len = readlink("/proc/self/exe", buffer, PATHSIZE - 1);
	if (len == -1)
		ompicc_error(1, "couldn't retrieve installation path using readlink.\n");
	else
		if (len == PATHSIZE - 1)
			ompicc_error(1, "path to %s too long.\n", PACKAGE_TARNAME);
		else
			buffer[len] = '\0';

	/* Get the path and also get rid of the trailing "bin/" */
	get_path(buffer, InstallPath);
	if (strcmp(InstallPath + strlen(InstallPath) - 4, "bin/"))
		ompicc_error(1, "invalid installation path for a portable build.\n");
	InstallPath[strlen(InstallPath) - 4] = 0;
	
	if (strlen(InstallPath) + 8 + strlen(PACKAGE_TARNAME) + 1 > PATHSIZE)
		ompicc_error(1, "path to %s too long.\n", PACKAGE_TARNAME);

	strcpy(LibDir, InstallPath);
	strcat(LibDir, "lib/");
	strcat(LibDir, PACKAGE_TARNAME);
	strcpy(IncludeDir, InstallPath);
	strcat(IncludeDir, "include/");
	strcat(IncludeDir, PACKAGE_TARNAME);
#endif

	ompicc_get_envvars();
	parse_args(argc, argv);
	get_ort_flags();
	modules_employ(reqmodules);

	if (argc == 1)
	{
		ompi_info();
		fprintf(stderr,
		        "\nUsage: %s [ompi options] [system compiler options] programfile(s)\n",
		        argv[0]);
		fprintf(stderr,
		        "\n"
		        "   Useful OMPi options:\n"
		        "                  -k: keep intermediate file\n"
		        "                  -v: be verbose (show the actual steps)\n"
		        "        --ort=<name>: use a specific OMPi runtime library\n"
		        "    --devs=<devices>: target the given devices\n"
		        "           --devinfo: show short info about configured devices\n"
		        "           --options: show all OMPi options\n"
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
		modules_show_info(longdevinfo);
		if (user_files.head == NULL)
			exit(0);
	}

	if (user_files.head == NULL)
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
		for (p = user_files.head; p != NULL; p = p->next)
			ompicc_compile(p->val);
		if (mustlink)
			ompicc_link();
	}

	return (0);
}
