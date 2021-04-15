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

/* MAPPER.C
 * Mapper mechanism used by ompicc to support adaptive runtime library
 * selection for kernels.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <keyval.h>
#include <str.h>
#include "mapper.h"
#include "mal.h"

#define RULESFILE "rules.mal"
#define MAX_METRICS 32

typedef struct map_module_s
{
	malgr_t graph;
	char    *modulename;
} *map_module_t;

typedef struct metrics_s
{
	char *name[MAX_METRICS];
	int  value[MAX_METRICS];
	int  nmetrics;
	int  nactive;
} *metrics_t;


/*
 * Functions to load the metrics of a particular kernel
 */


static
void assign_metric(char *key, char *value, void *arg) 
{
	metrics_t mtr = (metrics_t) arg;
	int       n = mtr->nmetrics;
	
	/* TODO: we should search if the key already exists and replace its value */
	if (n >= MAX_METRICS)
		return;

	/* FIXME: ugly hack to avoid going past the last metric... */
	if (n >= 19)
		return;
	
	/* max_par_lev is not counted as a metric, but as an additional parameter 
	   in the flavor decision */
	if (strcmp(key, "max_par_lev") == 0)
		return;

	mtr->name[n] = strdup(key);
	if (strcmp(key, "criticals") == 0)  /* Critical names treated differently */
		mtr->value[n] = (strcmp(value, "0") != 0);
	else
		mtr->value[n] = atoi(value);

	/* count every metric except for `openmp` and scheduling */
	if (strcmp(key, "openmp") && (strstr(key, "sched") == 0) && 
		  (strstr(key, "has") == 0)) 
		mtr->nactive += (mtr->value[n] > 0);
	mtr->nmetrics++;
}


static 
void get_kernel_metrics(char *filename, metrics_t mtr) 
{
	FILE *fp;

	if ((fp = fopen(filename, "r")) == NULL)
		return;
	fscanf(fp, "$OMPi__CARS:");   /* Get to the metrics section */
	keyval_read(fp, assign_metric, (void *) mtr);
	fclose(fp);
}


/* 
 * Metric query funtions required for graph traversal
 */


int query_has(void *allmetrics, char *metric, ...)
{
	metrics_t mtr = (metrics_t) allmetrics;
	int       i, exists = 0;

	if (strcmp(metric, "openmp") == 0)
		return mtr->nactive;
	for (i = 0; i < mtr->nmetrics; i++)
		if (strcmp(mtr->name[i], metric) == 0)
		{
			if (mtr->value[i] > 0)
				return 1;
			exists = 1;
		};
	return (exists) ? 0 : -1;
}


int query_hasonly(void *allmetrics, char *metric, ...)
{
	metrics_t mtr = (metrics_t) allmetrics;
	int       i, hasmetric = 0;

	for (i = 0; i < mtr->nmetrics; i++)
		if (mtr->value[i] > 0)
		{
			if (strcmp(mtr->name[i], metric))
				return 0;
			else
				hasmetric = 1;
		};
	return hasmetric; 
}


int query_num(void *allmetrics, char *metric, ...)
{
	metrics_t mtr = (metrics_t) allmetrics;
	int       i;
	
	if (strcmp(metric, "totalmetrics") == 0)
		return (mtr->nactive);
	for (i = 0; i < mtr->nmetrics; i++)
		if (strcmp(mtr->name[i], metric) == 0)
			return mtr->value[i];
	return -1;
}


static 
char *decide_flavor(map_module_t mm, metrics_t mtr)
{
	mg_queryfunc_t queryfuncs[] = { query_has, query_hasonly, query_num };
	return mal_graph_traverse(mm->graph, queryfuncs, mtr);
}


char *mapper_select_flavor(mapmod_t mm, char *kernelpath)
{
	if (mm)
	{
		struct metrics_s metrics;
		char *devflv;

		metrics.nmetrics = 0;
		metrics.nactive = 0;
		get_kernel_metrics(kernelpath, &metrics);
		devflv = decide_flavor((map_module_t) mm, &metrics);
		if (strcmp(devflv, "-1"))
			return devflv;
	}
	return NULL;
}


/**
 * Initialize mapper for a given module, i.e. get its MAL rules file (if any)
 * and build the corresponding graph.
 * @param modulename  the name of the module
 * @return the mapper module structure or NULL on error
 */
mapmod_t mapper_load_module(char *modulename)
{
	FILE *rulesfp;
	map_module_t mm = malloc(sizeof(struct map_module_s));
	static str tmpstr = NULL;
	
	if (!mm)
		return NULL;
	mm->modulename = strdup(modulename);
	
	if (tmpstr == NULL) 
		tmpstr = Strnew();
	else
		str_truncate(tmpstr);
	str_printf(tmpstr, "%s/devices/%s/%s", LibDir, modulename, RULESFILE);
	if ((rulesfp = fopen(str_string(tmpstr), "r")) == NULL)
	{
		fprintf(stderr, "[mapper]: cannot open rules file %s\n",str_string(tmpstr));
		return NULL;
	}
	
	if ( mal_parse(rulesfp, &mm->graph) )
	{
		fprintf(stderr, "[ mapper => using default flavor for module %s]\n", 
		        modulename);
		free(mm);
		mm = NULL;
	}
	fclose(rulesfp);
	return (mapmod_t) mm;
}


/**
 * Free a module loaded my the mapper
 */
void mapper_free_module(mapmod_t mm)
{
	if (mm)
	{
		mal_graph_free(((map_module_t) mm)->graph);
		free(((map_module_t) mm)->modulename);
		free(mm);
	}
}
