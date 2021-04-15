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

/* MAL_GRAPH.C
 * A graph structure specifically used for storing an entire MAL rule file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "mal.h"
#include "mal_graph.h"


/* This global pending nodes is the single reason this code is not re-entrant.
 * However, there is no re-entrancy requirement here, and it makes things
 * quite more manageable.
 */
#define MAX_PENDING_NODES 128
static mgnode_t pending_nodes[MAX_PENDING_NODES];
static int      npending;


/*
 * Build the graph
 */


/* Grow a graph in 10-node increments, if needed */
static 
int grow_graph(mal_graph_t *m)
{
	/* FIXME: need to check for NULL allocations */
	if (m->size == 0)
		m->nodes = malloc(10 * sizeof(mgnode_t));
	else
		if (m->size % 10 == 0)
			m->nodes = realloc(m->nodes, (m->size + 10) * sizeof(mgnode_t));
	return ( m->size++ );
}


void mal_graph_init(mal_graph_t *m)
{
	m->nodes = NULL;
	m->rootid = -1;
	m->size = 0;
	npending = 0;
}


/* Initialize the adjacency array of a mal node */
static
void init_neighbors(mal_graph_t *m, int id)
{
	int i;
	mgadjnode_t *adjacents = m->nodes[id].neighbors;

	for (i=0; i<MAX_MGNEIGHBORS; i++)
		adjacents[i].id = -1;
	m->nodes[id].degree = 0;
}


void mal_graph_free(malgr_t g)
{
	int i;

	if (g)
	{
		mal_graph_t *m = (mal_graph_t *) g;

		if (m->nodes)
			free(m->nodes);
		m->rootid = -1;
		m->size = 0;
	}
	for (i=0; i<npending; i++)
		pending_nodes[i].id = -1;
	npending = 0;
}


/* Add a mal node to a graph's pending nodes list */
static 
int add_pending_node(mal_graph_t *m, char *nodename)
{
	mgnode_t newpn;
	int newid = grow_graph(m);

	/* FIXME: somehow signal an error */
	if (npending == MAX_PENDING_NODES)
		return -1;
	strncpy(pending_nodes[npending].name, nodename, BUFFER_SIZE);
	pending_nodes[npending].id = newid;
	npending++;
	return newid;
}


/* Return 1 if a mal node is terminal in the given graph  */
static 
int node_is_term(mal_graph_t *m, int id)
{
	int i;

	for (i=0; i<MAX_MGNEIGHBORS; i++)
		if (m->nodes[id].neighbors[i].id != -1)
			return 0;
	return 1;
}


/* 
 * Set a mal node's query type (has|hasonly|num).
 */
void mal_graph_set_node_type(mal_graph_t *m, int id, mg_qtype_e type, char *qompcon)
{
	m->nodes[id].qtype = type;
	strncpy(m->nodes[id].qompcon, qompcon, BUFFER_SIZE);
}


/* 
 * Get the id of a node, given its name.
 */
int mal_graph_search(mal_graph_t *m, char *nodename)
{
	int i;
	for (i = 0; i < m->size; i++)
	{
		if (strcmp(m->nodes[i].name, nodename) == 0)
			return m->nodes[i].id;
	}

	return -1;
}


/**
 * Returns the name of the 1st pending node or NULL if there is none.
 */
char *mal_graph_unfinished(mal_graph_t *m)
{
	int i;
	
	for (i=0; i<npending; i++)
		if (pending_nodes[i].id != -1)
			return pending_nodes[i].name;
	return NULL;
}


/* 
 * Get the index of a pending node (node that was seen
 * as a neighbour but is not declared yet).
 */
static
int pending_search(mal_graph_t *m, char *node)
{
	int i;
	
	for (i=0; i<npending; i++)
		if (pending_nodes[i].id != -1 && strcmp(node, pending_nodes[i].name)==0)
			return i;
	return -1;
}


/* 
 * Add a node to a mal graph.
 */
int mal_graph_add_node(mal_graph_t *m, char *name, int isflavor)
{
	int nid;
	int pnindex;

	if ((pnindex = pending_search(m, name)) == -1)
		nid = grow_graph(m);
	else 
	{
		nid = pending_nodes[pnindex].id;   /* Keep pending node ID for later use */
		pending_nodes[pnindex].id = -1;    /* Free pending node */
	}

	strncpy(m->nodes[nid].name, name, BUFFER_SIZE);
	m->nodes[nid].id = nid;

	init_neighbors(m, nid);

	if (!isflavor && m->rootid < 0)
		m->rootid = nid;
	return nid;
}


/* 
 * Get a mal graph node by its id.
 */
mgnode_t mal_graph_get_node(mal_graph_t *m, int id)
{
	return m->nodes[id];
}


/* 
 * Add an adjacency condition to an existing mal node.
 */
void set_node_cond(mal_graph_t *m, int id, int nindex, int value, int type)
{
	/* FIXME: handle errors gracefully */
	if (m->nodes[id].degree < nindex)
	{
		fprintf(stderr, "error: invalid neighbour index %d\n", nindex);
		exit(0);
	}
	m->nodes[id].neighbors[nindex].value = value;
	m->nodes[id].neighbors[nindex].type  = type;
}


