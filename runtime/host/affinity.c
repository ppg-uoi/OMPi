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

/* affinity.c -- implement OpenMP affinity format */

#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "ort_prive.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * GLOBAL VARIABLES / DEFINITIONS / MACROS                           *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define AFFINITY_FORMAT_TYPES     10
#define DEFAULT_AFFINITY_FORMAT   "Level: %L, TID: %n/%N, Affinity: %A"
#define j_format_error(n)         longjmp(j_formaterr, n)
#define OUTPUT_BUFFER_SIZE        1024
#define HOSTNAME_BUFFER_SIZE      128

enum {INT_VAL, STR_VAL};

static char *aftok;                 /* For parsing the affinity format string */
static jmp_buf j_formaterr;         /* Error handling */
static char *hostname = NULL;
static void *af_lock = NULL;        /* Protects access to aftok and j_formaterr */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * FORMAT VALIDATION                                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
void aff_format_error_handle(int n) {
	switch (n)
	{
		case 1:
			ort_warning("ran out of memory while parsing affinity format; "
					"falling back to previous value.\n");
			break;
		case 2:
			ort_warning("affinity format error: no field specifier exists; "
					"falling back to previous value.\n");
			break;
		case 3:
			ort_warning("affinity format error: size in field specifier cannot "
					"start with a zero; falling back to previous value.\n");
			break;
		case 4:
			ort_warning("affinity format error: invalid character in field "
					"specifier; falling back to previous value.\n");
			break;
		case 5:
			ort_warning("affinity format error: invalid long name; "
					"falling back to previous value.\n");
			break;
		case 6:
			ort_warning("affinity format error: missing '}' after long name; "
					"falling back to previous value.\n");
			break;
		case 7:
			ort_warning("affinity format error: invalid short name; "
					"falling back to previous value.\n");
			break;
		case 8:
			ort_warning("affinity format error: missing '.' after '0' modifier; "
					"falling back to previous value.\n");
			break;
		case 9:
			ort_warning("affinity format error: missing size in "
					"field specifier; falling back to previous value.\n");
			break;
		case 10:
			ort_warning("affinity format error: unexpected null-byte "
				"('\\0'); falling back to previous value.\n");
			break;
	}
}


/* type := short-name | {long-name}
 */
static
void parse_affinity_format_type(void)
{
	int i, valid_index = -1;
	char *longnames[AFFINITY_FORMAT_TYPES] = {"team_num", "num_teams", "nesting_level",
		"thread_num", "num_threads", "ancestor_tnum", "host",
		"process_id", "native_thread_id", "thread_affinity"};

	if (*aftok == '{')
	{
		aftok++; // consume '{'
		if (*aftok == '\0')
			j_format_error(10);
		for (i = 0; i < AFFINITY_FORMAT_TYPES; i++)
			if (strncmp(aftok, longnames[i], strlen(longnames[i])) == 0)
			{
				valid_index = i;
				break;
			}
		if (valid_index == -1)
			j_format_error(5);
		aftok += strlen(longnames[valid_index]);
		if (*aftok != '}')
			j_format_error(6);
		aftok++; // consume '}'
	}
	else
	{
		switch (*aftok) // to reduce dereferences
		{
			case 't':
			case 'T':
			case 'L':
			case 'n':
			case 'N':
			case 'a':
			case 'H':
			case 'P':
			case 'i':
			case 'A':
				aftok++;
				break;
			case '\0':
				j_format_error(10);
			default:
				j_format_error(7);
		}
	}
}


/* field := [size]type */
static
void parse_affinity_format_with_size(void)
{
	int num = atoi(aftok);
	if (*aftok == '0')
		j_format_error(3);
	if (num == 0)
		j_format_error(9);
	// consume decimal digits
	for (; isdigit(*aftok); aftok++)
		;
	parse_affinity_format_type();
}


/* field := [[.]size]type */
static
void parse_affinity_format_with_justification(void)
{
	if (*aftok != '.')
		j_format_error(8);
	aftok++; // consume '.'
	parse_affinity_format_with_size();
}


/* field := [[[0].]size]type */
static
void parse_affinity_format_with_padding(void)
{
	aftok++; // Consume '0'
	parse_affinity_format_with_justification();
}


/* field := %[[[0].]size]type */
static
void parse_affinity_format_field(void)
{
	int size;

	aftok++; // consume '%'

	if (*aftok == '0')
		parse_affinity_format_with_padding();
	else if (*aftok == '.')
		parse_affinity_format_with_justification();
	else if ('1' <= *aftok && *aftok <= '9')
		parse_affinity_format_with_size();
	else if (*aftok == '{' || isalpha(*aftok))
		parse_affinity_format_type();
	else if (*aftok == '\0')
		j_format_error(10);
	else
		j_format_error(4);
}


