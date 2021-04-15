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

/* MAL_PARSE.C
 * A recursive-descent parser, used for recognizing the MAL grammar 
 * and building a MAL graph.
 */

#include <stdlib.h>
#include <stdio.h>
#include <str.h>
#include <set.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include "mal_scan.h"
#include "mal_graph.h"
#include "mapper.h"
#include "mal.h"

static scanif_t scif;            /* The scanner */
static int      token;           /* The current token */

int lastid = -1;
mal_graph_t *mgraph;

#define MAXFLAVORS 64
#define LARGE_BUFFER_SIZE 1024
static char flavors[MAXFLAVORS][LARGE_BUFFER_SIZE];
int nflavors;

jmp_buf ErrJmp;


static void parse_error(char *text, ...)
{
	int currlinepos;
	int currline = mal_scan_get_position(scif, &currlinepos);
	va_list alist;
	
	va_start(alist, text);
	fprintf(stderr, "MAL error (rules.mal:%d,%d):\n\t", currline, currlinepos);
	vfprintf(stderr, text, alist);
	va_end(alist);
	longjmp(ErrJmp, 1);
}

int is_flavor(char *name)
{
	int i;
	for (i=0; i<nflavors; i++)
		if (strcmp(flavors[i], name) == 0)
			return 1;
	return 0;
}

/* The most important function */
void expect(int toktype)
{
	if (token != toktype)
		parse_error("unexpected token '%s' (type %d instead of %d)\n", 
		            mal_scan_get_toktext(scif), token, toktype);
	token = mal_scan_nexttoken(scif);
}

void integer(int *condvalue)
{
	if (token != S_INT)
		parse_error("expected integer\n");
	*condvalue = mal_scan_get_intval(scif);
	expect(S_INT);
}

void id(char *value)
{
	if (token != S_WORD)
		parse_error("expected a word identifier\n");
	strcpy(value, mal_scan_get_toktext(scif));    /* keep token name */
	expect(S_WORD);                        /* consume the token */
}

/* relop ::= '>' | '<' | '=' | '<=' | '>=' */
void relop(mg_cond_e *condtype)
{
	switch ( token )
	{
		case EQ: *condtype = CND_EQ; break;
		case LE: *condtype = CND_LE; break;
		case GE: *condtype = CND_GE; break;
		case NE: *condtype = CND_NE; break;
		case LT: *condtype = CND_LT; break;
		case GT: *condtype = CND_GT; break;
		default: 
			parse_error("expected a relation operator\n");
	}
	expect(token);
}

/* boolean ::= TRUE | FALSE */
void boolean(mg_cond_e *condtype)
{
	if (token == S_TRUE)
		*condtype = CND_TRUE;
	else if (token == S_FALSE)
		*condtype = CND_FALSE;
	else
		parse_error("expected TRUE or FALSE\n");

	expect(token);
}

/* stringid ::= '"' id. '"' | id. */
void stringid(char *value)
{
	if (token != S_STRING && token != S_WORD)
		parse_error("expected a string\n");
	strcpy(value, mal_scan_get_toktext(scif));    /* keep token name */
	expect(token);
}

/* numedge ::= relop int. ':' stringid */
void numedge()
{
	mg_cond_e condtype;
	int condvalue;
	char adjname[BUFSIZE];

	relop(&condtype);    /* Parse a relative operator */
	integer(&condvalue); /* Parse an integer */
	expect(':');
	stringid(adjname);    /* Parse a string */

	mal_graph_add_neighbour(mgraph, lastid, adjname, condvalue, condtype);
}

/* numadjlist ::= numedge [',' numedge]* */
void numadjlist()
{
	numedge();
	while (token != '}')
	{
		expect(',');
		numedge();
	}
}

/* numquery ::= NUM ( id. ) */
void numquery()
{
	char tokval[BUFSIZE];

	expect(S_NUM);
	expect('(');
	id(tokval);
	expect(')');
	mal_graph_set_node_type(mgraph, lastid, Q_NUM, tokval);
}

