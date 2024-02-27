/*
    All function names are prefixed with 'L'!

    POSIX syscall wrappers for linux in standalone code (matches xv6 list),
    which can be used directly without C startup routines (without main).

    The linux style syscall(2) function is implemented in syscall-riscv64.S.

    Updated 2024 February 13 by Abhijit Dasgupta (MIT license).

*/

#include "posix-calls.h"

int
Lfork(void)
{
    return Lsyscall(SYS_clone, 0, 0, 0, 0);
}

void
Lexit(int status)
{
    Lsyscall(SYS_exit, status, 0, 0, 0);
}

int
Lwait(int *status)
{
    return Lsyscall(SYS_wait4, -1, status, 0, 0);
}

int
Lpipe(int fd[2])
{
    return Lsyscall(SYS_pipe2, fd, 0);
}

long int
Lread(int fd, void *buf, long unsigned int count)
{
    return Lsyscall(SYS_read, fd, buf, count);
}

long int
Lwrite(int fd, const void *buf, long unsigned int count)
{
    return Lsyscall(SYS_write, fd, buf, count);
}

int
Lclose(int fd)
{
    return Lsyscall(SYS_close, fd);
}

int
Lkill(int pid)
{
    return Lsyscall(SYS_kill, pid, 15);
}

int
Lexec(char *prog, char **args)
{
    return Lsyscall(SYS_execve, prog, args, 0);
}

int
Lopen(const char *path, int flags, ...)
{
    return Lsyscall(SYS_openat, AT_FDCWD, path, flags, 0);
}

int
Lmknod(char *path, short dev, short mode)
{
    return Lsyscall(SYS_mknodat, AT_FDCWD, path, (int) mode, (int) dev);
}

int
Lunlink(const char *path)
{
    return Lsyscall(SYS_unlinkat, AT_FDCWD, path, 0);
}

int
Lfstat(int fd, struct stat *statbuf)
{
    return Lsyscall(SYS_fstat, fd, statbuf);
}

int
Llink(const char *oldpath, const char *newpath)
{
    return Lsyscall(SYS_linkat, AT_FDCWD, oldpath, AT_FDCWD, newpath);
}

int
Lmkdir(char *path)
{
    return Lsyscall(SYS_mkdirat, AT_FDCWD, path, 0777);
}

int
Lchdir(const char *path)
{
    return Lsyscall(SYS_chdir, path);
}

int
Ldup(int oldfd)
{
    return Lsyscall(SYS_dup, oldfd);
}

int
Ldup2(int oldfd, int newfd)
{
    return Lsyscall(SYS_dup3, oldfd, newfd, 0);
}

int
Lgetpid(void)
{
    return Lsyscall(SYS_getpid);
}

unsigned int
Lsleep(unsigned int decisecs)   /* Match xv6 */
{
    struct timespec ts;
    ts.tv_sec = decisecs/10;   /* seconds */
    ts.tv_nsec = (decisecs % 10) * 100000000;     /* nanoseconds */
    return Lsyscall(SYS_nanosleep, ts, 0);
}

int
Luptime(void)       /* Not implemented */
{
    return 0;
}

/* Simple sbrk allocator without freeing mechanism */
#define HEAPMAX (16*1024*1024)          // 16MiB
static char *heap = 0;
static char *brkp = 0;
static char *endp = 0;

void *
Lsbrk(long int size)
{
    if (heap == 0 && brkp == 0 && endp ==0) {   /* First call to sbrk */
        // Pre-allocate the HEAPMAX (16MiB) COW chunk, no real free :(
        heap = (char *) Lsyscall(SYS_mmap,
                            0,
                            HEAPMAX,
                            (PROT_READ | PROT_WRITE),
                            (MAP_PRIVATE | MAP_ANONYMOUS),
                            -1,
                            0);
        brkp = heap;
        endp = brkp + HEAPMAX;
    }

    if (size == 0) {
        return (void *) brkp;
    }

    void *newbeg = (void *) brkp;
    brkp += size;
    if (brkp >= endp) {
        return 0;
    }
    if (brkp < heap) {
        brkp = heap;
        return 0;
    }
    return newbeg;
}

