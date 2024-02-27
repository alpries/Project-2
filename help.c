#include "Llibc.h"
#include "Implt.h"

/* Help function for the OS */

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

//int
//Lmain(int argc, char *argv[])
//{
//  help();
//  return 0;
//}
