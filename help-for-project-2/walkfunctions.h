/* 
File walkfunctions.h

  Functions are described below.  Some functions here will need to
  walk through all the blocks of a file, given its inode.  
*/


int getinode(struct dinode *inode,  uint inodenum);
/*
  Find the inode struct for the given inode number, and fill in the
  caller supplied pointer to struct dinode.
  Return 0 on success, -1 on error.

  Used by several other functions!
*/


uint find_name_in_dirblock(uint blockptr, const char *nam);
/* 
  Assuming that blockptr points to a block of some directory file,
  find a directory entry with matching name and return its inode number.
  If no match, return 0.

  Calller must ensure that blockptr points to a block of a directory file.

  Used by find_dent() below.
*/


uint find_dent(uint inum, const char *name);
/*
  For the given inode number and the given name:
  If this inode is a directory file and there is a directory entry in it
  whose name matches the given name, return the inode number of that
  matching entry; else return 0

  Used by function namei() below.
*/


uint namei(const char *pathname);
/*
  Convert pathname to inode number (as in class/quiz).

  A fundamental filesystem algorithm!
*/


void lsdir(uint blockptr);
/*
  Print the list of directory entries in the block pointed to by blockptr.
  Caller must ensure that blockptr points to a block of a dirctory file.

  Easy function, but make sure to begin with bread() and end with brelse().

  Used by lspath().
*/


int lspath(const char *pathname);
/*
  If pathname is a directory, list its contents.

  Use namei, getinode, and lsdir().
*/


