#include "posix-calls.h"
#include "Llibc.h"
#include "Lcli.h"
#include "walkfunctions.h"


#define LINESIZE 1024   /* Line buffer size */
#define NTOKS 128       /* Max number of tokens in a line */
#define TOKEN_SIZE 100
#define MAX_NAME_LENGTH 256
#define STACK_SIZE 128

/* From Lbio.c */
void binit(void);
struct buf* bread(uint, uint);
void brelse(struct buf*);


/* For this File */
int parseLine(char **line, int len, char **token);
void help();



/*
Since we are only going to use a single device, and since its superblock
will not change, we can have a global int DEVFD after opening fs.img,
and a global struct suberblock SB!  Also, since throughout the duration
of the CLI, we will have to track CWD, we will have a global struct for it;
*/

/* Our globals */
int DEVFD;
struct superblock SB;

typedef struct{
	uint inum;
	char name[MAXPATH];
} CWD;

typedef struct{
    CWD entries[STACK_SIZE];
    int top;
} DirectoryStack;

DirectoryStack dirStack = {{{0}}, -1}; // Initialize the stack with an empty state


// More Prototype
void push(DirectoryStack *stack, CWD dir);
CWD pop(DirectoryStack *stack);
CWD peek(const DirectoryStack *stack);
void printStack(const DirectoryStack *stack);
int lsCommand(char *token[], int curr);


void
devfd_init(const char *devpath)
{
	/* First do this readonly to implement pwd, ls, cd, and upload commands */
	if ((DEVFD = Lopen(devpath, O_RDONLY)) < 0 ) {
		Lfprintf(2, "Could not open %s\n", devpath);
		Lexit(2);
	}
}

void
superblock_init(int devfd)
{
	struct buf *b;
	if ((b = bread(DEVFD, 1)) == 0) {
		Lfprintf(2, "Could not read superblock\n");
		Lexit(3);
	}
	Lprintf("The superblock was read:");
	Lprintf("  blockno = %d, refcnt = %d, valid = %d, dirty = %d\n",
		b->blockno, b->refcnt, b->valid, b->dirty);
	/* Populate SB */
	// struct superblock *s = &SB;
	Lmemcpy(&SB, &b->data[0], sizeof(struct superblock));
	brelse(b);
}

void
cwd_init(void)
{
	CWD newDir = {1, ""};
	Lstrcpy(newDir.name, "/");
	push(&dirStack,newDir); 
}

void
cwd_update(uint inum, const char *name){
	CWD newDir = {inum, ""};
	Lstrcpy(newDir.name, name);
	push(&dirStack,newDir);
}

