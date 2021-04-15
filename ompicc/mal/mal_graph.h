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

#ifndef __MALGRAPH_H__
#define __MALGRAPH_H__

#define BUFFER_SIZE 256
#define MAX_MGNEIGHBORS 16

typedef enum {
	CND_TRUE = 0, CND_FALSE, CND_GE, CND_LE, CND_EQ, CND_NE, CND_GT, CND_LT
} mg_cond_e;

typedef enum {
	Q_HAS = 0, Q_HASONLY, Q_NUM
} mg_qtype_e;

typedef struct {
	int id;
	int value;
	mg_cond_e type;
} mgadjnode_t;

typedef struct {
	int         id;                    /* node id */
	char        name[BUFFER_SIZE];     /* node name */
	mg_qtype_e  qtype;                 /* node (query) type */
	char        qompcon[BUFFER_SIZE];  /* the OpenMP construct queried */
	int         degree;                /* # neighbors */
	mgadjnode_t neighbors[MAX_MGNEIGHBORS];
} mgnode_t;

typedef struct {
	mgnode_t *nodes;   /* grows dynamically */
	int size;          /* # nodes */
	int rootid;        /* id of root node (first non-terminal node) */
} mal_graph_t;

void mal_graph_init(mal_graph_t *m);
void mal_graph_set_node_type(mal_graph_t *m, int id, mg_qtype_e type, char *ompcon);
int  mal_graph_search(mal_graph_t *m, char *nodename);
int  mal_graph_add_node(mal_graph_t *m, char *name, int isflavor);
int  mal_graph_add_neighbour(mal_graph_t *m, int id, char *nodename, int condval, int condtype);
char *mal_graph_unfinished(mal_graph_t *m);

#endif  /* __MALGRAPH_H__  */
