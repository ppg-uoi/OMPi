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
 * 2014/01/07
 *   integrated ptester
 * 2009/05/10
 *   fixed '#include <string.h>' bug
 */

/* ompi.c -- the starting point */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "ompi.h"
#include "ast.h"
#include "ast_free.h"
#include "ast_gv.h"
#include "ast_csource.h"
#include "ast_xform.h"
#include "ast_show.h"
#include "x_types.h"
#include "x_target.h"
#include "x_task.h"
#include "cfg.h"
#include "callgraph.h"
#include "str.h"
#include "config.h"
#include "git_version.h"


static aststmt ast;          /* The AST we use as our original */
symtab         stab;         /* Our symbol table */
bool           testingmode;  /* For internal testing only */
char           *filename;    /* The file we parse */
char           advert[256];  /* A string added to the top of generated files */

#ifdef PORTABLE_BUILD
static char *InstallPath; 
#endif

/* This is for taking care of main() in the parsed code;
 * OMPi generates its own main() and replaces the original one.
 */
char *MAIN_NEWNAME   = "__original_main";  /* Rename the original main */
bool hasMainfunc     = false; /* true if main() function is defined in the file */
bool needMemcpy      = false; /* true if generated code includes memcpy()s */
bool needMemset      = false; /* true if generated code includes memset()s */
bool needLimits      = false; /* true if need limits.h constants (min/max) */
bool needFloat       = false; /* true if need float.h constants (min/max) */
bool nonewmain       = false; /* If true, we won't output a new "main()" */
bool processmode     = false; /* If true, turn on process mode */
bool threadmode      = false; /* Will become true by default */
bool enableOpenMP    = true;  /* If false, ignore OpenMP constructs */
bool enableOmpix     = true;  /* Enable OMPi-extensions */
bool cppLineNo       = true;  /* Output precompiler line directives (# n) */
bool showdbginfo     = false; /* Allow assorted debugging info on stderr */
bool analyzeKernels  = false; /* Force kernel analysis for our C.A.R. system */
bool oldReduction    = false; /* Force newer reduction code */
bool enableAutoscope = false; /* No auto scoping analysis */
int  mainfuncRettype = 0;     /* 0 = int, 1 = void */
taskopt_e taskoptLevel = OPT_FAST;  /* Task code optimizations for speed */

static str modstr;        /* String containing all used module names */
static int usedmods;      /* # used modules */


void append_new_main()
{
	if (!hasMainfunc) return;
	
#ifdef PORTABLE_BUILD
	/* Pass the installation path before the first module name */
	str_insert(modstr, 0, "\"");
	str_insert(modstr, 0, InstallPath);
	str_insert(modstr, 0, ", \"");
#else
	if (usedmods == 0)
		modstr = Str(", \"dummy\"");
#endif
	
	A_str_truncate();
	str_printf(strA(),
	           "/* OMPi-generated main() */\n"
	           "int %s(int argc, char **argv)\n{\n",
	           nonewmain ? "__ompi_main" : "main");

	if (mainfuncRettype == 0)
	{
		if (enableOpenMP || testingmode || processmode)
			str_printf(strA(),
			           "  int _xval = 0;\n\n"
			           "  ort_initialize(&argc, &argv, 0, %d%s);\n"
			           "  _xval = (int) %s(argc, argv);\n"
			           "  ort_finalize(_xval);\n"
			           "  return (_xval);\n",
			           usedmods, str_string(modstr), MAIN_NEWNAME);
		else
			str_printf(strA(),
			           "  int _xval = 0;\n\n"
			           "  _xval = (int) %s(argc, argv);\n"
			           "  return (_xval);\n", MAIN_NEWNAME);
	}
	else
	{
		if (enableOpenMP || testingmode || processmode)
			str_printf(strA(),
			           "  ort_initialize(&argc, &argv, 0, %d%s);\n"
			           "  %s(argc, argv);\n"
			           "  ort_finalize(0);\n"
			           "  return (0);\n",
			           usedmods, str_string(modstr), MAIN_NEWNAME);
		else
			str_printf(strA(),
			           "  %s(argc, argv);\n"
			           "  return (0);\n", MAIN_NEWNAME);
	}
	str_printf(strA(), "}\n");

	ast = BlockList(ast, Verbatim(strdup(A_str_string())));
	ast->u.next->parent = ast;   /* Parentize correctly */
	ast->body->parent = ast;
}


