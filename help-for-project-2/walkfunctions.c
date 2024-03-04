#include "posix-calls.h"
#include "Llibc.h"
#include "Lcli.h"
#include "walkfunctions.h"
extern int DEVFD;
extern struct superblock SB;

/* From Lbio.c */
void binit(void);
struct buf* bread(uint, uint);
void brelse(struct buf*);
void bwrite(struct buf*);
extern struct {
    struct buf buf[NBUF];
    struct buf head;
} bcache;

int inodeUsage[BSIZE]; 
void initInodeUsage() {
    for (int i = 0; i < BSIZE; i++) {
        inodeUsage[i] = 0; 
    }
}


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
    uint parentInum = dirWithFileToRm(pathname, fileName);
    if(parentInum == 0){
        return -1; // Failed to find parent directory or file
    }
    struct dinode parentInode;
    if(getinode(&parentInode, parentInum) == -1 || parentInode.type != T_DIR){
        return -1; // Parent is not a directory or failed to read inode
    }

    uint targetInum = find_dent(parentInum, fileName);
    if(targetInum == 0){
        return -1; // Failed to find the target file or directory in the parent
    }

    struct dinode targetInode;
    if(getinode(&targetInode, targetInum) == -1){
        return -1; // Failed to read target inode
    }
    if(targetInode.type != T_FILE && targetInode.type != T_DIR){
        return -1; // Only files and directories can be unlinked
    }

    int removed = 0;
    for(int i = 0; i < 13 && !removed; i++){
        if (parentInode.addrs[i] == 0) continue;

        struct buf *b = bread(DEVFD, parentInode.addrs[i]);
        if (b->valid == 1){
            struct dirent *dir;
            for (int k = 0; k < 64; k++) {
                dir = (struct dirent *) &b->data[k*16];
                if (Lstrcmp(dir->name, fileName) == 0){
                    Lmemset(dir, 0, sizeof(struct dirent));
                    b->dirty = 1;
                    bwrite(b);
                    removed = 1;
                    break;
                }
            }
            brelse(b);
        }
    }
    if (!removed) {
        return -1; 
    }
    return 0; 
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
    struct buf *b;
    struct dirent *dir;

    if (getinode(&inode, inum) == -1 || inode.type != T_DIR) {
        return -1;
    }

    for (int i = 0; i < 13; i++) {
        if (inode.addrs[i] == 0) {
            continue;
        }

        b = bread(DEVFD, inode.addrs[i]);
        for (uint k = 0; k < BSIZE / sizeof(struct dirent); k++) {
            dir = (struct dirent *)&b->data[k * sizeof(struct dirent)];
            if (dir->inum == 0) {
                size_t name_len = Lstrlen((char *)name);
                if (name_len >= DIRSIZ) {
                    name_len = DIRSIZ - 1;
                }
                Lmemcpy(dir->name, name, name_len);
                dir->name[name_len] = '\0';

                uint emptyInode = 0;
                for (uint j = 1; j < BSIZE; j++) { // Start at 1 to skip the root inode
                    if (inodeUsage[j] == 0) { // Found a free inode
                        emptyInode = j;
                        inodeUsage[j] = 1; // Mark as in use
                        break;
                    }
                }

                if (emptyInode == 0) {
                    brelse(b);
                    return -1; // No free inode found
                }

                dir->inum = emptyInode;
                b->dirty = 1;
                bwrite(b);
                brelse(b);

                // Additional code to initialize the new inode goes here.

                return emptyInode;
            }
        }
        brelse(b);
    }
	Lprintf("%d\n", inode);
	Lprintf("%d\n", inum);

    return -1;
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

int iupdate(struct dinode *inode, uint inum) {
    uint blockno = IBLOCK(inum, SB);
    struct buf *bp = bread(DEVFD, blockno);
    struct dinode *dip = (struct dinode *)(bp->data) + (inum % IPB);

    *dip = *inode;
    bp->dirty = 1;
    bwrite(bp);
    brelse(bp);
    return 0; 
}

