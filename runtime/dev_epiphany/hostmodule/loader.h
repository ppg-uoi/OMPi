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

#ifndef __LOADER_H__
#define __LOADER_H__

/*
 * Extra definitions for esdk 2015.1
 */
#if ESDKVERSION == 2015
#define E_CORE_GP_REG_BASE  0xf0000
#define E_REG_CORE_RESET    E_REG_RESETCORE
#include <e-loader.h>
#else
#include <e-hal.h>
#endif

#define PARALLELLA_ROWS                    4
#define PARALLELLA_COLS                    4
#define PARALLELLA_CORES                   16
#define MAX_KERNEL_FILES                   64

typedef struct kernel_code_s
{
	char *executable;
	char *buf;
	int   lines;
	char  hdr[5];
} kernel_code_t;

int  my_e_load_group(char *executable, e_epiphany_t *dev, unsigned row,
                     unsigned col, unsigned rows, unsigned cols, e_bool_t start);
void init_kernel_buffers(void);
int my_e_reset_group(e_epiphany_t *dev[], int num_of_groups);
void ee_write_buf(e_epiphany_t *, unsigned int, unsigned int, unsigned long, unsigned char[], unsigned int);
void ee_mwrite_buf(e_mem_t *, unsigned long, unsigned char[], unsigned int);
void ee_get_coords_from_id(e_epiphany_t *, unsigned int, unsigned int *, unsigned int *);
void ee_set_core_config(e_epiphany_t *, e_mem_t *, unsigned int, unsigned int);

#endif /* __LOADER_H__ */
