#ifndef __H_BUILTINS
#define __H_BUILTINS

#include "lispcalc.h"

#define NUMERIC(val) ((val)->valtype == ival || (val)->valtype == fval)

struct retrn* builtin_add(struct args *args);
struct retrn* builtin_dump_symtbl(struct args *args);
struct retrn* builtin_println(struct args *args);

#endif	/* __H_BUILTINS */
