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

/* MAL_SCAN.H -- Simple re-entrant scanner for MAL */

#ifndef __MALSCAN_H__
#define __MALSCAN_H__
 
#include <stdio.h>

/* Max allowable token size */
#define BUFSIZE     1024

/* Standard tokens returned from scanner_nexttoken() */
#define EQ       500   /* == */
#define GE       501   /* >= */
#define LE       502   /* <= */
#define NE       503   /* != */
#define LT       '<'
#define GT       '>'
#define LOR      512   /* || */
#define LAND     513   /* && */
#define S_WORD   550
#define S_INT    551
#define S_STRING 552
#define S_STREOF 553   /* Error - unterminated string */
#define S_STRBIG 554   /* Error - string too big */

/* Keywords */
#define S_FLAVORS 599
#define S_NODES   600
#define S_TRUE    601
#define S_FALSE   602
#define S_HAS     603
#define S_HASONLY 604
#define S_NUM     605

/* Opaque type for scanner state
 */
typedef void *scanif_t;
extern scanif_t mal_scan_file_init(FILE *fp);         /* Scan a file */
extern scanif_t mal_scan_string_init(char *string);   /* Scan a string */
extern scanif_t mal_scan_string_reinit(scanif_t *st, char *string);
extern void     mal_scan_done(scanif_t *st);          /* Cleanup at the end */

/* Main scanner function.
 * Returns:
 * ========
 *   EOF       on EOF
 *   a char    if single char read
 *   S_INT     if an integer was read
 *   S_WORD    if a word was read
 *   any of the given keywords if the corresponding text was read
 */
extern int    mal_scan_nexttoken(scanif_t s);

/* Get running info
 */
extern int    mal_scan_get_position(scanif_t s, int *linepos); /* Ret: #line */
extern char  *mal_scan_get_toktext(scanif_t s);
extern int    mal_scan_get_toklen(scanif_t s);
extern int    mal_scan_get_intval(scanif_t s);   /* also weekday/month value */

#endif  /* __MALSCAN_H__  */
