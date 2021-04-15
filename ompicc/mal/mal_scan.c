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

/* MAL_SCAN.C -- Simple re-entrant scanner for MAL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mal_scan.h"


typedef struct {
	char *key;
	int  value;
} keyval_t;


/* Our keywords, along with their return values */
static keyval_t keyword[] = {
	{ "flavors", S_FLAVORS },
	{ "nodes",   S_NODES },
	{ "true",    S_TRUE },
	{ "false",   S_FALSE },
	{ "has",     S_HAS },
	{ "hasonly", S_HASONLY },
	{ "num",     S_NUM }
};


typedef struct {
	char *inputstring;       /* If reading from a string */
	FILE *fp;                /* If reading from a file */
	void (*nextcharfunc)();  /* Function to call upon nextchar() */
	int  stringpos;          /* current pos in string (if str scan) */
	int  linepos;            /* position in current line */
	int  currline;           /* current line no. */
	char currchar;           /* current char (always 1 ahead) */
	char toktext[BUFSIZE+1]; /* The caller can find the text here */
	int  toklen;             /* = strlen( toktext ) */
	int  tokival;            /* Here is any read integer stored */
} scanner_t;


/* Get next char from a file */
static void nextcharfile(scanner_t *s)
{
	s->currchar = fgetc(s->fp);
	s->linepos++;
}


/* Get next char from a string */
static void nextcharstring(scanner_t *s)
{
	if ((s->currchar = s->inputstring[ s->stringpos ]) == 0)
		s->currchar = EOF;
	else
	{
		s->stringpos++;
		s->linepos++;
	}
}


scanif_t mal_scan_file_init(FILE *fp)
{
	scanner_t *s;

	if ((s = (scanner_t *) malloc(sizeof(scanner_t))) == NULL)
		return (NULL);
	s->linepos = s->currline = 0;
	s->currchar = '\n';
	s->nextcharfunc = nextcharfile;
	s->fp = fp;
	return ((void *) s);
}


scanif_t mal_scan_string_init(char *string)
{
	scanner_t *s;

	if ((s = (scanner_t *) malloc(sizeof(scanner_t))) == NULL)
		return (NULL);
	s->stringpos = s->linepos = s->currline = 0;
	s->currchar = '\n';
	s->nextcharfunc = nextcharstring;
	s->inputstring = string;
	return ((void *) s);
}


/* Re-initialize a previously used scanner state with a new string */
scanif_t mal_scan_string_reinit(scanif_t *st, char *string)
{
	scanner_t *s = (scanner_t *) st;

	if (st == NULL)    /* Equivalent to mal_scan_init() */
		return ( mal_scan_string_init(string) );
	s->stringpos = s->linepos = s->currline = 0;
	s->currchar = '\n';
	s->inputstring = string;
	return (st);
}


void mal_scan_done(scanif_t *st)
{
	free(st);
}


/* Check for legal char to *start* a word */
static int isinitwordchar(scanner_t *s, char c)
{
	return ( isalpha(c) || c == '_' );
}

/* Check for legal char to within a word */
static int iswordchar(scanner_t *s, char c)
{
	return ( isalnum(c) || c == '_' || c == '-');  /* allow '-', too */
}


static int check_keyword(scanner_t *s, char *text)
{
	int i;

	for (i = sizeof(keyword)/sizeof(keyval_t) - 1; i >= 0; i--)
		if (strcasecmp(keyword[i].key, text) == 0)
			return ( s->tokival = keyword[i].value );
	return ( S_WORD ); 
}


