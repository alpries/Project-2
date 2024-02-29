#include "types.h"
#include "param.h"
#include "fs.h"
#include "buf.h"
#include "Ldiskio.h"
#include "Llibc.h"

/*  This is where the global buffer pool is defined and storage allocated
    for all the NBUF+1 struct bufs!  Note that this is in bss, and
    while storage is allocated here, everything is null, including
    all the pointers inside each of the NBUF+1 struct bufs here.
    (Things will be initialized via binit().)
*/
struct {
  /* struct spinlock lock; */
  struct buf buf[NBUF];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache;

/*
    Picture of bcache after binit():

   +---------------->------------->--------------->--------------+
   |                                                             |
   ^        head     buf[NBUF-1]         buf[1]    buf[0]        v
   |      +---------+---------+-     -+---------+---------+      |
   +--<---|<-prev   |<-prev   | - - - |<-prev   |<-prev   |<--<--+
          |         |         |       |         |         |
   +-->-->|   next->|   next->| - - - |   next->|   next->|--->--+
   |      +---------+---------+-     -+---------+---------+      |
   ^                   MRU                          LRU          v
   |                                                             |
   +------------------<------------<---------------<-------------+

*/

/* 
    This is where the NBUF+1 prev/next pointer pairs actually get
	meaningful values and the doubly linked list is formed as above!
*/
void
binit(void)
{
  struct buf *b;

  /* initlock(&bcache.lock, "bcache"); */

  // Create linked list of buffers
  /* Start with a singleton self-loop bcache.head */
  bcache.head.prev = &bcache.head;  /* self-ref */
  bcache.head.next = &bcache.head;  /* self-ref */
  /* Incrementally grow cycles! */
  for (b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head; 
    /* initsleeplock(&b->lock, "buffer"); */
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  /* 
	Study the four lines in the body of the for loop carefully
	to understand how the doubly linked list is formed.  It
	grows incrementally in each iteration of the for loop:
      - Starts with the singleton of only bcache.head
      - Then for k = 0, 1, ... it grows like this (left prev, right next):
            head buf[0]         (k=0 two-element cycle bcache.head,bcache.buf+0)
            head buf[1] buf[0]  (k=1 three-element cycle head,buf+1,buf+0)
            ...
            head buf[k-1] . . . buf[1] buf[0]
            head buf[k] buf[k-1] . . . buf[1] buf[0]
            ...
            head buf [NBUF-1] . . . buf[k] buf[k-1] . . . buf[1] buf[0]
  */
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
/* bget(uint dev, uint blockno) */
bget(uint dev_fd, uint blockno)
{
  struct buf *b;

  /* acquire(&bcache.lock); */

  // Is the block already cached?
  for (b = bcache.head.next; b != &bcache.head; b = b->next){
    /* if (b->dev == dev && b->blockno == blockno){ */
    if (b->dev_fd == dev_fd && b->blockno == blockno){
      b->refcnt++;
      /*
      release(&bcache.lock);
      acquiresleep(&b->lock);
      */
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (b = bcache.head.prev; b != &bcache.head; b = b->prev){
    if (b->refcnt == 0) {
      b->dev_fd = dev_fd;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      /*
      release(&bcache.lock);
      acquiresleep(&b->lock);
      */
      return b;
    }
  }
  /* panic("bget: no buffers"); */
  return (struct buf *) 0;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev_fd, uint blockno)
{
  struct buf *b;

  b = bget(dev_fd, blockno);
  if (!b->valid) {
    /* virtio_disk_rw(b, 0); */
	/* Lfprintf(2, "DEBUG:  disk i/o for blockno = %d\n", blockno); */
    disk_block_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  /*
  if (!holdingsleep(&b->lock))
    panic("bwrite");
  */
  /* virtio_disk_rw(b, 1); */
    disk_block_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  /*
  if(!holdingsleep(&b->lock))
    panic("brelse");
  */

  /* 
  releasesleep(&b->lock);

  acquire(&bcache.lock);
  */
  if (b->refcnt > 0) {
  	b->refcnt--;
  }
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }

  /* release(&bcache.lock); */
}

void
bpin(struct buf *b) {
  /* acquire(&bcache.lock); */
  b->refcnt++;
  /* release(&bcache.lock); */
}

void
bunpin(struct buf *b) {
  /* acquire(&bcache.lock); */
  if (b->refcnt > 0)
  	b->refcnt--;
  /* release(&bcache.lock); */
}

