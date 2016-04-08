#include "header.h"

int getInodeFromFile(char *filename)
{
  char path[10][32];
  int numberOfFiles = 0;
  int i = 0, ino = 0;
  u16 blk;

  //printf("GetInodeNumberFromFile() \n");
  //printf("filename = %s\n", filename);

  //parse filename into a char *path[MAX]
  numberOfFiles = parseInput(filename, path, "/");

  getblk(2, buffer); // get the group descriptor
  gp = (GD *)buffer;
  InodeBeginBlk = (u16)gp->bg_inode_table; //get the inode begin block
  //printf("InodeBeginBlk=%d\n", InodeBeginBlk);
  getblk(InodeBeginBlk, buffer);  /* read first inode block */
  ip = (INODE *)buffer +1; // now you have the root INODE
  //printf("ip->i_mode=%x\n", ip->i_mode);

  //search for file using the path.
  ino = findInode(path);
  return ino;
}

int findInode(char path[10][32])
{
    int i = 0, j = 0, ino=0;

    while (strcmp(path[i], "") != 0)
    {
      //printf("path[%d] = %s\n", i, path[i]);
      ino = search(path[i]);
      if (ino ==-1)
      {
        printf("seach could not find file\n" );
        return 0;
      }
      ip = getINODE(ino);
      i++;
    }
    //printf("ino = %d\n", ino);
    return ino;
}

u16 search(char name[32])
{
  int i; char c; DIR *d;
  printf("searching for %s\n", name);
  for (i=0; i<12; i++){ // assume a DIR has at most 12 direct blocks
    if ( (u16)ip->i_block[i] ){
      getblk((u16)ip->i_block[i], buffer2);
      d = (DIR *)buffer2;
      while ((char *)d < &buffer2[1024]){
        c = d->name[d->name_len]; // save last byte
        d->name[d->name_len] = 0; // make name into a string
        prints(d->name); putc(' '); // show dp->name string
        if ( strcmp(d->name, name) == 0 ){
          prints("\n\r");
          return((u16)d->inode);
        }
        d->name[d->name_len] = c; // restore last byte
        d = (char *)d + d->rec_len;
      }
    }
 }
 return -1;
}

INODE *getINODE(int ino)
{
	int offset = 0;
	int inoblock=0;
	u16 blocknumber;
	INODE * inode;

	//printf("In getINODE ino = %d\n", ino);
	//printf("InoBeginBlk = %d\n", InodeBeginBlk);

	inoblock = (ino-1)/8;
	offset = (ino-1)%8;

	blocknumber = inoblock + InodeBeginBlk;
	get_block(blocknumber, buffer3);

	//printf("InodeBeginBlk = %d\n", InodeBeginBlk);

	// block is now in buffer
	// access specific inode with offset
	inode = (INODE *)buffer3 + offset;
	//printf("returning an inode %d ", ino);
	//printf("at address %x from buffer %x\n",inode,buffer);
	return inode;
}



int chopFirstStringElement(char path[10][32])
{
  int i = 0, j = 0;
  char temp[32];

  /*while (strcmp(path[j], "") != 0)
  {
    printf("chop(): path[%d] = %s\n", j, path[j]);
    j++;
  }*/

  while (strcmp(path[i], "") != 0)
  {
    if (i)
    {
      strcpy(temp, path[i]);
      strcpy(path[i-1], temp);
    }
    i++;
  }
  //make sure to null last string
  strcpy(path[i-1], "");
  return 1;
}

int parseInput(char *input, char parsedinput[10][32], char *delimiter)
{
	int i = 0;
	char *token = "0";
	char line[MAX];
	strcpy(line, input);

  //get first token from strtok
  token = strtok(line, delimiter);
  strcpy(parsedinput[i],token);

  //get the rest of the tokens and put into input
  i = 1;
  while((token = strtok(NULL, delimiter))!= NULL )
  {
	   //parsedinput[i] = (char*)calloc(MAX, sizeof(char));
	   strcpy(parsedinput[i],token);
     i++;
  }

  parsedinput[i][0] = 0;
  //number of seperate filenames returned
  return i;
}
