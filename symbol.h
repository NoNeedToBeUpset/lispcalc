#ifndef __H_SYMBOL
#define __H_SYMBOL

#include "lispcalc.h"

void setval(struct value *dst, const struct value *src);

struct symbol* findsymbol(const char *symname);
struct symbol* getlastsymbol(void);
struct symbol* setsymbol(const char *symname, struct value *val);

struct value* mkint(int i);
struct value* mkfloat(double f);
struct value* mkstr(const char *s);

void freeerr(struct error *err);
void freesym(struct symbol *sym);
void freeval(struct value *val);

/* actual data defined in main.c */
extern struct symbol *symtbl;

#endif	/* __H_SYMBOL */
