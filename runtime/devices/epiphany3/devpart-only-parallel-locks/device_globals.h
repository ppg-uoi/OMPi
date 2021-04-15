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

#ifndef __DEVICE_GLOBALS_H__
#define __DEVICE_GLOBALS_H__

#include "device.h"

extern pe_shared_t     mbox[PE_SHAREDDATA_SIZE] SECTION("section_shared_bank"); // Last 2KB of bank 3
extern private_eecb_t  default_eecb             SECTION("section_shared_bank"); // Last 2KB of bank 3
extern private_eecb_t *myeecb                   SECTION("section_shared_bank"); // Last 2KB of bank 3

#ifdef MEASURE
extern unsigned int sleep_time                  SECTION("section_shared_bank");
#endif

#ifdef USER_LOCKS
extern locking_t       lock_table;
#endif

#ifndef OLD_BAR
	extern ee_barrier_t team_barrier[MAX_ACTIVE_LEVELS];
#endif

#endif /* __DEVICE_GLOBALS_H__ */

