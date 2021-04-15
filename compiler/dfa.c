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

/* dfa.h -- Data flow analysis module for OMPi */

#include <stdio.h>
#include <stdlib.h>
#include "symtab.h"
#include "ompi.h"
#include "dfa.h"

SET_TYPE_IMPLEMENT(dfa)

void dfa_userfunc_add(symbol f, aststmt fBody)
{
}

autoshattr_t *dfa_parreg_get_results(int l)
{
	return NULL;
}

void dfa_parreg_remove(int l)
{
}

autoshattr_t dfa_analyse(aststmt tree)
{
	autoshattr_t as;
	return as;
}
