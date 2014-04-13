#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "lispcalc.h"
#include "symbol.h"
#include "util.h"
#include "builtins.h"

struct symbol *symtbl;

static int lispcalc(int fd);
static struct retrn* process(char *buf);

int
main(int argc, char **argv)
{
	struct value *val;

	symtbl = xmalloc(sizeof(struct symbol));
	symtbl->sym_next = NULL;
	symtbl->name = "PROGRAM";
	symtbl->val.valtype = sval;
	symtbl->val.sval = PROGRAM;

	setsymbol("VERSION", mkstr(VERSION));

	val = xmalloc(sizeof(struct value));
	val->valtype = funval;
	val->funval = xmalloc(sizeof(struct funspec));
	val->funval->funtype = fun_builtin;
	val->funval->builtin = builtin_add;

	setsymbol("+", val);
	setsymbol("add", val);

	val->funval->builtin = dump_symtbl;
	setsymbol("dump", val);
	setsymbol("symtbl", val);

	return lispcalc(STDIN_FILENO);
}

static int
lispcalc(int fd)
{
	char *inbuf;
	int i, inbufsz;
	struct retrn *retrn;

	/* get a 64K buffer */
	inbufsz = 64 * 1024;
	inbuf = xmalloc(inbufsz);

	for(;;){
		i = read(fd, inbuf, inbufsz-1);
		if(i < 0){
			/* if EINTR, reason was probably ^c */
			if(errno == EINTR)
				continue;
			die("read");
		}

		if(i == 0){
			puts("eof");
			return 0;
		}

		/* ensure NULL termination */
		inbuf[i] = 0;
		retrn = process(inbuf);
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

static struct retrn*
process(char *buf)
{
	struct symbol *sym;
	struct value *val;

	SKIPWS(buf);
	/* if we don't find '(', we expect a just a value (int, float,
	 * string or symbol name) here so we find and return it */
	if(*buf != '('){
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

	/* here, we will always have '(', so we skip that */
	if(!strmatches(++buf, "define")){
		/* define a new symbol */
		sym = xmalloc(sizeof(struct symbol));

		/* first skip forward to new sym name */
		buf += 6;
		SKIPWS(buf);

		/* extract symbol name and skip forward to the value */
		sym->name = getsymname(buf);
		while(ISSYMCHAR(*buf))
			buf++;

		SKIPWS(buf);

		/* first identify strings because that is easy */
		if(*buf == '"'){
			sym->val.valtype = sval;
			sym->val.sval = getstring(buf);
			if(sym->val.sval == NULL){
				freesym(sym);
				return mkretrn(1, genericerror(err_syntax,
					"invalid string specification"));
			}
		}
	}
	/* not a number, string or built-in special? search for a symbol */
	else {
		char *symnam;
		struct args *args;
		struct retrn *retv;

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
			memcpy(&lastarg->val, retv->val, sizeof(struct value));

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
