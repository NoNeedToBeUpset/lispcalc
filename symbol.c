#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "lispcalc.h"
#include "util.h"
#include "builtins.h"

struct symbol*
findsymbol(const char *symname)
{
	struct symbol *sym;

	for(sym = symtbl; sym; sym = sym->sym_next){
		if(!strcmp(symname, sym->name))
			break;
	}

	/* either we found it or it is NULL, either makes this function
	 * happy */
	return sym;
}

struct symbol*
getlastsymbol(void)
{
	struct symbol *sym;

	for(sym = symtbl; ; sym = sym->sym_next)
		if(sym->sym_next == NULL)
			return sym;

	DEVELOPERPLS("getlastsymbol");
	return NULL;
}

struct symbol*
setsymbol(const char *symname, struct value *val)
{
	struct symbol *sym;

	/* search for a matching symbol name */
	sym = findsymbol(symname);

	/* if there isn't one, add a new to the end of the list */
	if(!sym){
		sym = getlastsymbol();
		sym->sym_next = xmalloc(sizeof(struct symbol));
		sym = sym->sym_next;

		/* set the name, ensure list integrity and move along as
		 * if nothing happened */
		sym->name = allocstring(symname);
		sym->sym_next = NULL;
	}
	/* if a symbol existed, clear the old value */
	else freeval(&sym->val);

	setval(&sym->val, val);
	return sym;
}

void
setval(struct value *dst, const struct value *src)
{
	dst->valtype = src->valtype;

	switch(src->valtype){
	case ival:
		dst->ival = src->ival;
		return;
	case fval:
		dst->fval = src->fval;
		return;
	case sval:
		dst->sval = xmalloc(strlen(src->sval) + 1);
		strcpy(dst->sval, src->sval);
		return;
	case funval:
		dst->funval = xmalloc(sizeof(struct funspec));
		dst->funval->funtype = src->funval->funtype;
		if(src->funval->funtype == fun_builtin)
			dst->funval->builtin = src->funval->builtin;
		return;
	case errval:
		puts("TODO: fix setval for errval");
		return;
	case invalidval:
		DEVELOPERPLS("setval on invalidval");
	}
}

struct value*
mkfloat(double f)
{
	struct value *v;

	v = xmalloc(sizeof(struct value));
	v->valtype = fval;
	v->fval = f;

	return v;
}

struct value*
mkint(int i)
{
	struct value *v;

	v = xmalloc(sizeof(struct value));
	v->valtype = ival;
	v->ival = i;

	return v;
}

struct value*
mkstr(const char *s)
{
	struct value *v;

	v = xmalloc(sizeof(struct value));
	v->valtype = sval;
	v->sval = xmalloc(strlen(s) + 1);
	strcpy(v->sval, s);

	return v;
}
