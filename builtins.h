#ifndef __H_BUILTINS
#define __H_BUILTINS

#include "lispcalc.h"

#define NUMERIC(val) ((val)->valtype == ival || (val)->valtype == fval)

/* output, debugging and data inspection */
struct retrn* builtin_dump_symtbl(struct args *args);
struct retrn* builtin_println(struct args *args);

/* arithmetic */
struct retrn* builtin_add(struct args *args);
struct retrn* builtin_div(struct args *args);
struct retrn* builtin_mul(struct args *args);
struct retrn* builtin_sub(struct args *args);

#endif	/* __H_BUILTINS */
