#ifndef __H_UTIL
#define __H_UTIL

struct retrn* mkretrn(int shouldfree, struct value *val);
void die(const char *err);
char* getsymname(const char *b);
void printval(struct value *val);
int strmatches(const char *s, const char *m);

char* errstr(errtype_t et);
struct value* genericerror(errtype_t type, const char *reason);
struct value* symresolveerror(const char *what);

void* xmalloc(size_t sz);

char* getstring(char *b);

char* allocstring(const char *p);

int hasfloat(const char *p);

#endif
