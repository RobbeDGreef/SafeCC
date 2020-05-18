#ifndef __STDLIB_H
#define __STDLIB_H

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif /* NULL */

#ifndef __SIZE_T
#define __SIZE_T
typedef uint32_t size_t;
#endif /* __SIZE_T */

#ifndef __WCHAR_T
#define __WCHAR_T
typedef short whar_t;
#endif /* __WCHAR_T */

#ifndef __DIV_T
#define __DIV_T
typedef struct { int quot; int rem; } div_t;
#endif /* __DIV_T */

void abort(void);
void _Exit(int exitcode);
void exit(int exitcode);

double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
double strtod(const char *str, char **endptr);
long strtol(const char *str, char **endptr, int base);

int rand(void);
int random(void);
void srand(unsigned int seed);
void srandom(unsigned int seed);

void *malloc(size_t size) __attribute__((returns_heapval));
void *calloc(size_t nitems, size_t size) __attribute__((returns_heapval));
void free(void *ptr) __attribute__((clears_heapval (0)));
void *realloc(void *ptr, size_t size) __attribute__((returns_heapval));

char *getenv(const char *name);
int system(const char *str);
int abs(int x);
div_t div(int num, int denom);

#endif /* __STDLIB_H */