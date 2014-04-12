#ifndef __H_LISPCALC
#define __H_LISPCALC

/* needed for macros */
#include <ctype.h>

#include "types.h"

/* a macro to let ptr skip over whitespace in a uniform fashion */
#define SKIPWS(ptr) while(isspace(*ptr)){ptr++;}

/* symbol names must start with alpha, underscore or one of the
 * four basic arithmetic operators */
#define ISSYMFIRSTCHAR(chr) (isalpha(chr) || (chr) == '_' || \
		(chr) == '+' || (chr) == '-' || (chr) == '/' || \
		(chr) == '*')

/* but are more liberal later */
#define ISSYMCHAR(chr) (ISSYMFIRSTCHAR(chr) || isdigit(chr) || \
		(chr) == '-' || (chr) == '+' || (chr) == '?' || \
		(chr) == '/' || (chr) == '%' || (chr) == '!')

/* needed for struct args, which makes most sense to define here */
#include "symbol.h"

/* a macro to be called in places where you could theoretically end up
 * but never actually should */
#define DEVELOPERPLS(where) \
	do{ \
		fprintf(stderr, "stupid developer fucked something up " \
			" at %s (%s:%d)\n", where, __FILE__, __LINE__); \
		abort(); } while(0)

/* Definitions! We love to #define things */
#define PROGRAM	"lispcalc"
#define VERSION	"0.1"

#endif /* __H_LISPCALC */
