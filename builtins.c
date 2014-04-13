#include <stdio.h>
#include <stdlib.h>
#include "lispcalc.h"
#include "symbol.h"
#include "util.h"
#include "builtins.h"

static int _add(struct value *val, const struct value *addme);
static int _div(struct value *v1, const struct value *v2);
static int _mul(struct value *v1, const struct value *v2);
static int _sub(struct value *v1, const struct value *v2);

static int reduce(struct value *val, const struct value *val2,
		int(*reducefunc)(struct value*, const struct value*));

/* reduce two values to one, using the arithmetic function reducefunc
 * the cool thing: makes sure the types are compatible */
static int
reduce(struct value *val, const struct value *val2,
	int(*reducefunc)(struct value*, const struct value*))
{
	int retv;
	struct value *fv = NULL;

	if(val->valtype != val2->valtype){
		/* first off make sure we are not trying to do something
		 * utterly retarded */
		if(!NUMERIC(val2))
			return 0;

		/* now we can work with the assumption that
		 * t(val) ^ t(val2) == 1,
		 * which means we convert the int to float */
		if(val->valtype == ival){
			val->valtype = fval;
			val->fval = (double)val->ival;
		}
		else if(val2->valtype == ival){
			fv = xmalloc(sizeof(struct value));
			fv->valtype = fval;
			fv->fval = (double)val2->ival;
		}
		else DEVELOPERPLS("THE TYPES ARE MESSED UP");

	}
	retv = reducefunc(val, fv ? fv : val2);

	/* clean up if we allocated memory */
	if(fv)
		free(fv);

	return retv;
}

/* this adds an arbitrary number of values together,
 * regardless of them being ints/floats
 * if one or more of the values are floats the result is a float
 * and otherwise it is an int */
struct retrn*
builtin_add(struct args *args)
{
	struct value *val;

	val = xmalloc(sizeof(struct value));

	/* start with a known good value (also the empty sum, which is neat)
	 * this will be converted to float if needed */
	val->valtype = ival;
	val->ival = 0;

	/* add ALL the arguments */
	for(; args; args = args->next){
		/* the retard is at it again */
		if(!reduce(val, &args->val, _add)){
			free(val);
			return mkretrn(1, genericerror(err_type,
					"adding those make no sense"));
		}
	}

	/* return with great success */
	return mkretrn(1, val);
}

struct retrn*
builtin_div(struct args *args)
{
	struct value *val;

	/* division only operates on doubles */
	val = xmalloc(sizeof(struct value));
	val->valtype = fval;
	if(args->val.valtype == fval)
		val->fval = args->val.fval;

	else if(args->val.valtype == ival)
		val->fval = (double)args->val.ival;

	else goto div_typeerr;

	for(args = args->next; args; args = args->next){
		if(!reduce(val, &args->val, _div))
			goto div_typeerr;
	}

	return mkretrn(1, val);

div_typeerr:
	free(val);
	return mkretrn(1, genericerror(err_type,
			"that makes no sense"));
}

struct retrn*
builtin_mul(struct args *args)
{
	struct value *val;

	/* start with a known good value, the empty product */
	val = xmalloc(sizeof(struct value));
	val->valtype = ival;
	val->ival = 1;

	for(; args; args = args->next){
		if(!reduce(val, &args->val, _mul)){
			free(val);
			return mkretrn(1, genericerror(err_type,
					"multiplying those makes no sense"));
		}
	}

	return mkretrn(1, val);
}

struct retrn*
builtin_sub(struct args *args)
{
	struct value *val;

	if(!NUMERIC(&args->val))
		goto sub_typeerror;

	val = xmalloc(sizeof(struct value));
	setval(val, &args->val);

	for(args = args->next; args; args = args->next){
		if(!reduce(val, &args->val, _sub))
			goto sub_typeerror;
	}

	return mkretrn(1, val);

sub_typeerror:
	free(val);
	return mkretrn(1, genericerror(err_type,
			"that subtraction makes no sense"));
}

/* to be passed as reducefunc for reduce
 * we can make lots of assumptions, reduce fixed everything */
static int
_add(struct value *val, const struct value *addme)
{
	if(val->valtype == ival)
		val->ival += addme->ival;

	else val->fval += addme->fval;

	return 1;
}

static int
_div(struct value *v1, const struct value *v2)
{
	/* div is only done with floats */
	v1->fval /= v2->fval;

	return 1;
}

static int
_mul(struct value *v1, const struct value *v2)
{
	if(v1->valtype == ival)
		v1->ival *= v2->ival;

	else v1->fval *= v2->fval;

	return 1;
}

static int
_sub(struct value *v1, const struct value *v2)
{
	if(v1->valtype == ival)
		v1->ival -= v2->ival;

	else v1->fval -= v2->fval;

	return 1;
}

/* dumps the symbol table to stdout, always returns NULL */
struct retrn*
builtin_dump_symtbl(struct args* args)
{
	char *s;
	struct symbol *sym;

	puts("--- START OF SYMBOL TABLE DUMP");
	for(sym = symtbl; sym; sym = sym->sym_next){
		s = stringify(&sym->val);
		printf("\"%s\" = %s\n", sym->name, s);
		free(s);
	}
	puts("--- END OF SYMBOL TABLE DUMP");

	return NULL;
}

struct retrn*
builtin_println(struct args *args)
{
	char *s;

	/* no arguments -> just newline */
	if(!args){
		puts("");
		return NULL;
	}

	for(; args; args = args->next){
		s = stringify(&args->val);
		printf("%s", s);
		free(s);
	}

	puts("");
	return NULL;
}