/* Main scanner function.
* Returns:
* ========
*   EOF       on EOF / EndOfString
*   a char    if single char read
*   S_INT     if an integer was read
*   S_WORD    if a word (other than the keywords) was read
*   any of the given keywords if the corresponding text was read
*/
int mal_scan_nexttoken(void *ss)
{
	scanner_t *s = (scanner_t *) ss;
	char      c;

#define curCH    (s->currchar)
#define nextchar (*(s->nextcharfunc))

	NS:

	/* Skip spaces */
	while (isspace(curCH))
	{
		if (curCH == '\n')
		{
			s->currline++;
			s->linepos = 0;
		}
		nextchar(s);
	}
	
	/* Skip comments till end of line
 */
	if (curCH == '#')
	{
		while (curCH != '\n' && curCH != EOF)
			nextchar(s);
		goto NS;
	}
	
	s->toklen = 0;

	/* Check for a number */
	if ( isdigit(curCH) )
	{
		int sign = 0;

		if (curCH == '-')         /* Prepare for a negative number */
		{
			sign = 1;
			s->toktext[ s->toklen++ ] = '-';
			nextchar(s);
		}

		for ( ; isdigit(curCH) && (s->toklen < BUFSIZE); nextchar(s))
			s->toktext[ s->toklen++ ] = curCH;

		if (sign && s->toklen == 1)           /* A single '-' */
			goto SECONDCHAR;                    /* Not a number, after all! */

		s->toktext[s->toklen] = 0;            /* Calculate the value & return */
		s->tokival = atoi(s->toktext);
		return ( S_INT );
	}

	/* String */
	if (curCH == '\"')
	{
		for (nextchar(s); curCH != '\"'; nextchar(s))
			if (s->toklen < BUFSIZE)
			{
				if (curCH == EOF)
					return (S_STREOF);
				if (curCH == '\\')     /* Escape char */
				{
					nextchar(s);
					switch (curCH)
					{
						case 'n': curCH = '\n'; break;
						case 't': curCH = '\t'; break;
						case 'b': curCH = '\b'; break;
					} /* In any other case \x = x (also for x = ") */
				}
				s->toktext[ s->toklen++ ] = curCH;
			}
			else
				return (S_STRBIG);
		
		nextchar(s);               /* Since we just read the closing " */
		s->toktext[s->toklen] = 0;
		return ( S_STRING );
	}
	
	/* Check for keyword or any other (unknown) word. */
	if (isinitwordchar(s, curCH))    /* A name or a key word */
	{
		s->toktext[ s->toklen++ ] = curCH;
		for (nextchar(s); iswordchar(s,curCH) && (s->toklen < BUFSIZE); nextchar(s))
			s->toktext[ s->toklen++ ] = curCH;
		s->toktext[s->toklen] = 0;
		return ( check_keyword(s, s->toktext) );
	}

	s->toktext[0] = c = curCH;
	nextchar(s);                      /* Always 1 char ahead */

	SECONDCHAR:

	s->toktext[1] = curCH;      /* If only 1 char needed we'll correct it */
	s->toktext[ s->toklen = 2 ] = 0;  /* Ditto */

#define checknext(c,val);  if (curCH == c) { nextchar(s); return (val); }

	/* Check if any of the standard comparison/logical operators */
	switch (c)
	{
		case '=':  checknext('=', EQ);   break;
		case '>':  checknext('=', GE);   break;
		case '<':  checknext('=', LE);   break;
		case '!':  checknext('=', NE);   break;
		case '|':  checknext('|', LOR);  break;
		case '&':  checknext('&', LAND); break;
	}
	
#undef checknext

	s->toktext[ s->toklen = 1 ] = 0;         /* Correct the above */
	return (c);

#undef curCH
#undef nextchar
}


/* Returns # line
*/
int mal_scan_get_position(void *ss, int *linepos)
{
	if (linepos != NULL)
		*linepos = ((scanner_t *) ss)->linepos;
	return ( ((scanner_t *) ss)->currline );
}


char *mal_scan_get_toktext(void *ss)
{
	return ( ((scanner_t *) ss)->toktext );
}


int mal_scan_get_toklen(void *ss)
{
	return ( ((scanner_t *) ss)->toklen );
}


int mal_scan_get_intval(void *ss)
{
	return ( ((scanner_t *) ss)->tokival );
}