/*
 * Options
 */


#define OPTNAME(opt)   "-" #opt
#define OPTNAME_L(opt) "--" #opt "="
#define OPTION(opt)    OPT_##opt

typedef enum {
	OPTION(unknown) = -1, /* unknown option */
	OPTION(nomain) = 0,   OPTION(procs),       OPTION(threads),
	OPTION(nomp),         OPTION(nox),         OPTION(oldred), 
	OPTION(nolineno),     OPTION(showdbginfo), OPTION(usemod),
	OPTION(taskopt0),     OPTION(taskopt1),    OPTION(taskopt2),
	OPTION(drivecar),     OPTION(autoscope),
	OPTION(lastoption)    /* dummy */
} option_t;

char *optnames[] = {
	OPTNAME(nomain),      OPTNAME(procs),       OPTNAME(threads),
	OPTNAME(nomp),        OPTNAME(nox),         OPTNAME(oldred),
	OPTNAME(nolineno),    OPTNAME(showdbginfo), OPTNAME_L(usemod),
	OPTNAME(taskopt0),    OPTNAME(taskopt1),    OPTNAME(taskopt2),
	OPTNAME(drivecar),    OPTNAME(autoscope)
};


void showopts()
{
	int i;

	for (i = 0; i < OPTION(lastoption); i++)
		fprintf(stderr, "\t%s\n", optnames[i]);
}


option_t optid(char *arg)
{
	int i;

	for (i = 0; i < OPTION(lastoption); i++)
		if (optnames[i][1] == '-')     /* Long option */
		{
			if (strncmp(optnames[i], arg, strlen(optnames[i])) == 0)
				return ((option_t) i);
		}
		else
			if (strcmp(optnames[i], arg) == 0)
				return ((option_t) i);
	return OPTION(unknown);
}


int getopts(int argc, char **argv)
{
	int i = 0;

#ifdef PORTABLE_BUILD
	/* In portable builds the 1st argument is the installation path */
	InstallPath = argv[0];
	argc--;
	argv++;
	fprintf(stderr, "InstallPath=%s, argc=%d, *argv=%s\n", InstallPath, argc, *argv);
#endif
	
	modstr = Strnew();
	for (i = 0; i < argc; i++)
		switch ( optid(argv[i]) )
		{
			case OPTION(nomain):      nonewmain = true; break;
			case OPTION(procs):       processmode = true; 
			                          /* Force old reduction style since newer one
			                             passes local pointers to the master; array
			                             reductions are, hence, precluded. */
			                          oldReduction = true;
			                          break;
			case OPTION(threads):     threadmode = true; break;
			case OPTION(nomp):        enableOpenMP = false; break;
			case OPTION(nox):         enableOmpix = false; break;
			case OPTION(taskopt0):    taskoptLevel = OPT_NONE; break;
			case OPTION(taskopt1):    taskoptLevel = OPT_FAST; break;
			case OPTION(taskopt2):    taskoptLevel = OPT_ULTRAFAST; break;
			case OPTION(nolineno):    cppLineNo = false; break;
			case OPTION(showdbginfo): showdbginfo = true; break;
			case OPTION(drivecar):    analyzeKernels = true; break;
			case OPTION(oldred):      oldReduction = true; break;
			case OPTION(autoscope):   enableAutoscope = true; break;
			case OPTION(usemod):
				str_printf(modstr, ", \"%s\"", argv[i]+strlen(OPTNAME_L(usemod)) );
				usedmods++;
			  break;
			default:
				return (1);
		};
	return (0);
}


