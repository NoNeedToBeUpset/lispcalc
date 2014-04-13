#ifndef __H_INITSYM
#define __H_INITSYM

#include "lispcalc.h"
#include "types.h"

/* a few macros to make it easier to specify initsyms */
#define BUILTIN_FUNSYM(k,fun) \
	{ .name = k, .val = { .valtype = funval, \
		.funval = &(struct funspec) { \
			.funtype = fun_builtin, \
			.builtin = fun}}}

#define FLOATSYM(k,x) \
	{ .name = k, .val = { .valtype = fval, .fval = x}}

#define STRSYM(k,s) \
	{ .name = k, .val = { .valtype = sval, .sval = s}}

/* this is where all the initial symbols are defined, they will
 * later be copied to symtbl to be arranged as a linked list */
static struct symbol initsyms[] = {
	/* program-related info */
	STRSYM("PROGRAM", PROGRAM),
	STRSYM("VERSION", VERSION),
	/* mathematical constants */
	FLOATSYM("e", M_E),
	FLOATSYM("pi", M_PI),
	/* builtin functions */
	BUILTIN_FUNSYM("dump", builtin_dump_symtbl),
	BUILTIN_FUNSYM("println", builtin_println),
	BUILTIN_FUNSYM("+", builtin_add),
	BUILTIN_FUNSYM("-", builtin_sub),
	BUILTIN_FUNSYM("*", builtin_mul),
	BUILTIN_FUNSYM("/", builtin_div),
};

#endif	/* __H_INITSYM */
