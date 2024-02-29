#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <linux/sched.h>
#include "syscall.h"

extern int errno;

int Lfork(void);
void Lexit(int status);
int Lwait(int *status);
int Lpipe(int fd[2]);
long int Lread(int fd, void *buf, long unsigned int count);
long int Lwrite(int fd, const void *buf, long unsigned int count);
long int Llseek(int fd, long int offset, int whence);
int Lclose(int fd);
int Lkill(int pid);
int Lexec(char *prog, char **args);
int Lexecve(char *prog, char **args, char **envp);
int Lopen(const char *path, int flags, ...);
int Lmknod(char *path, short dev, short mode);
int Lunlink(const char *path);
int Lfstat(int fd, struct stat *statbuf);
int Llink(const char *oldpath, const char *newpath);
int Lmkdir(char *path);
int Lchdir(const char *path);
int Ldup(int oldfd);
int Ldup2(int oldfd, int newfd);
int Lgetpid(void);
int Lpause(void);
unsigned int Lsleep(unsigned int decisecs);
int Luptime(void);
void * Lsbrk(long int size);    /* Implementaion needs to improve */

