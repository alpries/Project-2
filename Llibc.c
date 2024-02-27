/*
File Llibc.c

User library functions (like the C library functions, cf. man 3).

All non-static global function names are prefixed with 'L'

The README file:

What is this?
=============

This is a stanalone set of a few C library and POSIX syscall functions
that depend only on the riscv64 linux kernel.  They are completely
independent of the standard C/C++ libraries, and can serve as a micro
sized C library built from scracth.  To build programs based on them,
use the `-nostdlib -static' gcc/linker flags and note that:

    All non-static global function names are prefixed with 'L'

The source files are:

- Llibc.c : A set of ~15 C library functions built upon the POSIX syscalls
    Lwrite() and Lread() (and no others) from posix-calls.c.  It needs:

- posix-calls.c :  A set of ~20 POSIX syscalls based on linux syscall(2).
    In turn, the functions in posix-calls.c need the linux syscall ABI:

- syscall-riscv64.S :  Uses riscv64 calling ABI to implement linux syscall(2) 
    with errno support

- crt-callmain-riscv64.S :  Implements C runtime code for calling main()
    based on linux execv ABI and riscv64 calling ABI

- Llinker.ld :  Linker script

Dependency structure
====================

                            Llibc.c
        (Library functions, like the C library functions in man 3)
              |                                       |
              |                                       | for using main()
              |                                       |
        posix-calls.c                       crt-callmain-riscv64.S
      (Posix syscall functions)           (C runtime code, linux/riscv ABI)
              |
              |
        syscall-riscv64.S
      (Standalone linux syscall(2) ABI, with errno)

To build programs using Llibc functions, link against the full object set:

    Llibc.o posix-calls.o syscall-riscv64.o crt-callmain-riscv64.o

You could also bypass Llibc/main() and use the functions in posix-calls.c
(link with posix-calls.o and syscall-riscv64.o), or even bypass that and
use the syscall(2) from syscall-riscv64.S (link with syscall-riscv64.o).

How programs are run
====================

    - The linker uses an entry point symbol (default label is "_start")
      to know the machine code instruction where program execution
      should begin, and when the program executable file is built,
      the header is marked with the address of this entry point

    - When executing an executable binary, the OS kernel looks at the
      header to find out this entry point, loads the program into RAM,
      makes the CPU jump to the entry point - and the program starts
      running

    - During program building/compilation, the linker (behind the scenes) 
      adds an initial piece of program code (the one with the "_start" label)
      to the beginning of your program code.  This initial piece of code,
      known as the C runtime startup routine, is the one responsible for
      calling main(), using an architecture-dependent calling convention
      ABI.  This code is in:

        crt-callmain-riscv64.S (source), crt-callmain-riscv64.o (object) 

      The crt startup routine simply sets up main()'s argument parameters
      (that the kernel has put on program stack when exec'ing the program),
      and then calls main().  When main() returns, the crt routine uses the
      exit syscall to terminate the program with the return value returned
      by main().

How to build the Llibc linux library
====================================

  - Compile source files into object files (without linking):

        gcc -Wall -c syscall-riscv64.S -o syscall-riscv64.o
        gcc -Wall -c posix-calls.c -o posix-calls.o
        gcc -Wall -c crt-callmain-riscv64.S -o crt-callmain-riscv64.o
        gcc -Wall -c Llibc.c -o Llibc.o

  - Put them in a static library file lib4490.a (optional):

        ar rcs lib4490.a \
            syscall-riscv64.o \
            posix-calls.o \
            crt-callmain-riscv64.o \
            Llibc.o

  (To link with the static library file lib4490.a, use the build flags
  -L. -l4490; see example below.)

How to build standalone linux programs using Llibc
==================================================

    - Compile the library/syscall object files as above

    - In your userprog.c file, #include "Llibc.h" to use the functions
      in Llibc.h --- you have to use the L prefix for any function!

    - Build a standalone user program userprog.c as follows:

        # Compile (but do not link) the user program object code:
        gcc -Wall -c userprog.c -o userprog.o

        # Build program by linking against lib4490.a (if already done)
        ld -TLlinker.ld -static -nostdlib userprog.o -L. -l4490 -o userprog

        # Or build by linking directly against the object files:
        ld -TLlinker.ld -static -nostdlib userprog.o \
            syscall-riscv64.o posix-calls.o \
            crt-callmain-riscv64.o Llibc.o \
            -o userprog

      Order can be important during linking!

    You can make a Makefile for doing all this (exercise!)

Example user codes that use Llibc
=================================

You can build the following sample user codes using the above procedure:

    cat.c           Sample user code (same as xv6:user/cat.c)
    xecho.c         Sample user code

For practice, convert the xv6 user program codes in grep.c and wc.c
and build them against Llibc/linux.

(End of README file)

  Updated 2024 February 13 (Abhijit Dasgupta, MIT license)

  Some of these are adapted from the file kernel.c found in
  GNU Multiboot Specification documents, version 0.6.96, but
  the code there assumed stack based calling convention ABI
  (like i386), so it was ported to the ANSI/ISO C varargs standard
  to support register based calling ABIs (x86_64, riscv, arm).
  See Steve Summit's notes on eskimo.com.  Some very simple ones
  can be found everywhere (e.g., MIT xv6 code).

*/

