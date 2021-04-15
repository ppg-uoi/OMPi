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

/* KERNELS.H
 * Module and kernels support for ompicc
 */

#ifndef __KERNELS_H__
#define __KERNELS_H__

extern int nmodules;

extern void modules_employ(char *modstr);   /* Find requested modules/devices */
extern char *modules_argfor_ompi();         /* Make a string to pass to _ompi */
extern void modules_show_info(int verbose); /* Show module/device info        */
extern void kernel_makefiles(char *fname, int nkernels);   /* Produce kernels */

#endif /* __KERNELS_H__ */
