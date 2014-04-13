#include <stdio.h>
#include <stdlib.h>
#include "lispcalc.h"
#include "symbol.h"
#include "util.h"
#include "builtins.h"

static int addtwo(struct value *val, const struct value *addme);

/* this adds an arbitrary number of values together,
 * regardless of them being ints/floats
 * if one or more of the values are floats the result is a float
 * and otherwise it is an int */
struct retrn*
builtin_add(struct args *args)
{
	struct value *val;

	val = xmalloc(sizeof(struct value));

	val->valtype = args->val.valtype;
	if(val->valtype == ival)
		val->ival = args->val.ival;

	else if(val->valtype == fval)
		val->fval = args->val.fval;

	/* what the shit this guy is trying to add a function to a string
	 * or something */
	else goto add_typeerr;

	/* add ALL the arguments */
	for(args = args->next; args; args = args->next){
		/* the retard is at it again */
		if(!addtwo(val, &args->val))
			goto add_typeerr;
	}

	/* return with great success */
	return mkretrn(1, val);

	/* oh noes, someone tried printf+"potato" */
add_typeerr:
	free(val);
	return mkretrn(1, genericerror(err_type,
				"adding those make no sense"));
}

/* helper for builtin_add, val+addme -> val, with conversion if necessary */
static int
addtwo(struct value *val, const struct value *addme)
{
	/* make sure the types match */
	if(val->valtype != addme->valtype){
		/* first off make sure we are not trying to do something
		 * utterly retarded */
		if(!NUMERIC(addme))
			return 0;

		/* now we can work with the assumption that
		 * t(val) ^ t(addme) == 1,
		 * which means we convert the int to float */
		else if(val->valtype == ival){
			val->valtype = fval;
			val->fval = (double)val->ival;	/* is this safe? */
			val->fval += addme->fval;	/* addition complete */
		}
		else val->fval += (double)addme->ival;
		return 1;
	}

	if(val->valtype == ival)
		val->ival += addme->ival;

	else val->fval += addme->fval;

	return 1;
}

/* dumps the symbol table to stdout, always returns NULL */
struct retrn*
dump_symtbl(struct args* args)
{
	struct symbol *sym;

	puts("--- START OF SYMBOL TABLE DUMP");
	for(sym = symtbl; sym; sym = sym->sym_next){
		printf("\"%s\" = ", sym->name);
		printval(&sym->val);
	}
	puts("--- END OF SYMBOL TABLE DUMP");

	return NULL;
}

