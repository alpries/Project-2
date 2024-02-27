#include "Llibc.h"

/* MIT xv6 cat for standalone linux/riscv64 */

char buf[512];

void
cat(int fd)
{
  int n;

  while((n = Lread(fd, buf, sizeof(buf))) > 0) {
    if (Lwrite(1, buf, n) != n) {
      Lfprintf(2, "cat: write error\n");
      Lexit(1);
    }
  }
  if(n < 0){
    Lfprintf(2, "cat: read error\n");
    Lexit(1);
  }
}

int
Lmain(int argc, char *argv[])
{
  int fd, i;

  if (argc <= 1){
    cat(0);
    Lexit(0);
  }

  for(i = 1; i < argc; i++){
    if((fd = Lopen(argv[i], 0)) < 0){
      Lfprintf(2, "cat: cannot open %s\n", argv[i]);
      Lexit(1);
    }
    cat(fd);
    Lclose(fd);
  }
  return 0;
}

