#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "lispcalc.h"
#include "symbol.h"
#include "util.h"
#include "builtins.h"

#include "initsym.h"

struct symbol *symtbl;

static void init_symtbl(void);
static int lispcalc(int fd);
static struct retrn* process(char *buf);

int
main(int argc, char **argv)
{
	init_symtbl();

	return lispcalc(STDIN_FILENO);
}

/* initialize the symbol table, initial contents:
 * PROGRAM - program name
 * VERSION - lispcalc version */
static void
init_symtbl(void)
{
	int i;

	/* the first one takes special treatment */
	symtbl = xmalloc(sizeof(struct symbol));
	symtbl->sym_next = NULL;

	symtbl->name = initsyms[0].name;
	setval(&symtbl->val, &initsyms[0].val);

	/* then we can just repeat the same recipe */
	for(i = 1; i < sizeof(initsyms)/sizeof(initsyms[0]); i++)
		setsymbol(initsyms[i].name, &initsyms[i].val);
}

static int
lispcalc(int fd)
{
	char *inbuf, *p;
	int i, inbufsz;
	struct retrn *retrn;

	/* get a 64K buffer */
	inbufsz = 64 * 1024;
	inbuf = xmalloc(inbufsz);

	for(;;){
		if(isatty(STDOUT_FILENO)){
			i = write(STDOUT_FILENO, "> ", 2);
			if(i < 0)
				die("write");
		}

		i = read(fd, inbuf, inbufsz-1);
		if(i < 0){
			/* if EINTR, reason was probably ^c */
			if(errno == EINTR)
				continue;
			die("read");
		}

		if(i == 0){
			if(isatty(STDOUT_FILENO))
				puts("eof");
			return 0;
		}

		/* ensure NULL termination */
		inbuf[i] = 0;
		p = inbuf;
		while(*p){
			retrn = process(p);

			/* process should have processed 1 statement, skip that
			 * before the next loop */
			skipitem(&p);
			SKIPWS(p);

			/* loop again if the statement didn't return anything */
			if(retrn == NULL)
				continue;

			/* otherwise display and clear return value */
			printval(retrn->val);
			if(retrn->shouldfree){
				freeval(retrn->val);
				free(retrn->val);
				free(retrn);
			}
		}
	}
}

static struct retrn*
process(char *buf)
{
	char *symnam;
	struct retrn *retv;
	struct symbol *sym;
	struct value *val;

	SKIPWS(buf);

	/* return immidiately on an empty string */
	if(!*buf)
		return NULL;

	else if(*buf != '('){
		/* handle numeric types */
		if(isdigit(*buf) || *buf == '-' || *buf == '+'){
			/* either we detect float or assume int */
			if(hasfloat(buf)){
				double d = strtod(buf, NULL);
				return mkretrn(1, mkfloat(d));
			}
			else {
				int i = strtol(buf, NULL, 0);
				return mkretrn(1, mkint(i));
			}
		}
		/* strings start with " */
		else if(*buf == '"'){
			val = xmalloc(sizeof(struct value));
			val->valtype = sval;
			val->sval = getstring(buf);
			if(val->sval == NULL){
				free(val);
				return mkretrn(1, genericerror(err_syntax,
					"invalid string specification"));
			}
			return mkretrn(1, val);
		}
		/* else we believe we have a symbol name */
		else {
			char* symnam = getsymname(buf);
			sym = findsymbol(symnam);
			free(symnam);
			return mkretrn(0, &sym->val);
		}
		DEVELOPERPLS("something unexpected turned up");
	}

	/* skip over the leading (, which we will always have here,
	 * and then look for something very special
	 * note: I could probably implement define as the other builtins,
	 * i.e. place in symtbl */
	else if(!strmatches(++buf, "define")){
		/* first skip forward to new sym name */
		buf += 6;
		SKIPWS(buf);

		/* extract symbol name and skip forward to the value */
		symnam = getsymname(buf);
		while(ISSYMCHAR(*buf))
			buf++;

		SKIPWS(buf);

		/* make use of process()'s recursive capability */
		retv = process(buf);

		setsymbol(symnam, retv->val);

		if(retv->shouldfree){
			freeval(retv->val);
			free(retv->val);
			free(retv);
		}
	}
	/* not a number, string or built-in special? search for a symbol */
	else {
		struct args *args;

		symnam = getsymname(buf);

		for(sym = symtbl; sym != NULL; sym = sym->sym_next){
			if(!strcmp(symnam, sym->name))
				break;
		}

		if(!sym)
			return mkretrn(1, symresolveerror(buf));

		/* we are going to call this so it has to be a function */
		else if(sym->val.valtype != funval)
			return mkretrn(1, genericerror(err_type,
							"can't call that"));

		/* skip over the symbol name and free it */
		buf += strlen(symnam);
		free(symnam);

		/* parse arguments
		 * could we do lazy evaluation? that'd be cool
		 * later maybe */
		SKIPWS(buf);
		args = NULL;
		while(*buf && *buf != ')'){
			struct args *lastarg;

			retv = process(buf);

			if(retv == NULL)
				break;

			if(!args){
				args = xmalloc(sizeof(struct args));
				lastarg = args;
			}
			else {
				lastarg->next = xmalloc(sizeof(struct args));
				lastarg = lastarg->next;
			}

			lastarg->next = NULL;
			setval(&lastarg->val, retv->val);

			skipitem(&buf);
			SKIPWS(buf);
		}

		if(*buf != ')'){
			freeargs(args);
			return mkretrn(1, genericerror(err_syntax,
						"expected ')'"));
		}

		if(sym->val.funval->funtype == fun_builtin)
			retv = sym->val.funval->builtin(args);

		freeargs(args);

		return retv;
	}

	return NULL;
}