/* booledge ::= boolean ':' stringid */
void booledge()
{
	mg_cond_e condtype;
	char adjname[BUFSIZE];

	boolean(&condtype);
	expect(':');
	stringid(adjname);
	mal_graph_add_neighbour(mgraph, lastid, adjname, 0, condtype);
}

/* booladjlist ::= booledge ',' booledge */
void booladjlist()
{
	booledge();
	expect(',');
	booledge();
}

/* boolquery ::= HAS ( id. ) | HASONLY ( id. ) */
void boolquery()
{
	char       tokval[BUFSIZE];
	mg_qtype_e qtype = (token == S_HAS) ? Q_HAS : Q_HASONLY;

	expect(token);   /* We already know that token is either HAS or HASONLY */
	expect('(');
	id(tokval);
	expect(')');

	mal_graph_set_node_type(mgraph, lastid, qtype, tokval);
}

/* nodebody ::= boolquery ',' booladjlist | numquery ',' numadjlist */
void nodebody()
{
	char tokval[BUFSIZE];
	
	strcpy(tokval, mal_scan_get_toktext(scif));
	if (token == S_HAS || token == S_HASONLY)
	{
		boolquery();
		expect(',');
		booladjlist();
	}
	else if (token == S_NUM)
	{
		numquery();
		expect(',');
		numadjlist();
	}
	else
		parse_error("expected `has`, `hasonly` or `num` as a query type\n");
}

/* node ::= id [ '=' ] '{' nodebody '}' */
void node()
{
	char tokval[BUFSIZE];   /* To remember the node name */

	id(tokval);

	/* Create the node before reading its body. */
	if (mal_graph_search(mgraph, tokval) == -1)
		lastid = mal_graph_add_node(mgraph, tokval, 0);

	if (token == '=')                     /* = is optional */
		expect('=');
	expect('{');
	nodebody();    /* store the fields somewhere */
	expect('}');
}

/* nodelist ::= NODES '=' '[' node ',' node ',' node ',' ... ']' */
void nodelist()
{
	expect(S_NODES);
	expect('=');
	expect('[');
	while (token != ']')
	{
		node();
		if (token != ']')
			expect(',');     /* consume the ',' */
	}
	expect(']');
}

/* flavor ::= stringid */
void flavor()
{
	char tokval[BUFSIZE];   /* To remember the flavor name */

	stringid(tokval);

	/* Add the new flavor to the node list */
	if (mal_graph_search(mgraph, tokval) == -1)
	{
		if (nflavors >= MAXFLAVORS)
			parse_error("too many flavors\n");
		lastid = mal_graph_add_node(mgraph, tokval, 1);
		strncpy(flavors[nflavors], tokval, BUFSIZE);
		nflavors++;
	}
}

/* flavorlist ::= FLAVORS '=' '[' flavor ',' flavor ',' flavor ',' ... ']' */
void flavorlist()
{
	expect(S_FLAVORS);
	expect('=');
	expect('[');

	while (token != ']')
	{
		flavor();
		if (token != ']')
			expect(',');     /* consume the ',' */
	}

	expect(']');
}

/* mal ::= flavorlist [ ',' ] nodelist */
void mal()
{
	flavorlist();
	if (nflavors == 0)
		parse_error("the rules file should contain at least one flavor\n");
	if (token == ',')         /* Optional comma */
		expect(',');
	nodelist();
}


int mal_parse(FILE *mal_file, malgr_t *g)
{
	int         i;
	char        *pn;
	
	*g = NULL;
	if ((mgraph = (mal_graph_t *) malloc(sizeof(mal_graph_t))) == NULL)
		return -2;
	
	scif = mal_scan_file_init(mal_file);
	token = mal_scan_nexttoken(scif);      /* Get our first token */
	mal_graph_init(mgraph);
	
	if (setjmp(ErrJmp))
	{
		mal_scan_done(scif);
		free(mgraph);                  /* Maybe free some of the fields, too? */
		return -1;
	}
	
	mal();                           /* Now call the grammar parser */

	/* Check if pending nodes exist and if yes, abort */
	if ((pn = mal_graph_unfinished(mgraph)) != NULL)
		parse_error("undeclared node %s\n", pn);
	mal_scan_done(scif);
	*g = (malgr_t) mgraph;
	return 0;
}