#include <stdarg.h>         /* From gcc really, not C library  */
#include "Llibc.h"

unsigned int
Lstrlen(char *p)
{
    int len;
    for (len = 0; *(p + len) != 0; len++)
        ;
    return len;
}

char*
Lstrcpy(char *dst, const char *src)
{
    char *origdst;

    origdst = dst;
    while((*dst++ = *src++) != 0)
        ;
    return origdst;
}

int
Lstrcmp(char *ptr, char *ref)
{
    while (1 == 1) {
        if (*ptr == '\0' || *ref == '\0')
            break;
        if (*ptr != *ref)
            break;
        ptr++; ref++;
    }
    return (int) (*ptr - *ref);
}

char*
Lstrchr(const char *s, char c)
{
    for (; *s; s++) {
        if (*s == c)
            return (char*) s;
    }
    return 0;
}

int
Lisspace(char c)
{
    switch (c) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\f':
        case '\v':
            return 1;
    }
    return 0;
}

char*
Lgets(char *buf, int max)
{
    int i, cc;
    char c;

    for (i=0; i+1 < max; ) {
        cc = Lread(0, &c, 1);
        if (cc < 1)
            break;
        buf[i++] = c;
        if (c == '\n' || c == '\r')
            break;
    }
    buf[i] = '\0';
    return buf;
}

void*
Lmemset(void *dst, int c, unsigned int n)
{
    char *cdst = (char *) dst;
    int i;
    for (i = 0; i < n; i++) {
        cdst[i] = c;
    }
    return dst;
}

void*
Lmemmove(void *vdst, const void *vsrc, int n)
{
    char *dst;
    const char *src;

    dst = vdst;
    src = vsrc;
    if (src > dst) {
        while (n-- > 0)
            *dst++ = *src++;
    } else {
        dst += n;
        src += n;
        while (n-- > 0)
            *--dst = *--src;
    }
    return vdst;
}

