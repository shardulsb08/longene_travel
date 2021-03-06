/*
 * Utility routines
 *
 * Copyright 1998,2000 Bertho A. Stultiens
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "wmctypes.h"
#include "utils.h"
#include "wmc.h"

#define SUPPRESS_YACC_ERROR_MESSAGE

static void generic_msg(const char *s, const char *t, va_list ap)
{
	fprintf(stderr, "%s:%d:%d: %s: ", input_name ? input_name : "stdin", line_number, char_number, t);
	vfprintf(stderr, s, ap);
}

/*
 * The yyerror routine should not exit because we use the error-token
 * to determine the syntactic error in the source. However, YACC
 * uses the same routine to print an error just before the error
 * token is reduced.
 * The extra routine 'xyyerror' is used to exit after giving a real
 * message.
 */
int mcy_error(const char *s, ...)
{
#ifndef SUPPRESS_YACC_ERROR_MESSAGE
	va_list ap;
	va_start(ap, s);
	generic_msg(s, "Yacc error", ap);
	va_end(ap);
#endif
	return 1;
}

int xyyerror(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	generic_msg(s, "Error", ap);
	va_end(ap);
	exit(1);
	return 1;
}

int mcy_warning(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	generic_msg(s, "Warning", ap);
	va_end(ap);
	return 0;
}

void internal_error(const char *file, int line, const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "Internal error (please report) %s %d: ", file, line);
	vfprintf(stderr, s, ap);
	va_end(ap);
	exit(3);
}

void error(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, s, ap);
	va_end(ap);
	exit(2);
}

void warning(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "Warning: ");
	vfprintf(stderr, s, ap);
	va_end(ap);
}

char *dup_basename(const char *name, const char *ext)
{
	int namelen;
	int extlen = strlen(ext);
	char *base;
	char *slash;

	if(!name)
		name = "wmc.tab";

	slash = strrchr(name, '/');
	if (slash)
		name = slash + 1;

	namelen = strlen(name);

	/* +4 for later extension and +1 for '\0' */
	base = xmalloc(namelen +4 +1);
	strcpy(base, name);
	if(!strcasecmp(name + namelen-extlen, ext))
	{
		base[namelen - extlen] = '\0';
	}
	return base;
}

void *xmalloc(size_t size)
{
    void *res;

    assert(size > 0);
    assert(size < 102400);
    res = malloc(size);
    if(res == NULL)
    {
	error("Virtual memory exhausted.\n");
    }
    memset(res, 0x55, size);
    return res;
}


void *xrealloc(void *p, size_t size)
{
    void *res;

    assert(size > 0);
    assert(size < 102400);
    res = realloc(p, size);
    if(res == NULL)
    {
	error("Virtual memory exhausted.\n");
    }
    return res;
}

char *xstrdup(const char *str)
{
	char *s;

	assert(str != NULL);
	s = xmalloc(strlen(str)+1);
	return strcpy(s, str);
}

int unistrlen(const WCHAR *s)
{
	int n;
	for(n = 0; *s; n++, s++)
		;
	return n;
}

WCHAR *unistrcpy(WCHAR *dst, const WCHAR *src)
{
	WCHAR *t = dst;
	while(*src)
		*t++ = *src++;
	*t = 0;
	return dst;
}

WCHAR *xunistrdup(const WCHAR * str)
{
	WCHAR *s;

	assert(str != NULL);
	s = xmalloc((unistrlen(str)+1) * sizeof(WCHAR));
	return unistrcpy(s, str);
}

int unistricmp(const WCHAR *s1, const WCHAR *s2)
{
	int i;
	int once = 0;
	static const char warn[] = "Don't know the uppercase equivalent of non acsii characters;"
	       		     "comparison might yield wrong results";
	while(*s1 && *s2)
	{
		if((*s1 & 0xffff) > 0x7f || (*s2 & 0xffff) > 0x7f)
		{
			if(!once)
			{
				once++;
				mcy_warning(warn);
			}
			i = *s1++ - *s2++;
		}
		else
			i = toupper(*s1++) - toupper(*s2++);
		if(i)
			return i;
	}

	if((*s1 & 0xffff) > 0x7f || (*s2 & 0xffff) > 0x7f)
	{
		if(!once)
			mcy_warning(warn);
		return *s1 - *s2;
	}
	else
		return 	toupper(*s1) - toupper(*s2);
}

int unistrcmp(const WCHAR *s1, const WCHAR *s2)
{
	int i;
	while(*s1 && *s2)
	{
		i = *s1++ - *s2++;
		if(i)
			return i;
	}

	return *s1 - *s2;
}
