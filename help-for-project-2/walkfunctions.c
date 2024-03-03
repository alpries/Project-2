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
  // Using Mailman algorithm
  b = bread(DEVFD, 32 + inodenum / 16);
  //inode = (struct dinode *) &b->data[(inodenum % 16)*64];
  Lmemcpy(inode, &b->data[(inodenum % 16)*64], 64);
  brelse(b);
  //Lprintf("inode %d:\n", 0);
  if (inode->type == 0) {
  //	Lprintf("  UNUSED (file type = 0)\n");
  	return -1;
  }
  //Lprintf("  file type = %d\n", inode->type);
  //Lprintf("  number of links = %d\n", inode->nlink);
  //Lprintf("  file size = %u (bytes)\n", inode->size);
  //Lprintf("  block map:\n");
  //for (uint j = 0; j < NDIRECT && inode->addrs[j] != 0; j++)
  //	  Lprintf("    direct block 0x%08x\n", inode->addrs[j]);
  //if (inode->size > NDIRECT * BSIZE && inode->addrs[NDIRECT - 1] != 0)
 // 	  Lprintf("      indirect block 0x%08x\n", inode->addrs[NDIRECT]);
  return 0;

}

uint find_name_in_dirblock(uint blockptr, const char *nam){
  //Lprintf("Block ptr: %d Name: %s\n", blockptr, nam);
  struct buf *b;
  b = bread(DEVFD, blockptr);
  //Lprintf("Validity:  %d\n", b->valid);
  if (b->valid == 1){
    struct dirent *dir;
    for (int k = 0; k < 64; k++) {
      dir = (struct dirent *) &b->data[k*16];
      if (Lstrcmp(dir->name, (char *)nam) == 0){
	      return dir->inum;
      }
     //Lprintf("Inode: %d   Name: %s\n", dir->inum, dir->name);
    }	
  }

  return 0;
}

uint find_dent(uint inum, const char *name){
  struct dinode inode;
  int result = getinode(&inode, inum);

  if (result == -1 || inode.type != T_DIR ) {
    	return 0;
  }
  //uint blockptr = inode.addrs[0];
  //uint dentnum = find_name_in_dirblock(blockptr, name);
  uint dentnum = 0;
  for(int i = 0; i < 13; i++){
    if (inode.addrs[i] == 0){
       break;
    }
    dentnum = find_name_in_dirblock(inode.addrs[i], name);
    if (dentnum != 0){
     break;
    }

  }
  return dentnum;
}

uint namei(const char *pathname){
  uint inum;
  if (pathname[0] != '/') {
    return 0;
  }
    char dirName[1024];
    // Location of root inode
    inum = 1;
    int j = 0;
    for(int i = 1; pathname[i] != '\0'; i++){
      dirName[j] = pathname[i];
      if (pathname[i] == '/'){
        dirName[j] = '\0';
        inum = find_dent(inum, dirName);
        if (inum == 0) return 0; // Directory not found
        dirName[0] = '\0';
        j = -1;
      }
      j++;
    }
    if (j > 0) { // Handle the last part of the path
        dirName[j] = '\0';
        inum = find_dent(inum, dirName);
    }
    return inum;
}


void lsdir(uint blockptr){
  struct buf *b;
  b = bread(DEVFD, blockptr);
  struct dirent *dir;

  for (int k = 0; k < 64; k++) {
    dir = (struct dirent *) &b->data[k*16];
    struct dinode inode;
    int result = getinode(&inode, dir->inum);
    if(result == -1){
      continue;
    }
   // Lprintf("Inode: %d   Name: %s\n", dir->inum, dir->name);
    Lprintf("%s      %d %d %d\n", dir->name, inode.type, dir->inum, inode.size);
  }

  brelse(b);
}

int lspath(const char *pathname){
  uint inum = namei(pathname);
  if(inum == 0){
    return -1;
  }

  struct dinode inode;
  int result = getinode(&inode, inum);
  if(result == -1 || inode.type != T_DIR){
    return -1;
  }

  lsdir(inode.addrs[0]);

  return 0;
}

uint cd(uint inum, const char *name){
  uint inumReturn = find_dent(inum, name);
  if (inumReturn == 0){
    return -1;
  }
  struct dinode inode;
  int result = getinode(&inode, inumReturn);
  if(result == -1 || inode.type != T_DIR){
    return -1;
  }
  return inumReturn;
}
