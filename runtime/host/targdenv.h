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

/* targdenv.h */

#ifndef __TARGDENV_H__
#define __TARGDENV_H__

#include "stddefs.h"

#define STSIZE 17  /* Prime */

typedef struct tdenv_s  *tdenv_t;
typedef struct tditem_s *tditem_t;

struct tditem_s
{
	void    *hostvar;        /* The variable itself */
	void    *hostaddr;       /* The space it refers to (=hostvar except for ptrs) */
	void    *imedaddr;       /* The internal mediary address hostaddr maps to */
	int      init_imedaddr;  /* Initial internal mediary address (used for MPI initialization) */
	void    *initfrom;       /* Where to initialize from (only for globals) */
	size_t   secoffset;      /* array section offset (in bytes) from hostaddr ... */
	size_t   seclen;         /* ... and length (in bytes) */
	int      noalloc;        /* till we do reference counters; not allocated */
	int      refctr;         /* Reference counter */
	int      decltarg;       /* True for #declare target vars (to differentiate them
	                            from other injected (e.g. #target enter data) ones */
	int      bylink;         /* True only for link() #declare target vars */
	char     firstmapguard;  /* Is this the first variable to map this space (bool) */
	tdenv_t  de;
	tditem_t bucketnext;
};

struct tdenv_s
{
	tditem_t table[STSIZE];
	int      depth;
	int      devid;
	tditem_t entries;
	int      used_entries;
	int      num_of_entries;
	tdenv_t  next;               /* For recycler */
	tdenv_t  parent;             /* Only for print */
};

#define IsMapped(e) ((e) && (e)->imedaddr != NULL)
#define DTto(e)     ((e) && (e)->decltarg && !(e)->bylink)
#define DTlink(e)   ((e) && (e)->decltarg && (e)->bylink)

/* Global scope */
void     tdenv_init_globals();
tdenv_t  tdenv_global_env(int devid);
tditem_t tdenv_global_get(int deviceid, void *hostvar);

/* All other scopes */
tdenv_t  tdenv_start(int tdvars, tdenv_t parent, int deviceid);
void     tdenv_end(tdenv_t t, void (*dev_free)(void *, void *, int));
tditem_t tdenv_get(tdenv_t t, int deviceid, void *hostvar);
tditem_t tdenv_get_currlevel(tdenv_t t, int deviceid, void *hostvar);
tditem_t tdenv_put(tdenv_t t,
                  void *hostvar, void *hostaddr, size_t secoffst, size_t seclen,
                  void *imedaddr, int noalloc);
tditem_t tdenv_rem(tdenv_t t, int devid, void *hostvar);
void     tdenv_show(tdenv_t t);   /* debug */

/* Ranges */
#define PointInside(p,lb,ub) ((lb) <= (p) && (p) <= (ub))
int     tdenv_search_range(tdenv_t t,
                         void *haddr, size_t seclen, size_t secoff,tditem_t *e);
        /* tdenv_search_range() return values */
#define RANGE_NEW      0      /* No overlap and base address never met before */
#define RANGE_SAMEBASE 1      /* No overlap but base address known */
#define RANGE_OVERLAPS 2      /* There exists an overlapping range */
#define RANGE_INCLUDED 3      /* This is a proper subset of an existing range */

#endif /* __TARGDENV_H__ */
