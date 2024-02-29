// On-disk file system format.
// Both the kernel and user programs use this header file.

/* Also see param.h and Lstat.h */

#define ROOTINO  1   // root i-number
#define BSIZE 1024  // block size

// Disk layout:
// [ boot block | super block | log blocks | inode blocks |
//                                             block bitmap | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint magic;        // Must be FSMAGIC
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

#define FSMAGIC 0x10203040

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

/*
			  xv6 inode
	+------------------------------+
	| 2 bytes: File type           | 1 = dir, 2 = reg file (0 = inode free)
	+------------------------------+
	| 2 bytes: Major               |
	+------------------------------+
	| 2 bytes: Minor               |
	+------------------------------+
	| 2 bytes: Number of links     |
	+------------------------------+
    | 4 bytes: Size (bytes)        |
	+------------------------------+
    | 4 bytes: 1st direct ptr      |
	+------------------------------+
    | 4 bytes: 2nd direct ptr      |
	+------------------------------+
	.                              .
	.                              .
	+------------------------------+
    | 4 bytes: 12th direct ptr     |
	+------------------------------+
    | 4 bytes: Single Indirect ptr |
	+------------------------------+

*/

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

/*
		xv6 directory entry (each):

	  	2 bytes			14 bytes
	+-----------+------------------------+
	| inode-num |		filename         |
	+-----------+------------------------+
*/

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

