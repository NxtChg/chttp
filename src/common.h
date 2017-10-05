/*=============================================================================
  Created by NxtChg (admin@nxtchg.com), 2017. License: Public Domain.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <winsock2.h>

//#pragma comment(lib, "ws2_32")

#ifndef snprintf
 #define snprintf _snprintf
#endif

#define null NULL

#define till(a)   for(int i = 0, __cnt = a; i < __cnt; i++)
#define loop(a,b) for(int a = 0, __cnt = b; a < __cnt; a++)

#define dimof(a) (sizeof(a)/sizeof(*a))

typedef unsigned char  byte;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef wchar_t        wchar;

#define Kb *1024
#define Mb *1024 Kb
#define Gb *1024 Mb
//_____________________________________________________________________________

char* skip_spaces(char *str, bool and_lines = false)
{
	if(and_lines) while(*str >   0  && *str <= ' ' ) str++;
	else          while(*str == ' ' || *str == '\t') str++;

	return str;
}//____________________________________________________________________________

bool str_equal(const char *a, const char *b, int n = 0xFFFFF)
{
	if(!a || !b) return false;

	for(int i = 0; i != n; i++)
	{
		if(a[i] != b[i]) return false;

		if(!a[i]) return true;
	}

	return true;
}//____________________________________________________________________________

bool str_equal_nc(const char *a, const char *b, int n = 0xFFFFF) // case-insensitive
{
	if(!a || !b) return false;

	for(int i = 0; i != n; i++)
	{
		if(tolower(a[i]) != tolower(b[i])) return false;

		if(!a[i]) return true;
	}

	return true;
}//____________________________________________________________________________

inline int str_copy(char *dst, const char *src, int limit = 0xFFFFF)
{
	if(limit > 0) limit--; else{ *dst = 0; return 0; }

	const char *end = dst + limit;

	while(dst < end && (*dst = *src)){ dst++; src++; } *dst = 0;

	return (limit + dst - end);
}//____________________________________________________________________________

char* str_char(char *haystack, char *needles) // returns the end of the string if not found
{
	for(char *p = haystack; *p; p++)
	{
		char c = *p;

		char *n = needles; while(*n && *n != c) n++;

		if(*n == c) break;
	}

	return p;
}//____________________________________________________________________________

char* str_read(char *start, char *dst, int limit, char *div = ",") // trims spaces and lines
{
	if(!start) return null;

	start = skip_spaces(start, true); if(!*start){ *dst = 0; return null; } // trim left

	char *end = str_char(start, div);

	int len = end - start + 1; if(len > limit) len = limit;

	len = str_copy(dst, start, len);

	while(len > 0 && dst[len-1] > 0 && dst[len-1] <= ' ') dst[--len] = 0; // trim right

	return (*end ? skip_spaces(end+1, true) : end);
}
