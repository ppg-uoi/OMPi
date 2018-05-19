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

/* keyval.c -- A simple reader for key-value files */

/* The input file is assumed to hold lines of the form
 *    keyword = string
 * Any leading & trailing spaces from "string" get removed.
 * Any lines starting with # are considered comments and are ignored.
 * Any other (wrong) lines are ignored.
 *
 * Use it by calling keyval_read(fp, act).
 * For each correct line found, it calls act(keyword, string).
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "keyval.h"


/*
 * The scanner
 */


static FILE *infile;
static int lastchar;


static void sc_init_scanner(FILE *fp)
{
	infile   = fp;
	lastchar = '\n';
}


static void sc_nextchar()
{
	lastchar = fgetc(infile);
}


static void sc_ignore_line()
{
	for (; lastchar != '\n' && lastchar != EOF; sc_nextchar())
		;
	if (lastchar == '\n')
		sc_nextchar();
}


static void sc_skip_blank_lines()
{
	while (1)
	{
		while (isspace(lastchar))
			sc_nextchar();

		if (lastchar == '#')       /* Comment line */
			sc_ignore_line();
		else
			break;
	}
}


static void sc_skip_spaces()
{
	while (isspace(lastchar))
		sc_nextchar();
}


static int sc_get_word(int n, char *word)
{
	int len;

	sc_skip_spaces();

	if (lastchar == EOF)
		return (EOF);

	if (lastchar == '=')
	{
		sc_nextchar();
		word[0] = '=';
		word[1] = 0;
		return (0);
	}

	for (len = 0; !isspace(lastchar) && len < n - 1; sc_nextchar())
		if (lastchar == EOF || lastchar == '=')
			break;
		else
			word[len++] = lastchar;

	word[len++] = 0;
	return (0);
}


static void sc_get_rest_of_line(int n, char *buf)
{
	int i = 0;

	for (; lastchar != '\n' && lastchar != EOF && i < n - 1; sc_nextchar())
		buf[i++] = lastchar;
	buf[i++] = 0;
	if (lastchar == EOF)
		return;
	sc_ignore_line();           /* Discard remaining line */

	/* Remove trailing spaces */
	for (i -= 2; i >= 0; i--)
		if (!isspace(buf[i]))
			break;
	buf[i + 1] = 0;
}


/*
 * The driver
 */


void keyval_read(FILE *fp, void (*act)(char *key, char *value))
{
	char key[KV_KEYLEN], value[KV_VALUELEN];

	for (sc_init_scanner(fp); ;)
	{
		sc_skip_blank_lines();

		if (sc_get_word(KV_KEYLEN, key) == EOF) return;
		if (sc_get_word(KV_VALUELEN, value) == EOF) return;
		if (strcmp(value, "="))   /* errorneous line */
		{
			sc_ignore_line();
			continue;
		}

		sc_skip_spaces();
		sc_get_rest_of_line(KV_VALUELEN, value);
		act(key, value);
	}
}
