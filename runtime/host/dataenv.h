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

/* dataenv.h */

#ifndef __DATAENV_H__
#define __DATAENV_H__

#define STSIZE 17  /* Prime */

typedef struct dataenv_ *dataenv;
typedef struct dtentry_ *dtentry;

struct dtentry_
{
	void     *host_addr;
	void     *dev_addr;
	dataenv   de;
	dtentry   bucketnext;
};

struct dataenv_
{
	dtentry  table[STSIZE];
	int      depth;
	int      device_id;
	dtentry  entries;
	int      used_entries;
	int      num_of_entries;
	dataenv  next;               /* For recycler */
	dataenv  parent;             /* Only for print */
};


void   *dataenv_get(dataenv t, void *host_addr, int device_id);
void   *dataenv_get_current_level(dataenv t, void *host_addr, int device_id);
void   *dataenv_put(dataenv t, void *host_addr, void *dev_addr);
dataenv dataenv_start(int tdvars, dataenv parent, int device_id);
void    dataenv_end  (dataenv t, void (*dev_free)(void *, void *, int));
void    dataenv_show (dataenv t);

#endif