static
char *validate_affinity_format(const char* format)
{
	int   valid = 0, n;
	char *ret_format = NULL;

	if (!format)
	{
		ort_warning("validate_affinity_format(): format is NULL\n");
		return NULL;
	}

	if ((n = setjmp(j_formaterr)) != 0)
	{
		aff_format_error_handle(n);
		return NULL;
	}

	if ((aftok = ret_format = strdup(format)) == NULL)
		j_format_error(1);

	for ( ; *aftok; )
	{
		if (*aftok == '%')
		{
			if (*(aftok + 1) == '\0')
				j_format_error(10);
			else if (*(aftok + 1) == '%')   // Escaped '%'
				aftok += 2;
			else
			{
				parse_affinity_format_field();
				valid = 1; // Format has at least one field
			}
		}
		else
			aftok++;
	}

	if (!valid)
		j_format_error(2);

	// If this line is reached, the format has been successfully parsed.
	return ret_format;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * UTILITY FUNCTIONS                                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*
 * Copies src to dst according to the values of size and src,
 * in compliance to the OpenMP 5.0 specification.
 */
static
size_t buffer_copy(char *dst, size_t size, char *src, char *ompfunc)
{
	size_t len, outlen, i;

	len = strlen(src);
	if (size > 0)
	{
		if (dst == NULL)
		{
			ort_warning("%s: buffer is NULL\n", ompfunc);
			return len; // TODO return zero?
		}
		outlen = (len >= size) ? (size - 1) : (len);
		for (i = 0; i < outlen; i++)
			dst[i] = src[i];
		dst[i] = '\0';
	}

	return len;
}


static
char *get_hostname(void)
{
	ort_prepare_omp_lock(&af_lock, ORT_LOCK_NORMAL);
	ee_set_lock((ee_lock_t *) af_lock);

	if (hostname == NULL) // One-time initialization
	{
		char buffer[HOSTNAME_BUFFER_SIZE];

		if (gethostname(buffer, HOSTNAME_BUFFER_SIZE) == 0)
		{
			// null-terminate in case of a too long hostname
			buffer[HOSTNAME_BUFFER_SIZE - 1] = '\0';
			hostname = strdup(buffer); // Can be NULL
		}
	}

	ee_unset_lock((ee_lock_t *) af_lock);
	return hostname;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * AFFINITY FORMAT INTERPOLATION                                     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
size_t strcat_field(char *out_pp, size_t outbuff_size, char *field_str, size_t *bytes_written)
{
	char *fptr = field_str,
	     *int_format[2][2] = {{"%*d", "%-*d"},
		                  {"%0*d", "%0-*d"}},
	     *str_format[2] = {"%*s", "%-*s"},
	     *str_val = NULL;
	int pad = 0,
	    left_justif = 1,
	    size = 1,
	    ptype,
	    int_val;

	if (!out_pp || outbuff_size <= 1 || !field_str || !bytes_written)
		return 0;

	// Just after '%' in valid format field
	switch (*fptr)
	{
		case '0':
			pad = 1;
			fptr++;
		case '.':
			left_justif = 0;
			fptr++;
		default:
			if (isdigit(*fptr))
			{
				size = atoi(fptr);
				for (; isdigit(*fptr); fptr++)
					; // skip size digits
			}
	}

	ptype = INT_VAL; // Just some default value

	if (*fptr == '{')
	{
		fptr++;
		if (strncmp(fptr, "team_num", 8) == 0)
		{
			int_val = omp_get_team_num();
			fptr += 8;
		}
		else if (strncmp(fptr, "num_teams", 9) == 0)
		{
			int_val = omp_get_num_teams();
			fptr += 9;
		}
		else if (strncmp(fptr, "nesting_level", 13) == 0)
		{
			int_val = omp_get_level();
			fptr += 13;
		}
		else if (strncmp(fptr, "thread_num", 10) == 0)
		{
			int_val = omp_get_thread_num();
			fptr += 10;
		}
		else if (strncmp(fptr, "num_threads", 11) == 0)
		{
			int_val = omp_get_num_threads();
			fptr += 11;
		}
		else if (strncmp(fptr, "ancestor_tnum", 13) == 0)
		{
			int_val = omp_get_ancestor_thread_num(
					omp_get_level()-1);
			fptr += 13;
		}
		else if (strncmp(fptr, "host", 4) == 0)
		{
			ptype = STR_VAL;
			str_val = get_hostname();
			fptr += 4;
		}
		else if (strncmp(fptr, "process_id", 10) == 0)
		{
			int_val = getpid();
			fptr += 10;
		}
		else if (strncmp(fptr, "native_thread_id", 16) == 0)
		{
			if ((int_val = ee_getselfid()) < 0)
				ptype = STR_VAL;
			/* undefined */
			fptr += 16;
		}
		else if (strncmp(fptr, "thread_affinity", 15) == 0)
		{
			ptype = STR_VAL;
			str_val = places_get_list_str(__MYCB->pfrom, __MYCB->pto);
			fptr += 15;
		}
	}
	else
	{
		switch (*fptr)
		{
			case 't':
				int_val = omp_get_team_num();
				break;
			case 'T':
				int_val = omp_get_num_teams();
				break;
			case 'L':
				int_val = omp_get_level();
				break;
			case 'n':
				int_val = omp_get_thread_num();
				break;
			case 'N':
				int_val = omp_get_num_threads();
				break;
			case 'a':
				int_val = omp_get_ancestor_thread_num(
						omp_get_level()-1);
				break;
			case 'H':
				ptype = STR_VAL;
				str_val = get_hostname();
				break;
			case 'P':
				int_val = getpid();
				break;
			case 'i':
				if ((int_val = ee_getselfid()) < 0)
					ptype = STR_VAL;
				/* undefined */
				break;
			case 'A':
				ptype = STR_VAL;
				str_val = places_get_list_str(__MYCB->pfrom, __MYCB->pto);
				break;
		}
	}
	fptr++;   // for '}' or shortname

	if (ptype == INT_VAL) // Append to output buffer and advance related ptr
	{
		*bytes_written = snprintf(out_pp, outbuff_size,
				int_format[pad][left_justif], size, int_val);
	}
	else
	{
		int freestr = 1;
		if (str_val == NULL)
		{
			str_val = "undefined";
			freestr = 0;
		}
		*bytes_written = snprintf(out_pp, outbuff_size,
				str_format[left_justif], size, str_val);
		if (freestr && str_val != get_hostname())
			free(str_val);
	}
	return (fptr - field_str); // Bytes of valid format to skip
}


static
char *ort_get_display_affinity_str(char *valid_format)
{
	char  outbuff[OUTPUT_BUFFER_SIZE],
	     *out_ptr = outbuff;
	size_t vi, vflen, outbuff_size,
	       bw; // Bytes written in outbuff during last strcat_field call

	if (!valid_format)
	{
		ort_warning("ort_get_display_affinity_str(): valid_format is NULL\n");
		return NULL;
	}

	vflen = strlen(valid_format);

	for (vi = 0; vi < vflen; )
	{
		*(out_ptr++) = valid_format[vi++];
		if (valid_format[vi-1] == '%')
		{
			if (valid_format[vi] != '%')   // Escaped '%'
			{
				--out_ptr; // To overwrite '%'
				outbuff_size = OUTPUT_BUFFER_SIZE - (out_ptr - outbuff);
				vi += strcat_field(out_ptr, outbuff_size,
						valid_format + vi, &bw);
				out_ptr += bw;
			}
			else
				vi++;
		}
	}
	*out_ptr = '\0';
	return strdup(outbuff);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 * AFFINITY FORMAT API                                               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


char *ort_get_default_affinity_format(void)
{
	return DEFAULT_AFFINITY_FORMAT;
}


void ort_set_affinity_format(const char* format)
{
	char *valid_fmt,
	     *prev_fmt = ort->icvs.affinity_format;

	if (!format)
	{
		ort_warning("omp_set_affinity_format(): format is NULL\n");
		return;
	}

	ort_prepare_omp_lock(&af_lock, ORT_LOCK_NORMAL);
	ee_set_lock((ee_lock_t *) af_lock);

	valid_fmt = validate_affinity_format(format);

	ee_unset_lock((ee_lock_t *) af_lock);

	if (valid_fmt != NULL)
	{
		// Free old format (if needed) and replace it.
		if (prev_fmt != DEFAULT_AFFINITY_FORMAT)
			free(prev_fmt);
		ort->icvs.affinity_format = valid_fmt;
	}
}


size_t ort_get_affinity_format(char *buffer, size_t size)
{
	char *format;

	format = ort->icvs.affinity_format;

	return buffer_copy(buffer, size, format, "omp_get_affinity_format()");
}


void ort_display_affinity(const char *format)
{
	char *valid_fmt, *tmp_fmt, *output_str;

	valid_fmt = ort->icvs.affinity_format;
	if (format != NULL)
	{
		ort_prepare_omp_lock(&af_lock, ORT_LOCK_NORMAL);
		ee_set_lock((ee_lock_t *) af_lock);

		tmp_fmt = validate_affinity_format(format);

		ee_unset_lock((ee_lock_t *) af_lock);

		if (tmp_fmt != NULL)
			valid_fmt = tmp_fmt;
	}

	if ((output_str = ort_get_display_affinity_str(valid_fmt)) != NULL)
		printf("%s\n", output_str);

	if (valid_fmt != ort->icvs.affinity_format)
		free(valid_fmt); // valid_fmt == tmp_fmt
	free(output_str);

}


size_t ort_capture_affinity(char *buffer, size_t size, const char *format)
{
	size_t len = 0;  // Affinity str length excluding '\0'
	char *valid_fmt, *tmp_fmt, *output_str;

	valid_fmt = ort->icvs.affinity_format;
	if (format != NULL)
	{
		ort_prepare_omp_lock(&af_lock, ORT_LOCK_NORMAL);
		ee_set_lock((ee_lock_t *) af_lock);

		tmp_fmt = validate_affinity_format(format);

		ee_unset_lock((ee_lock_t *) af_lock);

		if (tmp_fmt != NULL)
			valid_fmt = tmp_fmt;
	}

	if ((output_str = ort_get_display_affinity_str(valid_fmt)) != NULL)
		len = buffer_copy(buffer, size, output_str, "omp_capture_affinity_format()");

	if (valid_fmt != ort->icvs.affinity_format)
		free(valid_fmt); // valid_fmt == tmp_fmt
	free(output_str);

	return len;
}

