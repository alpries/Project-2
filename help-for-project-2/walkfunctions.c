#include "posix-calls.h"
#include "Llibc.h"
#include "Lcli.h"
#include "walkfunctions.h"
extern int DEVFD;

/* From Lbio.c */
void binit(void);
struct buf* bread(uint, uint);
void brelse(struct buf*);

int getinode(struct dinode *inode,  uint inodenum){
  struct buf *b;
  b = bread(DEVFD, inodenum);
  int inodesize = sizeof(struct dinode);
  //for (int k = 0; k < n; k++) {
		inode = (struct dinode *) &b->data[0*inodesize];
		Lprintf("inode %d:\n", 0);
		if (inode->type == 0) {
			Lprintf("  UNUSED (file type = 0)\n");
			return 0;
			
		}
		Lprintf("  file type = %d\n", inode->type);
		Lprintf("  number of links = %d\n", inode->nlink);
		Lprintf("  file size = %u (bytes)\n", inode->size);
		Lprintf("  block map:\n");
		for (uint j = 0; j < NDIRECT && inode->addrs[j] != 0; j++)
			Lprintf("    direct block 0x%08x\n", inode->addrs[j]);
		if (inode->size > NDIRECT * BSIZE && inode->addrs[NDIRECT - 1] != 0)
			Lprintf("      indirect block 0x%08x\n", inode->addrs[NDIRECT]);
	//}
  return 1;

}
/*
  Find the inode struct for the given inode number, and fill in the
  caller supplied pointer to struct dinode.
  Return 0 on success, -1 on error.

  Used by several other functions!
*/
