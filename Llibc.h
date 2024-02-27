/*

File Llibc.h

User library functions (like the C library), but prefixed with 'L'

*/

#include "posix-calls.h"
#include "syscall.h"        /* int Lsyscall(...); */
extern int      errno;

unsigned int    Lstrlen(char *p);
char*           Lstrcpy(char *s, const char *t);
int             Lstrcmp(char *ptr, char *ref);
char*           Lstrchr(const char *s, char c);
int             Lisspace(char c);
char*           Lgets(char *buf, int max);
void*           Lmemset(void *dst, int c, unsigned int n);
void*           Lmemmove(void *vdst, const void *vsrc, int n);
int             Lmemcmp(const void *s1, const void *s2, unsigned int n);
void*           Lmemcpy(void *dst, const void *src, unsigned int n);
int             Latoi(const char *s);
void            Litoa (char *buf, int base, int d);
void            Lltoa (char *buf, int base, long d);
void            Lfprintf (int fd, const char *format, ...);
void            Lprintf (const char *format, ...);