int
Lmain(int argc, char *argv[])
{
	if (argc < 2) {
		Lprintf("Usage:  %s fs_img_path\n", argv[0]);
		return 1;
	}

	devfd_init(argv[1]);

	binit();

	superblock_init(DEVFD);

	cwd_init();

	/* Some code for test/debugging only!
	   bcache should never be directly accessed!

	int nbuf = 0;
	struct buf *b;
	for (b = bcache.head.next; b != &bcache.head; b = b->next){
		nbuf++;
	}
	Lprintf("nbuf = %d\n", nbuf);
	*/

	// Return a locked buf with the contents of the indicated block.
	/*
	struct buf *b;
	if ((b = bread(DEVFD, 1)) == 0)
		return 3;
	*/

	/*  Each call to bread increases refcnt for that buffer
		and each call to brelse will decrement refcnt.
		We are not using locking but the Lbio.c code depends on it,
		so make sure to brelse() after each bread() and each bwrite()
		so that refcnt resets to 0, or else even the LRU buffer
		may not get recycled!

		Also in original xv6 code, refcnt could get negative,
		so I patched Lbio.c so that refcnt stays >= 0.
		*/

	/*
	Lprintf("blockno = %d, refcnt = %d, valid = %d, dirty = %d\n",
		b->blockno, b->refcnt, b->valid, b->dirty);
	*/

	/* Dump block contents in hex */
	/*
	for (int k = 0; k < BSIZE; k++) {
		Lprintf("%02x ", b->data[k]);
		if ((k % 16) == 15 || k == BSIZE - 1)
			Lprintf("\n");
	}
	*/

	/* Analyze the superblock */
	struct superblock *s;
	// s = (struct superblock *) &b->data[0];
	s = &SB;

	Lprintf("Superblock magic = %08x\n", s->magic);
	Lprintf("FS device size = %d\n", s->size);
	Lprintf("Number of data blocks = %d\n", s->nblocks);
	Lprintf("Number of inodes = %d\n", s->ninodes);
	Lprintf("Block number for the first inode block = %d\n", s->inodestart);
	Lprintf("Block number for the first bitmap block = %d\n", s->bmapstart);

	Lprintf("Done with superblock for now!\n");

	/* */
	uint inodesize = sizeof(struct dinode);
	uint inodes_per_block = BSIZE / inodesize;
	Lprintf("Size of each inode = %d\n", inodesize);
	Lprintf("Number of inodes per block = %d\n", inodes_per_block);
	/* Mailman algorithm: Given inode number ino, its
		block number is:  				SB.inodestart + ino / inodes_per_block;
		byte offset within block is: 	inodesize * (ino % inodes_per_block);
	*/
	Lprintf("Dumping the first few inodes ...\n");
	int n = 4;
	/* Do this directly for now! */
	struct buf *b;
	// b = bread(fsdev_fd, SB.inodestart);
	b = bread(DEVFD, SB.inodestart);
	struct dinode *inode;
	for (int k = 0; k < n; k++) {
		inode = (struct dinode *) &b->data[k*inodesize];
		Lprintf("inode %d:\n", k);
		if (inode->type == 0) {
			Lprintf("  UNUSED (file type = 0)\n");
			continue;
		}
		Lprintf("  file type = %d\n", inode->type);
		Lprintf("  number of links = %d\n", inode->nlink);
		Lprintf("  file size = %u (bytes)\n", inode->size);
		Lprintf("  block map:\n");
		for (uint j = 0; j < NDIRECT && inode->addrs[j] != 0; j++)
		//	Lprintf("    direct block 0x%08x\n", inode->addrs[j]);
			Lprintf("    direct block decimal: %d, hexadecimal: 0x%08x\n", inode->addrs[j], inode->addrs[j]);
		if (inode->size > NDIRECT * BSIZE && inode->addrs[NDIRECT - 1] != 0)
			Lprintf("      indirect block 0x%08x\n", inode->addrs[NDIRECT]);
	}

	/* Directories */
	uint direntsize = sizeof(struct dirent);	/* 16 bytes */
	uint dirents_per_block = BSIZE / direntsize;
	Lprintf("Size of each dir entry = %d\n", direntsize);
	Lprintf("Number of dir entries per block = %d\n", dirents_per_block);

	brelse(b);

	/* Now list the current/root directory */
	// lspath(CWD.name);
        int flag = 0;
	char buf[LINESIZE];
	char *ptrBuf;
	//char *token[NTOKS] = {NULL};
	Lprintf("fscli> ");
	while (Lread(0, buf, LINESIZE) > 0 && flag == 0) {
		char *token[NTOKS] = {NULL};
		/* With the terminal in line buffered mode, buf will hold
			the '\n' character indicating the end of line
		   */
		
		/* Analyze line typed by user ... */
			//Write your code here!
		if (buf[0] != '\n'){
     			ptrBuf = buf;
     			// Parses Line to get the token
     			parseLine(&ptrBuf, Lstrlen(buf),token);
 
     			// Loops through the Line token
     			for(int j =0; j < 30; j++){
				// Makes sure the tokens arent null
       				if(token[j] == NULL){
	 				break;
       				}
       				/* Help Command was Called
				 Has \n because when user inputs help they have to click enter(\n)
				 Handles \"help\" command*/
       				if(Lstrcmp(token[j], "help") == 0){
 	 				help();
	 				break;
      			 	}
				// Handles pwd command
				if (Lstrcmp(token[j], "pwd") == 0){
					//Lprintf("%s\n", CWD.name);
					printStack(&dirStack);
					Lprintf("\n");
					break;
				}
				if (Lstrcmp(token[j], "cd") == 0){
					uint cdresult = cd(dirStack.entries[dirStack.top].inum, token[j+1]);
					if (cdresult == -1){
						Lprintf("Directory does not exist\n");
					       break;	
					}
					cwd_update(cdresult, token[j+1]);
					//Lprintf("Worked: %d\n", cdresult);
					break;
				}

				if(Lstrcmp(token[j], "find") == 0){
					Lprintf("Returned Inode: %d\n", find_name_in_dirblock(Latoi(token[j+1]),token[j+2]));
					break;
				}

				if(Lstrcmp(token[j], "dent") == 0){
					Lprintf("Returned Inode: %d\n", find_dent(Latoi(token[j+1]), token[j+2]));
					break;
				}
				if(Lstrcmp(token[j], "path") == 0){
					Lprintf("Returned Inode: %d\n", namei(token[j+1]));
					break;
				}
				
				if(Lstrcmp(token[j], "ls") == 0){
					int lsResult = lsCommand(token,j);
					if (lsResult == -1){
					    Lprintf("Directory does not exist\n");
					    break;
					}
					break;
			
				}

				if (Lstrcmp(token[j], "lspath") == 0){
					lspath(token[j+1]);
					break;
				}
				if(Lstrcmp(token[j], "quit") == 0){
					flag = 1;
					break;
				}
	
       				// Display an invalid message when token doesn't match an action
       				Lprintf("Invalid Command\n");
       				break;
    		 	}
  		}
		/*
			If command was pwd:

				Lprintf("%s\n", CWD.name);
		
			If command was ls, without any argument params:
				lspath(CWD.name);

			If command was ls, with argument param path1:
				lspath(path1);	// Handle errors, repeat for path2, path3, ...

			If command was cd, without any argument params:
				CWD.inum = 1;
				Lstrcpy(CWD.name, "/");

			If command was cd, with argument param path:
				// Strip extra repeated slashes, then
				uint in = namei(path);
				struct dinode inode;
				inode.type = 0;
				if (in != 0)
					getinode(&inode, in);
				if (inode.type == T_DIR) {
					// Try to do the cd, i.e., update struct CWD properly
					// Mostly string processing here
				} else {
				Lprintf("Could not cd to %s (not a directory)\n", path);
				}
		*/

		for(int i = 0; i< LINESIZE; i++){
			buf[i] = 0;
		}
		if (flag == 1){
			break;
		} else {
			Lprintf("fscli> ");
		}
	}
	//Lprintf("\n");
	return 0;

}

