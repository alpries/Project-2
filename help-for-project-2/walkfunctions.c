#include "posix-calls.h"
#include "Llibc.h"
#include "Lcli.h"
#include "walkfunctions.h"
extern int DEVFD;

/* From Lbio.c */
void binit(void);
struct buf* bread(uint, uint);
void brelse(struct buf*);
void bwrite(struct buf*);
extern struct {
    /* struct spinlock lock; */
    struct buf buf[NBUF];
    struct buf head;
} bcache;

int getinode(struct dinode *inode,  uint inodenum){
  struct buf *b;
  // Using Mailman algorithm
  b = bread(DEVFD, 32 + inodenum / 16);
  Lmemcpy(inode, &b->data[(inodenum % 16)*64], 64);
  brelse(b);
  if (inode->type == 0) {
  	return -1;
  }

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
    Lprintf("%-14s %d %d %d\n", dir->name, inode.type, dir->inum, inode.size);
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


int unlink(const char *pathname){
  char fileName[20] = {0};
  uint inum = dirWithFileToRm(pathname, fileName);
  if(inum == 0){
    return -1;
  }
  struct dinode inode;
  int result = getinode(&inode, inum);
  if(result == -1 || inode.type != T_DIR){
    return -1;
  }
  for(int i = 0; i < 13; i++){
    if (inode.addrs[i] == 0){
      return -1;
    }
    uint blockptr = inode.addrs[i];
    struct buf *b;
    b = bread(DEVFD, blockptr);
    //Lprintf("Validity:  %d\n", b->valid);
    if (b->valid == 1){
      struct dirent *dir;
      for (int k = 0; k < 64; k++) {
        dir = (struct dirent *) &b->data[k*16];
        if (Lstrcmp(dir->name, fileName) == 0){
          int result = getinode(&inode, dir->inum);
          if(result == -1){
             brelse(b);
            return -1;
          }
          if(inode.type == T_FILE){
            dir->inum = 0;
            b->dirty = 1;
	    bwrite(b);
            brelse(b);
            return 0;
          }
        }
      }
      brelse(b);
    }
  }
  return -1;
}

uint dirWithFileToRm(const char *pathname, char *name){
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
        Lstrcpy(name, dirName);
    }
    return inum;
}

int link(const char *pathname, const char *pathname2){
  char fileName[20] = {0};
  char fileName2[20] = {0};
  uint inum = dirWithFileToRm(pathname, fileName);
  uint inum2 = dirWithFileToRm(pathname2, fileName2);
  if(inum == 0 || inum2 == 0){
    return -1;
  }
  struct dinode inode;
  struct dinode inode2;
  int result = getinode(&inode, inum);
  int result2 = getinode(&inode2, inum2);
  if(result == -1 || result2 == -1 || inode.type != T_DIR || inode2.type != T_DIR){
    return -1;
  }
  for(int i = 0; i < 13; i++){
    if (inode.addrs[i] == 0){
      return -1;
    }
    uint blockptr = inode.addrs[i];
    uint blockptr2 = inode2.addrs[i];
    struct buf *b;
    struct buf *b2;
    b = bread(DEVFD, blockptr);
    b2 = bread(DEVFD, blockptr2);
    //Lprintf("Validity:  %d\n", b->valid);
    if (b->valid == 1 && b2->valid == 1){
      struct dirent *dir;
      struct dirent *dir2;

      for (int k = 0; k < 64; k++) {
        dir = (struct dirent *) &b->data[k*16];
        if (Lstrcmp(dir->name, fileName) == 0){
          int result = getinode(&inode, dir->inum);
          if(result == -1 || inode.type != T_FILE){
             brelse(b);
            return -1;
          }
            break;
        }
      }

      for (int k = 0; k < 64; k++) {
        dir2 = (struct dirent *) &b2->data[k*16];
        if (Lstrcmp(dir2->name, fileName2) == 0){
          int result2 = getinode(&inode2, dir2->inum);
          if(result2 == -1 || inode2.type != T_FILE){
             brelse(b2);
            return -1;
          }
            break;
        }
      }

      dir->inum = dir2->inum;
      bwrite(b);
      brelse(b);
      brelse(b2);
      return 0;
    }
  }
  return -1;
}

uint createPath(uint inum, const char *name) {
    struct dinode inode;
    struct buf *b = NULL;
    struct dirent *dir = NULL;

    if (getinode(&inode, inum) == -1 || inode.type != T_DIR) {
        return -1; // Inode must exist and be a directory
    }

    for (int i = 0; i < 13; i++) {
        if (inode.addrs[i] != 0) {
            b = bread(DEVFD, inode.addrs[i]);
            for (uint k = 0; k < 64; k++) {
                dir = (struct dirent *)&b->data[k * sizeof(struct dirent)];
                if (dir->inum == 0) { // Assumes an unused entry has inum set to 0
                    // Found an empty slot
                    size_t name_len = Lstrlen((char *)name);
                    if (name_len >= DIRSIZ) {
                        name_len = DIRSIZ - 1;
                    }
                    Lmemcpy(dir->name, name, name_len);
                    dir->name[name_len] = '\0';

                    // Assume emptyInode finds or allocates a free inode
                    uint emptyInode = 1;
                    if (emptyInode == 0) {
                        brelse(b);
                        return -1; // Handle error
                    }

                    dir->inum = emptyInode;
                    bwrite(b); // Write back to disk
                    brelse(b); // Release the buffer
                    return emptyInode;
                }
            }
            brelse(b); // Release the buffer if no empty entry found in this block
        }
    }
    return -1; // No space found
}




void sync() {
  struct buf *b;
    for (b = bcache.head.next; b!= &bcache.head; b = b->next) {
        if (b->valid == 0 || b->refcnt == 0) {
            continue;
        }
        if (b->dirty == 0) {
            continue;
        }
        bwrite(b);
    }
}