static void reveal_inside_info()
{
		fprintf(stderr, ">> package = %s\n", PACKAGE_STRING);
#ifdef PORTABLE_BUILD
		fprintf(stderr, ">> this is a portable build\n");
#endif
#ifdef MODULES_CONFIG
		fprintf(stderr, ">> configured modules: %s\n", MODULES_CONFIG);
#endif
		fprintf(stderr, ">> sizes of int, long, ptr: %d, %d, %d\n", 
			SIZEOF_INT, SIZEOF_LONG, SIZEOF_CHAR_P);
#ifdef TLS_KEYWORD
		fprintf(stderr, ">> thread-local storage (TLS) is available\n");
#else
		fprintf(stderr, ">> thread-local storage (TLS) is not available.\n");
#endif
		fprintf(stderr, ">> _ompi __ompi__ <options>:\n");
		showopts();
}


static
int privatemode(int argc, char *argv[])
{
	enum { GV_STMT = 0, GV_EXPR, GV_PROG, CODE_STMT, CODE_EXPR, CFG_SHOW,
		     CFG_SHOW_VERBOSE, CG_SHOW, SHOWINTS, PRVCMD_LAST /* dummy */
	} cmd;
	struct { char *name, *info; } privcmds[] = {
		{ "gv_stmt",   "print statement node parse tree" },
		{ "gv_expr",   "print expression node parse tree" },
		{ "gv_prog",   "print the AST of a whole program" },
		{ "code_stmt", "print C code for the tree" },
		{ "code_expr", "print C code for the tree" },
		{ "cfg",       "print the CFG for the tree" },
		{ "cfgv",      "print the CFG for the tree with full details" },
		{ "cg",        "print the call graph of a whole program" },
		{ "showints",  "show various internal infos" },
	};

	char    tmp[256];
	aststmt ast;
	astexpr expr;

	MAIN_NEWNAME = "main";    /* Do NOT change it!! */

	if (argc < 3)
	{
	PRIVATEMODE_FAILURE:
		fprintf(stderr, "Usage: %s __internal__ <command> [ file ]\n"
		        "where <command> is one of the following:\n", argv[0]);
		for (cmd = 0; cmd < PRVCMD_LAST; cmd++)
			fprintf(stderr, "%11s   %s\n", privcmds[cmd].name, privcmds[cmd].info);
		fprintf(stderr, "[ If gv output is produced, use 'dot -Tpng' "
		                "to make an image out of it. ]\n");
		return (1);
	}
	filename = (argc > 3) ? argv[3] : NULL;
	for (cmd = 0; cmd < PRVCMD_LAST; cmd++)
		if (strcmp(argv[2], privcmds[cmd].name) == 0)
			break;
	if (cmd == PRVCMD_LAST)
		goto PRIVATEMODE_FAILURE;
	if (cmd == SHOWINTS)
	{
		reveal_inside_info();
		return (1);
	}

	stab = Symtab();                            /* Create the symbol table */
	symtab_put(stab, Symbol("__builtin_va_list"), TYPENAME);
	symtab_put(stab, Symbol("__extension__"), TYPENAME);
	symtab_put(stab, Symbol("__func__"), IDNAME);

	/* Read the file into a string */
	if (1)
	{
		char s[1024];
		FILE *fp;

		if (filename)
		{
			if ((fp = fopen(filename, "r")) == NULL)
				goto PRIVATEMODE_FAILURE;
			A_str_truncate();
			for (; fgets(s, 1024, fp);)
				str_printf(strA(), "%s", s);
			fclose(fp);
		}
		else
		{
			fprintf(stderr, "Enter %s:\n", (cmd == GV_EXPR || cmd == CODE_EXPR) ?
			                "expression" : "statement");
			if (!fgets(s, 1024, stdin))
				exit_error(1, "fgets failed\n");
			str_printf(strA(), "%s", s);
		}
	}

	switch (cmd)
	{
		case GV_STMT:
			ast = parse_blocklist_string(A_str_string());
			if (ast != NULL)
				ast_stmt_gviz_doc(ast, A_str_string());
			break;
		case GV_EXPR:
			expr = parse_expression_string(A_str_string());
			if (expr != NULL)
				ast_expr_gviz_doc(expr, A_str_string());
			break;
		case GV_PROG:
			ast = parse_transunit_string(A_str_string());
			if (ast != NULL)
				ast_stmt_gviz_doc(ast, A_str_string());
			break;
		case CODE_STMT:
			ast = parse_blocklist_string(A_str_string());
			if (ast != NULL)
			{
				ast_stmt_csource(ast);
				printf("\n");
			}
			break;
		case CODE_EXPR:
			expr = parse_expression_string(A_str_string());
			if (expr != NULL)
			{
				ast_expr_csource(expr);
				printf("\n");
			}
			break;
		case CFG_SHOW:
		case CFG_SHOW_VERBOSE:
			ast = parse_blocklist_string(A_str_string());
			if (ast != NULL)
				cfg_test(ast, (cmd == CFG_SHOW) ? 0 : 1);
			break;
		case CG_SHOW:
			ast = parse_transunit_string(A_str_string());
			if (ast != NULL)
				cg_call_graph_test(ast);
			break;
	}
	return (0);
}


