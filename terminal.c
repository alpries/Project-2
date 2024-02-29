#include "posix-calls.h"
#include "Llibc.h"
#include "Implt.h"

#define TOKEN_SIZE 100

int parseLine(char **line, int len, char **token);
// Peek at the top directory on the stack without popping it
int cdDir(char *dir, int currInode);

/* Starts the Terminal*/

int 
getcmd(char *buf, int nbuf)
{
  Lwrite(2, "batcave$ ", 9);
  Lmemset(buf, 0, nbuf);
  Lgets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
Lmain(int argc, char *argv[]){
 static char buf[100];
 char *ptrBuf;
 int fd;
 char *token[TOKEN_SIZE];

 // Check the file descriptor is open.
 while((fd = Lopen("console",0)) >= 0){
   if(fd >= 3){
    Lclose(fd);
    break;
   }
 }

 // Reads and Runs commands inputted.
 while(getcmd(buf, sizeof(buf)) >= 0){ // Reads input from the console starting from $
  if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){ //Checks if its 'cd '
      // Chdir must be called by the parent, not the child.
      buf[Lstrlen(buf)-1] = 0;  // chop \n
     // if(chdir(buf+3) < 0)
     //   Lfprintf(2, "cannot cd %s\n", buf+3);
      continue;
    }

  if (buf[0] != ' '){
     ptrBuf = buf;
     // Parses Line to get the token
     parseLine(&ptrBuf, Lstrlen(buf),token);
 
     // Loops through the Line token
     for(int j =0; j < 30; j++){
	// Makes sure the tokens arent null
       if(token[j] == NULL){
	 break;
       }
        // Help Command was Called
	// Has \n because when user inputs help they have to click enter(\n)
       if(Lstrcmp(token[j], "help\n") == 0){
 	 help();
	 break;
       }
	
       // Display an invalid message when token doesn't match an action
       Lprintf("Invalid Command\n");
       break;
     }
  }

  

 }
 return 1;
}

// Takes the Line command, as well as the size
int
parseLine(char **line, int len, char **token){
    int tokenPos = 0;
    int inToken = 0;
    
    for(int i = 0; i < len;i++){
        if((*line)[i] == '\n'){
            break;
        }
        
        if ((*line)[i] != ' '){
            if (inToken == 0){
		if (i > 0){
                  (*line)[i-1] = '\0';
		}
                token[tokenPos++] = &(*line)[i];
            }
            
            inToken = 1;
        }
        
        if ((*line)[i] == ' '){
            if (inToken == 1){
                 (*line)[i] = '\0';
            }
            inToken = 0;
        }
    }
    return 1;
}

// Peek at the top directory on the stack without popping it
DirectoryEntry peek(const DirectoryStack *stack) {
    if (stack->top >= 0) {
        return stack->entries[stack->top];
    } else {
        printf("Stack is empty\n"); // Return something else when a stack is empty
        return (DirectoryEntry){0, ""}; // Return an empty directory entry
    }
}
