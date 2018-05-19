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

/*
 * This file contains common functionalities between
 * the host and and the hostmodules
 */

#include "rt_common.h"

int  **places_dup(int **from)
{
	int i;
	int num_of_places = numplaces(from);
	int **c_places;


	if(num_of_places <=0)
		return NULL;

	c_places = malloc(num_of_places*sizeof(int **));

	c_places[0] = malloc(sizeof(int));
	c_places[0][0] = from[0][0];

	for(i=1; i<=num_of_places; i++)
	{
		c_places[i] = malloc((placelen(from,i-1)+1)*sizeof(int));
		memcpy(c_places[i], from[i], (placelen(from,i-1)+1)*sizeof(int));
	}

	return c_places;
}

void  places_free(int **place)
{
	int n = numplaces(place) - 1;

	if(place == NULL)
		return;
	for (; n >=0; n--)
		if (placelen(place, n))
			free(place[n + 1]);
	free(place);
}