// Takes the Line command, as well as the size
/*****************************
 * IMPLEMENTING PARSELINE
 ****************************/
int
parseLine(char **line, int len, char **token){
    int tokenPos = 0;
    int inToken = 0;
    for(int i = 0; i < len;i++){

	if ((*line)[i] == '\n') {
            (*line)[i] = '\0'; // Replace '\n' with '\0'
            if (inToken == 1) {
                inToken = 0; // End the current token
            }
            break; // Exit the loop when '\n' is found
        }

        // Addd \0 before the token
        if ((*line)[i] != ' ' && inToken == 0){
            token[tokenPos++] = &(*line)[i];
            inToken = 1;
        }

        if ((*line)[i] == ' ' && inToken == 1){
            (*line)[i] = '\0';
            inToken = 0;
	}
    }
    return 1;
}
/*****************************
 * IMPLEMENTING LS COMMAND
 ****************************/

int
lsCommand(char *token[], int curr){
	struct dinode inode;
	uint targetInum;
	if (token[curr+1] != NULL){
		Lprintf("Extra word: %s\n", token[curr+1]);
	}


	if (token[curr+1] == NULL || Lstrcmp(token[curr+1], ".") == 0) {
    		targetInum = dirStack.entries[dirStack.top].inum;
	} else if (Lstrcmp(token[curr+1], "..") == 0) {
		if (dirStack.top == 0) {
			Lprintf("Cannot go above root\n");
			return -1;
		} else {
			targetInum = dirStack.entries[dirStack.top - 1].inum;
		}
	}else if(token[curr+1] != NULL) {
		uint cdresult = cd(dirStack.entries[dirStack.top].inum, token[curr+1]);
		if (cdresult == -1){
			return -1;
		} 
		targetInum = cdresult;
	} else {
		Lprintf("Path not supported\n");
		return -1;
	}


	int result = getinode(&inode, targetInum);
	if (result == -1 || inode.type != T_DIR){
		return -1;
	}
	for(int i = 0; i < 13; i++){
		if (inode.addrs[i] == 0){
			return 0;
		}
		lsdir(inode.addrs[i]);
	}
	return 0;
}

