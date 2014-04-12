#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "lispcalc.h"
#include "util.h"

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

	memcpy(&sym->val, val, sizeof(struct value));
	return sym;
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
