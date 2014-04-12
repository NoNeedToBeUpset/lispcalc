#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lispcalc.h"
#include "symbol.h"
#include "util.h"

char*
allocstring(const char *p)
{
	char *s;

	s = xmalloc(strlen(p) + 1);
	strcpy(s, p);

	return s;
}

void
die(const char *err)
{
	perror(err);
	exit(EXIT_FAILURE);
}

char*
errstr(errtype_t et){
	switch(et){
	case err_noerr:
		return "no error";
	case err_symresolve:
		return "unable to resolve symbol";
	case err_syntax:
		return "syntax error";
	case err_call:
		return "calling error";
	case err_type:
		return "invalid type";
	default:
		return "unknown error wtf kill the dev";
	}
}

/* DOES NOT free the actual err struct, only the fields within it */
void
freeerr(struct error *err)
{
	if(!err)
		return;

	free(err->reason);
}

/* DOES free the sym */
void
freesym(struct symbol *sym)
{
	if(!sym)
		return;

	freeval(&sym->val);

	if(sym->name)
		free(sym->name);

	free(sym);
}

/* DOES NOT free the actual val struct, only the fields within it */
void
freeval(struct value *val)
{
	if(!val)
		return;

	if(val->valtype == errval)
		freeerr(val->errval);

	else if(val->valtype == sval)
		free(val->sval);
}

/* create an error struct with errtype type and specified reason */
struct value*
genericerror(errtype_t type, const char *reason)
{
	struct error *e;
	struct value *v;

	e = xmalloc(sizeof(struct error));

	e->errtype = type;
	e->reason = xmalloc(strlen(reason) + 1);
	strcpy(e->reason, reason);

	v = xmalloc(sizeof(struct value));
	v->valtype = errval;
	v->errval = e;

	return v;
}

/* read a string from the buffer b, copy it into malloc'd
 * memory and return that
 * b should point at the initial " */
char*
getstring(char *b)
{
	char *s;
	int i;

	/* make sure it is actually a string */
	if(*b++ != '"')
		return NULL;

	/* b now points to the first letter in the string
	 * figure out the string length, it ends on a non-escaped " */
	for(i = 0; !(b[i] == '"' && b[i-1] != '\\'); i++){
		/* but hitting null before " is an error */
		if(!b[i])
			return NULL;
	}

	s = xmalloc(i + 1);
	memcpy(b, s, i);
	s[i] = '\0';

	return s;
}

/* reads ahead as long as there are valid symbol name characters,
 * then copy that into a malloc'd string and return */
char*
getsymname(const char *b)
{
	char *nam;
	int i;

	if(!ISSYMFIRSTCHAR(*b))
		return NULL;

	for(i = 0; ISSYMCHAR(b[i]); i++) ;

	nam = xmalloc(i+1);
	memcpy(nam, b, i);
	nam[i] = 0;

	return nam;
}

/* check if what we find in p should be interpreted as a float or int */
int
hasfloat(const char *p)
{
	while(isdigit(*p++)){
		if(*p == '.' || *p == ',')
			return 1;
	}

	return 0;
}

/* create a retrn struct, specifying if val will need to be freed */
struct retrn*
mkretrn(int shouldfree, struct value *val)
{
	struct retrn *r;

	r = xmalloc(sizeof(struct retrn));
	r->shouldfree = shouldfree;
	r->val = val;

	return r;
}

/* print a value, what happens when you (symbol) */
void
printval(struct value *val)
{
	if(!val){
		puts("<null>");
		return;
	}
	switch(val->valtype){
	case errval:
		printf("%s: %s\n", errstr(val->errval->errtype),
				val->errval->reason);
		return;
	case ival:
		printf("%i\n", val->ival);
		return;
	case fval:
		printf("%lf\n", val->fval);
		return;
	case sval:
		printf("\"%s\"\n", val->sval);
		return;
	case funval:
		printf("<function>\n");
		return;
	default:
		puts("invalid value, wat");
	}
}

/* perlspeak: checks if $s ~= /^$m\s/ */
int
strmatches(const char *s, const char *m)
{
	int i, l;

	l = strlen(m);
	i = strncmp(s, m, l);
	if(i)
		return i;

	return !(isspace(s[l]) || s[l] == ')') ? 1 : 0;
}

/* symresolveerror is specific enough to want its own errorgenerator
 * what can point to either "symbolname" or "symbolname BLARASJFSLNF"
 * for all we care, practical when passing a buf here */
struct value*
symresolveerror(const char *what)
{
	struct error *e;
	struct value *val;

	e = xmalloc(sizeof(struct error));
	e->errtype = err_symresolve;
	e->reason = getsymname(what);

	val = xmalloc(sizeof(struct value));
	val->valtype = errval;
	val->errval = e;

	return val;
}

/* die with considerable noise if a malloc fails, if(...) is annoying when
 * you will just die() everywhere anyway */
void*
xmalloc(size_t sz)
{
	void *p;

	p = malloc(sz);
	if(!p)
		abort();

	return p;
}
