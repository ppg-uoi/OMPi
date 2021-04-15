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

/* devpart.c
 * This is the device-side part of the module.
 * It is to be linked with every kernel.
 */

/* This converts a usable mediary address to an actual device address.
 * The size argument should be useless but is given as a possible help; if uaddr 
 * refers to a known, mapped object, then it represents its size in bytes in
 * the device memory. Otherwise, it is simply 0.
 * This is called in all kernels.
 */
char *devpart_med2dev_addr(void *uaddr, unsigned long size)
{
  if (uaddr == NULL) return (NULL);   /* Handle NULL */
  return (uaddr); 
}