#include "ort.defs"


int main(int argc, char *argv[])
{
	time_t  now;
	int     r, includes_omph;
	aststmt p;
	        /* flag if def'ed in user code */
	bool    knowMemcpy, knowMemset, knowSize_t;

	/*
	 * 1. Preparations
	 */

	if (argc < 2)
	{
	OMPI_FAILURE:
		fprintf(stderr, "** %s should not be run directly; use %scc instead\n",
		        argv[0], argv[0]);
		return (20);
	}

	if (strcmp(argv[1], "__internal__") == 0)
		return (privatemode(argc, argv));
	if (argc < 3)
		goto OMPI_FAILURE;

	if (strcmp(argv[2], "__ompi__") != 0)
	{
		if (strcmp(argv[2], "__intest__") == 0)
			testingmode = 1;
		else
			goto OMPI_FAILURE;
	}
	
	if (argc > 3 && getopts(argc - 3, argv + 3))
		goto OMPI_FAILURE;
		
	filename = argv[1];
	if (!processmode) threadmode = true;  /* By default */

	stab = Symtab();                            /* Create the symbol table */

	/* Take care of GCC */
	symtab_put(stab, Symbol("__builtin_va_list"), TYPENAME);
	symtab_put(stab, Symbol("__extension__"), TYPENAME);

	/* This is a reserved identifier in C99; it is actually supposed to be
	 * "declared" at the top of each function; we simply insert it
	 * @ global scope so it is visible everywhere.
	 */
	symtab_put(stab, Symbol("__func__"), IDNAME);

	/*
	 * 2. Parse & get the AST
	 */

	ast = parse_file(filename, &r);
	if (r) return (r);
	if (ast == NULL)        /* Cannot open file */
	{
		fprintf(stderr, "Error opening file %s for reading!\n", filename);
		return (30);
	}

	if ((!__has_omp || !enableOpenMP) && (!__has_ompix || !enableOmpix)
	    && !hasMainfunc && !testingmode && !processmode)
		return (33);          /* Leave it as is */

	/*
	 * 3. Transform & output
	 */

	/* The parser has left the symbol table at global scope; we must drain it */
	includes_omph = (symtab_get(stab, Symbol("omp_lock_t"), TYPENAME) != NULL);
	knowMemcpy = (symtab_get(stab, Symbol("memcpy"), FUNCNAME) != NULL);
	knowMemset = (symtab_get(stab, Symbol("memset"), FUNCNAME) != NULL);
	knowSize_t = (symtab_get(stab, Symbol("size_t"), TYPENAME) != NULL);
	symtab_drain(stab);

	if (hasMainfunc && (enableOpenMP || testingmode || processmode))
	{
		/* Need to declare the ort init/finalize functions */
		p = parse_and_declare_blocklist_string(rtlib_onoff);
		assert(p != NULL);
		if (cppLineNo)
			ast = BlockList(verbit("# 1 \"ort.onoff.defs\""), BlockList(p, ast));
		else
			ast = BlockList(p, ast);
	}

	if ((__has_omp  && enableOpenMP) ||
	    (__has_ompix && enableOmpix) || testingmode || processmode)
	{
		aststmt prepend = NULL;

		/* Runtime library definitions */
		if (__has_omp && enableOpenMP)
		{
			/* If <omp.h> was not included, then we must define a few things */
			if (includes_omph)
				p = NULL;
			else
			{
				p = parse_and_declare_blocklist_string(
				      "#pragma omp declare target\n"     /* Necessary since 4.0 */
				      "typedef void *omp_nest_lock_t;"   /* The only stuff we needed */
				      "typedef void *omp_lock_t; "       /* from <omp.h> */
				      "typedef enum omp_sched_t { omp_sched_static = 1,"
				      "omp_sched_dynamic = 2,omp_sched_guided = 3,omp_sched_auto = 4"
				      " } omp_sched_t;"
				      "\n#pragma omp end declare target\n"
				      "int omp_in_parallel(void); "
				      "int omp_get_thread_num(void); "
				      "int omp_get_num_threads(void); "
				      "int omp_in_final(void); "
				      "void *ort_memalloc(int size); "
				      "void ort_memfree(void *ptr); "
				    );
				assert(p != NULL);
				if (cppLineNo) p = BlockList(verbit("# 1 \"omp.mindefs\""), p);
			}

			/* Notice here that any types in omp.h will be defined *after* this */
			prepend = parse_and_declare_blocklist_string(rtlib_defs);
			assert(prepend != NULL);
			if (cppLineNo)
				prepend = BlockList(verbit("# 1 \"ort.defs\""), prepend);
			if (p)
				prepend = BlockList(p, prepend);
			if (__has_affinitysched)
				prepend = BlockList(prepend,
				                    parse_and_declare_blocklist_string(
				                      "int ort_affine_iteration(int *);"
				                    ));

			ast = BlockList(prepend, ast);
		}

		if (cppLineNo) ast = BlockList(verbit("# 1 \"%s\"", filename), ast);
		ast = BlockList(ast, verbit("\n"));    /* Dummy node @ bottom */
		ast->file = Symbol(filename);

		ast_parentize(ast);    /* Parentize */
		symtab_drain(stab);    /* Empty it; new globals will be traversed again */

		cg_find_defined_funcs(ast);   /* Discover all defined functions first */
		ast_xform(&ast);       /* The transformation phase */

		time(&now);  /* Advertise us */
		sprintf(advert, "/* File generated from [%s] by %s %s, %s"
		                "$OMPi__nfo:%d\n*/",
	                  filename, PACKAGE_NAME, GIT_VERSION, ctime(&now),
	                  targetnum);
		p = verbit(advert);    /* Comment @ top & other stuff */

		if (needLimits)
			p = BlockList(p, verbit("#include <limits.h>"));
		if (needFloat)
			p = BlockList(p, verbit("#include <float.h>"));
		if (needMemcpy && !knowMemcpy)
			p = BlockList(
			      p,
			      verbit("#if _WIN64 || __amd64__ || __X86_64__\n"
			             "   extern void *memcpy(void*,const void*,unsigned long int);\n"
			             "#else\n"
			             "   extern void *memcpy(void*,const void*,unsigned int);\n"
			             "#endif"
			            )
			    );
		if (needMemset && !knowMemset)
			p = BlockList(
			      p,
			      verbit("#if _WIN64 || __amd64__ || __X86_64__\n"
			             "   extern void *memset(void*,int,unsigned long int);\n"
			             "#else\n"
			             "   extern void *memset(void*,int,unsigned int);\n"
			             "#endif"
			            )
			    );

		ast = BlockList(p, ast);
	}
	append_new_main();

	cg_find_defined_funcs(ast);   /* The AST is finalized; mark all functions */

	ast_show(ast);

	if (usedmods)             /* No need otherwise */
		produce_target_files(); /* IMPORTANT: global scope is still alive! */

	if (testingmode)
	{
		/* Clean up (not needed actually; we do it only when testing)  */
		ast_free(ast);
		symtab_drain(stab);
		symbols_allfree();
		xt_free_retired();
	}
	return (0);
}


void exit_error(int exitvalue, char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(exitvalue);
}


void warning(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
