#ifndef __H_TYPES
#define __H_TYPES

typedef enum {err_noerr, err_symresolve, err_syntax,
	err_call, err_type} errtype_t;

typedef enum {invalidval, ival, fval, sval, errval, funval} valtype_t;

typedef enum {fun_inval, fun_builtin, fun_loaded} funtype_t;

struct error {
	errtype_t errtype;
	char *reason;
};

struct value {
	valtype_t valtype;
	union {
		int ival;
		double fval;
		char *sval;
		struct error *errval;
		struct funspec *funval;
	};
};

struct symbol {
	char *name;
	struct value val;
	struct symbol *sym_next;
};

struct args {
	struct value val;
	struct args *next;
};

struct funspec {
	funtype_t funtype;
	union {
		struct retrn* (*builtin)(struct args*);
		int tonsofdickscantloadyet;
	};
};

/* this exists because value structs returned are sometimes
 * malloc'd and sometimes not, we need a uniform way to tell
 * the caller what to do */
struct retrn {
	int shouldfree;
	struct value *val;
};

#endif	/* __H_TYPES */