int
Lmemcmp(const void *s1, const void *s2, unsigned int n)
{
    const char *p1 = s1, *p2 = s2;
    while (n-- > 0) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void *
Lmemcpy(void *dst, const void *src, unsigned int n)
{
    return Lmemmove(dst, src, n);
}
 
int
Latoi(const char *s)
{
    int n;

    n = 0;
    while ('0' <= *s && *s <= '9')
        n = n*10 + *s++ - '0';
    return n;
}

/* Convert the long int d to a string and save the string in buf.
   By default, regard d as unsigned and convert to hex (base == 'x').
   If base is 'd', regard d as signed and convert to decimal.
   If base is 'u', regard d as unsigned and convert to decimal.
 */
void
Lltoa (char *buf, int base, long d)     /* Default: convert unsigned to hex */
{
    char *p1, *p2, *p = buf;    /* Initialize p to buf */
    unsigned long ud, divisor;  /* ud will be divided by divisor */

    /* Default:  Take d as unsigned and base == 'x' */
    ud = (unsigned long) d;
    divisor = 16;

    if (base == 'u') {  /* Decimal case 'u', assume d is unsigned */
        divisor = 10;
    }

    if (base == 'd') {  /* Decimal case 'd', assume d is signed */
        divisor = 10;
        if (d < 0) {    /* Put `-' in the head and convert |d| instead */
            *p++ = '-';
            buf++;
            ud = (unsigned long) -d;    /* ud = abs val of d */
        }
    }

    /* Keep dividing ud by divisor until ud == 0. */
    do {
        /* See elsewhere for direct implementation of the division algorithm */
        unsigned long remainder = ud % divisor;
        *p++ = (remainder < 10) ? '0' + remainder: 'a' + remainder - 10;
    } while (ud /= divisor);

    /* Terminate buf. */
    *p = 0;

    /* Reverse buf. */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

/* Convert the integer d to a string and save the string in buf.
   By default, regard d as unsigned and convert to hex (base == 'x').
   If base is 'd', regard d as signed and convert to decimal.
   If base is 'u', regard d as unsigned and convert to decimal.
 */
void
Litoa (char *buf, int base, int d)
{
    char *p1, *p2, *p = buf;    /* Initialize p to buf */
    unsigned long ud, divisor;  /* ud will be divided by divisor */

    /* Default:  Take d as unsigned and base == 'x' */
    ud = (unsigned long) (unsigned int) d;
    divisor = 16;

    if (base == 'u') {  /* Decimal case 'u', assume d is unsigned */
        divisor = 10;
    }

    if (base == 'd') {  /* Decimal case, assume d is signed */
        divisor = 10;
        if (d < 0) {    /* Put `-' in the head and convert |d| instead */
            *p++ = '-';
            buf++;
            ud = (unsigned long) -d;    /* ud = abs val of d */
        }
    }

    /* Keep dividing ud by divisor until ud == 0. */
    do {
        /* See elsewhere for direct implementation of the division algorithm */
        unsigned long remainder = ud % divisor;
        *p++ = (remainder < 10) ? '0' + remainder: 'a' + remainder - 10;
    } while (ud /= divisor);

    /* Terminate buf. */
    *p = 0;

    /* Reverse buf. */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}


/* Write one character --- WITH BUFFERING!!! */
#define BUFSIZE  1024
#define MAXFD 32
static char outbuf[MAXFD][BUFSIZE];
static unsigned int nbuf[MAXFD];
/* putc function with buffering */
static int
Lfdputchar(int fd, char c, int flushonly /* if == 1 just flush buffer */)
{
    if (flushonly == 1) {   /* Ignore character c if flushonly == 1 */
        if (nbuf[fd] > 0) {
            Lwrite(fd, &outbuf[fd][0], nbuf[fd]);
            nbuf[fd] = 0;
        }
        return 0;
    }

    outbuf[fd][nbuf[fd]] = c;
    nbuf[fd]++;
    if (nbuf[fd] >= BUFSIZE || c == '\n') {
        Lwrite(fd, &outbuf[fd][0], nbuf[fd]);
        nbuf[fd] = 0;
    }

    return 0;
}

/* Libc printf like string formatting functions.
    Old multiboot only worked in stack based calling ABI (i386).  Updated
    to varargs standard for register based calling ABI (x86_64, riscv, arm).
    See Steve Summit notes:
        https://www.eskimo.com/~scs/cclass/int/sx11a.html
            25.1 Declaring ``varargs'' Functions
        https://www.eskimo.com/~scs/cclass/int/sx11b.html
            25.2 Writing a ``varargs'' Function
        https://www.eskimo.com/~scs/cclass/int/sx11c.html
            25.3 Special Issues with Varargs Functions
*/
void
Lvfprintf(int fd, const char *format, va_list argp)
{
    char c;
    char buf[80];
    long l;
    int longflag = 0;

    // va_list argp;            /* Caller passes this */
    // va_start(argp, format);  /* Caller will do this ... */

    while ((c = *format++) != 0) {
        if (c != '%')
            Lfdputchar(fd, c, 0);
        else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;

            if (c == '%') {
                Lfdputchar(fd, c, 0);
                continue;
            }

            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            while (c >= '0' && c <= '9') {
                pad = 10*pad + c - '0';
                c = *format++;
            }

            longflag = 0;
            if (c == 'l') {
                longflag = 1;
                c = *format++;
            }

            switch (c) {
                case '\0':  /* Premature end of format str, abort */
                    format--;   /* Now we have *format == '\0' */
                    break;

                case 'd':
                case 'u':
                case 'x':
                    if (longflag == 1) {
                        l = va_arg(argp, long int);
                        Lltoa(buf, c, l);
                    } else {
                        l = (long) va_arg(argp, int);
                        Litoa(buf, c, l);
                    }
                    p = buf;
                    goto string;
                    break;

                case 's':
                    p = va_arg(argp, char *);
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++);
                    c = (pad0 ? '0' : ' ');
                    for (; p2 < p + pad; p2++)
                        Lfdputchar(fd, c, 0);
                    while (*p)
                        Lfdputchar(fd, *(p++), 0);
                    break;

                default: /* Invalid format specifier charcter after '%' */
                    l = (long) va_arg(argp, int);
                    Litoa(buf, 'x', l);
                    p = buf;
                    Lfdputchar(fd, '[', 0);
                    Lfdputchar(fd, 'x', 0);
                    while (*p)
                        Lfdputchar(fd, *(p++), 0);
                    Lfdputchar(fd, ']', 0);
                    break;
            }
        }
    }
    Lfdputchar(fd, 0, 1); /* flush */
    // va_end(argp);   /* Caller will do this */
}

/* See Steve Summit:  25.3 Special Issues with Varargs Functions
    https://www.eskimo.com/~scs/cclass/int/sx11c.html */
void
Lfprintf(int fd, const char *format, ...)
{
    va_list argp;

    va_start(argp, format);
    Lvfprintf(fd, format, argp);
    va_end(argp);
}

/* See Steve Summit:  25.3 Special Issues with Varargs Functions
    https://www.eskimo.com/~scs/cclass/int/sx11c.html */
void
Lprintf(const char *format, ...)
{
    va_list argp;

    va_start(argp, format);
    Lvfprintf(1, format, argp);
    va_end(argp);
}