/* 
 * Add a neighbour in an existing mal node's adjacency list,
 * by declaring its name and its condition.
 */
int mal_graph_add_neighbour(mal_graph_t *m, int id, char *nodename,
		int condval, int condtype)
{
	int nid;

	/* Check if node is known */
	if ((nid = mal_graph_search(m, nodename)) == -1)
	{
		if ((nid = pending_search(m, nodename)) != -1)
			nid = pending_nodes[nid].id;
		else
			nid = add_pending_node(m, nodename);
	}
	else
	{
		/* Check if we are going to form a cycle 
		A cycle can only be formed if the adjacent
		has already been declared. */
		mgnode_t mnode = mal_graph_get_node(m, nid);
		int x;
		
		for (x=0; x<MAX_MGNEIGHBORS; x++)
		{
			if (m->nodes[nid].neighbors[x].id == id)
			{
				fprintf(stderr, "error: an edge from %d to %d already exists.\n", nid, id);
				exit(0);
			}
		}
	}

	m->nodes[id].neighbors[m->nodes[id].degree].id = nid;
	set_node_cond(m, id, m->nodes[id].degree, condval, condtype);
	m->nodes[id].degree++;
	return nid;
}


/* 
 * Traverse the graph
 */


static
char *visit_node(mal_graph_t *m, int id, mg_queryfunc_t *qfuncs, void *uarg)
{
	char *constr = m->nodes[id].qompcon;
	int x, y, next_node = -1, i;

	if (id == -1)
		return NULL;
	
	/* We have reached a terminal node (flavor) */
	if (node_is_term(m, m->nodes[id].id))
		return m->nodes[id].name;
	
	/* Decide which query function to use, based on the query type of nodebody */
	switch ( m->nodes[id].qtype )
	{
		case Q_HAS:
			x = qfuncs[Q_HAS](uarg, (constr)); 
			break;
		case Q_HASONLY: 
			x = qfuncs[Q_HASONLY](uarg, (constr)); 
			break;
		case Q_NUM:
			x = qfuncs[Q_NUM](uarg, (constr)); 
			break;
		default:
			return NULL;
	}

	/* Find the first condition that is true and
	visit the neighbour that is matched with it */
	for (i=0; i<MAX_MGNEIGHBORS; i++)
	{
		if (next_node != -1)
			break;

		y = m->nodes[id].neighbors[i].value; 
		switch ( m->nodes[id].neighbors[i].type )
		{
			/* has/hasonly queries */
			case CND_TRUE:  next_node = (x)     ? i : -1; break;
			case CND_FALSE: next_node = (!x)    ? i : -1; break;
			/* num queries */
			case CND_GE:    next_node = (x>=y)  ? i : -1; break;
			case CND_LE:    next_node = (x<=y)  ? i : -1; break;
			case CND_EQ:    next_node = (x==y)  ? i : -1; break;     
			case CND_NE:    next_node = (x!=y)  ? i : -1; break; 
			case CND_GT:    next_node = (x> y)  ? i : -1; break; 
			case CND_LT:    next_node = (x< y)  ? i : -1; break; 
			default: 
				break;
		}
	}

	if (next_node == -1) /* If none of the conditions is true, we have an error */
		return NULL;
	
	/* Visit the selected node */
	return ( visit_node(m, m->nodes[id].neighbors[next_node].id, qfuncs, uarg) );
}

/* 
 * Begin the graph traversal by visiting the first node
 * that is not a flavor.
 */
char *mal_graph_traverse(malgr_t g, mg_queryfunc_t *qfuncs, void *userarg)
{
	mal_graph_t *m = (mal_graph_t *) g;
	return ( visit_node(m, m->rootid, qfuncs, userarg) );
}


/* 
 * Print a mal graph.
 */
void mal_graph_print(mal_graph_t *m) 
{
	int i, j;
	
	for (i=0; i<m->size; i++)
	{
		fprintf(stderr, "graph node %s(%d) type %d\n", m->nodes[i].name, m->nodes[i].id, 
			m->nodes[i].qtype);
		if (strlen(m->nodes[i].qompcon) > 0)
			fprintf(stderr, "  construct used: %s\n", m->nodes[i].qompcon);
		if (i == m->rootid)
			fprintf(stderr, "  is root node\n");
		if (node_is_term(m, m->nodes[i].id))
			fprintf(stderr, "  is terminal\n");
		for (j=0; j<m->nodes[i].degree; j++)
		{
			fprintf(stderr, "    neighbour %d - condition value %d, type %d\n",
				m->nodes[i].neighbors[j].id, m->nodes[i].neighbors[j].value, 
				m->nodes[i].neighbors[j].type);
		}
			
		for (j=0; j<m->nodes[i].degree; j++)
			fprintf(stderr, "    condition val %d type %d\n", 
				m->nodes[i].neighbors[j].value, m->nodes[i].neighbors[j].type);
	}
}