uint mkdir(const char *path) {
    if (path[0] != '/') {
        return -1; 
    }

    char parentPath[1024] = {0};
    char newDirName[DIRSIZ] = {0};
    const char *slash = Lstrchr(path, '/');
    if (slash && slash != path) { 
        size_t len = slash - path;
        Lmemcpy(parentPath, path, len); 
        parentPath[len] = '\0'; 
        Lmemcpy(newDirName, slash + 1, Lstrlen((char *)slash + 1)); 
    } else {
        return -1; 
    }

    uint parentInum = namei(parentPath);
    if (parentInum == 0) {
        return -1; 
    }

    if (find_dent(parentInum, newDirName) != 0) {
        return -1; 
    }

    uint newDirInum = ialloc(DEVFD, T_DIR);
    if (newDirInum == 0) {
        return -1; 
    }

    struct dinode newDirInode;
    Lmemset(&newDirInode, 0, sizeof(struct dinode));
    newDirInode.type = T_DIR;
    newDirInode.nlink = 2;
    newDirInode.size = 2 * sizeof(struct dirent);

    uint newDirBlock = balloc(DEVFD); // You need to implement balloc
    if (newDirBlock == 0) {
        return -1; 
    }
    newDirInode.addrs[0] = newDirBlock;

    struct buf *b = bread(DEVFD, newDirBlock);
    struct dirent *de = (struct dirent *)b->data;
    de->inum = newDirInum;
    Lmemcpy(de->name, ".", 2);
    de++;
    de->inum = parentInum;
    Lmemcpy(de->name, "..", 3);
    Lmemset((void *)(de + 1), 0, BSIZE - 2 * sizeof(struct dirent));
    b->dirty = 1;
    bwrite(b);
    brelse(b);

    iupdate(&newDirInode, newDirInum);

    struct buf *parentBuf = bread(DEVFD, BBLOCK(parentInum, SB));
	struct dirent *parentDe = (struct dirent *)parentBuf->data;
	for (int i = 0; i < BSIZE / sizeof(struct dirent); i++, parentDe++) {
   	 if (parentDe->inum == 0) { 
       	 parentDe->inum = newDirInum;
       	 size_t dirNameLen = Lstrlen(newDirName);
       	 if (dirNameLen >= DIRSIZ) {
            dirNameLen = DIRSIZ - 1;
       	 }
       	 Lmemcpy(parentDe->name, newDirName, dirNameLen);
       	 parentDe->name[dirNameLen] = '\0'; 
       	 parentBuf->dirty = 1;
       	 bwrite(parentBuf);
       	 break;
  	  }
	}
	brelse(parentBuf);
    return newDirInum; 
}

#define TOTAL_BLOCKS 10000 
int blockBitmap[TOTAL_BLOCKS] = {0};

uint balloc(int dev) {
    for (uint i = 0; i < TOTAL_BLOCKS; i++) {
        if (blockBitmap[i] == 0) { 
            blockBitmap[i] = 1; 
            struct buf *b = bread(dev, i);
            Lmemset(b->data, 0, BSIZE);
            bwrite(b);
            brelse(b);

            return i; 
        }
    }
    return 0; 
}

#define TOTAL_INODES 1024 
int inodeBitmap[TOTAL_INODES] = {0}; 


uint ialloc(uint dev, int type) {
    for (uint i = 1; i < TOTAL_INODES; i++) { 
        if (inodeBitmap[i] == 0) { 
            inodeBitmap[i] = 1; 

            struct dinode newInode;
            Lmemset(&newInode, 0, sizeof(struct dinode));
            newInode.type = type;
            newInode.nlink = 1; 

            
            uint blockno = IBLOCK(i, SB); 
            struct buf *bp = bread(dev, blockno);
            struct dinode *dip = (struct dinode *)(bp->data) + (i % IPB);
            *dip = newInode; 
            bp->dirty = 1;
            bwrite(bp);
            brelse(bp);

            return i; 
        }
    }
    return 0; 
}