/*****************************
 * IMPLEMENTING HELP COMMAND
 ****************************/
void
help(){
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
  Lwrite(1,"| Command   | Description                                            |\n",72);
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
  Lwrite(1,"| help      | Print this table                                       |\n",72);
  Lwrite(1,"| pwd       | Show current directory (path and inode)                |\n",72);
  Lwrite(1,"| cd [path] | Change directory to path (or to /)                     |\n",72);
  Lwrite(1,"| ls [-d]   | List path as in ls -ail                                |\n",72);
  Lwrite(1,"| [-R] [path]|                                                       |\n",72);
  Lwrite(1,"| creat path| Create file at path (like touch)                       |\n",72);
  Lwrite(1,"| mkdir path| Create directory at path                               |\n",72);
  Lwrite(1,"| unlink path| Like rm and rmdir                                     |\n",72);
  Lwrite(1,"| link      | Like ln                                                |\n",72);
  Lwrite(1,"| oldpath   | newpath                                                |\n",72);
  Lwrite(1,"| sync      | Write all cached dirty buffers to device blocks        |\n",72);
  Lwrite(1,"| quit      | Exit CLI (should also sync)                            |\n",72);
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
  Lwrite(1,"| Additional CLI commands:                                           |\n",72);
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
  Lwrite(1,"| Command   | Description                                            |\n",72);
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
  Lwrite(1,"| uploadtree| Dump current ls -R / output text to host               |\n",72);
  Lwrite(1,"| filename  |                                                        |\n",72);
  Lwrite(1,"| upload    | Copy path in filesystem image to host                  |\n",72);
  Lwrite(1,"| path      | filename                                               |\n",72);
  Lwrite(1,"| download  | Copy a host file into filesystem image at path         |\n",72);
  Lwrite(1,"| filename  | path                                                   |\n",72);
  Lwrite(1,"+-----------+--------------------------------------------------------+\n",72);
}

/****************************
 * IMPLEMENTING STACK
 ***************************/

// Push a directory onto the stack
void push(DirectoryStack *stack, CWD dir) {
    // checks if stack is full
    if (stack->top < STACK_SIZE - 1) {
        stack->top++;
        stack->entries[stack->top] = dir;
    } else {
        //Do something when the stack is full
    }
}

// Pop a directory from the stack
CWD pop(DirectoryStack *stack) {
    if (stack->top >= 0) {
        CWD dir = stack->entries[stack->top];
        stack->top--;
        return dir;
    } else {
        // Return something else when stack is empty
        return (CWD){0, ""}; // Return an empty directory entry
    }
}

// Peek at the top directory on the stack without popping it
CWD peek(const DirectoryStack *stack) {
    if (stack->top >= 0) {
        return stack->entries[stack->top];
    } else {
        // Return something else when a stack is empty
        return (CWD){0, ""}; // Return an empty directory entry
    }
}

// print the current stack for pwd command
void printStack(const DirectoryStack *stack) {
    for (int i = 0; i <= stack->top; i++) {
        if (i<2){
            Lprintf("%s", stack->entries[i].name);
        }
        else{
            Lprintf("/%s", stack->entries[i].name);
        }

    }
   // Lprintf("\n");
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}


