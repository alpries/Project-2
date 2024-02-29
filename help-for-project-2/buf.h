struct buf {
  int valid;   // has data been read from disk?
  /* int disk; */    // does disk "own" buf?
  /* The original xv6 buf.disk indicates if disk is working on a block  */
  int dirty;	/* delayed write flag */
  int disk_rw_fail;
  /* uint dev; */
  uint dev_fd;  /* xv6 had a device file; we will have a device fd (opened) */
  uint blockno;
  /* struct sleeplock lock; */
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  uchar data[BSIZE];
};

