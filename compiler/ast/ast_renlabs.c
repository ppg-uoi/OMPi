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

/* ast_renlabs.c -- renames labels */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast_traverse.h"

#define MAXLABS 1024
static int _newlabs;
static char _ln[128];
symbol _renamed[MAXLABS];

static
symbol new_label_name(symbol oldlabel)
{
	int i;

	if (_newlabs == MAXLABS)
	{
		fprintf(stderr, "[new_label_name]: too many labels !!\n");
		return (oldlabel);
	}
	for (i = 0; i < _newlabs; i++)
		if (_renamed[i] == oldlabel)   /* done that */
			break;
	if (i == _newlabs)               /* first time seen */
		_renamed[ _newlabs++ ] = oldlabel;

	sprintf(_ln, "%s__%d_", _renamed[i]->name, i);
	return (Symbol(_ln));
}


void renlabs_goto_labeled(aststmt t, void *ignore1, int ignore2)
{
	t->u.label = new_label_name(t->u.label);
}


void ast_stmt_renlabs(aststmt tree)
{
	travopts_t renlabopts;
	
	travopts_init_noop(&renlabopts);
	renlabopts.doexpr = 0;    /* No need to traverse anything except stmts */
	renlabopts.dospec = 0;
	renlabopts.dodecl = 0;
	renlabopts.doomp  = 0;
	renlabopts.doox   = 0;
	renlabopts.stmtc.goto_c  = renlabs_goto_labeled;
	renlabopts.stmtc.label_c = renlabs_goto_labeled;
	ast_stmt_traverse(tree, &renlabopts);
}
