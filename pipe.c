#include "header.h"
show_pipe(PIPE *p)
{
   int i, j;
   //validate pipe
   if (!pipe)
	{
		printf("show_pipe(): Invalid Pipe\n");
   }
   // print pipe information
   printf("------------ PIPE %d CONTENTS ------------\n", p);
   printf("head: %d\n", p->head);
   printf("tail: %d\n", p->tail);
   printf("data: %d\n", p->data);
   printf("room: %d\n", p->room);
   printf("nreader: %d\n", p->nreader);
   printf("nwriter: %d\n", p->nwriter);
   printf("busy: %d\n", p->busy);
   printf("buffer: %s\n", p->buf);
   printf("----------------------------------------\n");
}

//char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
	int i = 0, j =0, mode = 0;
	char c;
	
  	// print running process' opened file descriptors
	printf("-----------Proc %dOpen File Decsriptors-----------\n", running->pid);
 	for(i=0; i<20; i++)
 	{
		if(running->fd[i])
		{
	   		printf("fd[%d]:     ",i);
	
			//print mode
			mode = running->fd[i]->mode;
			if(mode == 4)				{printf("Mode: read     ");}
			else if(mode == 5)		{printf("Mode: write    ");}
			else 							{printf("Mode: unknown  ");}
    
		   printf("refCount: %d  ",   running->fd[i]->refCount);
			printf("Pipe data: %d\n", running->fd[i]->pipe_ptr->data);
			printf("buf:");
			while(c)
			{
				//c = get_byte(running->uss, running->fd[i]->pipe_ptr->buf+j);
				c =  running->fd[i]->pipe_ptr->buf[j];
				printf("%c", c);
				j++;
			}
			j = 0;c = 'a';printf("\n");
		}	
   }
  	if (0 == i) {printf("NONE\n");}
  	printf("--------------------------------------------------\n");
}

int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe()
	int r = 0;
	char c;
  OFT *oft = running->fd[fd];
  PIPE *p = oft->pipe_ptr;
  if (n<=0)
  return 0;

  //perform checks on pipe ptr and open file table
 	if (!oft) {
   	printf("read_pipe: not a valid fd\n");
    	return -1;
  	}
  	if (!p) {
    	printf("read_pipe: not a valid pipe_ptr\n");
    	return -1;
  	}
	//make sure you are using the oft of the correct mode
	if(oft->mode != READ_PIPE)
	{
		printf("fd is of wrong mode \n");
		return -1;
	}

  	printf("ReadPipe: \n");
	printf("fd = %d\n", fd);
	printf("buf: %x\n", buf);
	printf("p->buf %x\n", p->buf);
  	//show_pipe(p);

  	//validate fd; from fd, get OFT and pipe pointer p;
  	while(n)
  	{
    	while(p->data)
    	{//read a byte from pipe to buf;
			c = p->buf[p->tail++];
			put_byte(c, running->uss, buf);
         p->tail %= PSIZE;
         p->data--; p->room++;
         n--; r++; buf++;

      	//buf[r] = p->buf[r];
			//where is buf in relation to user space and kernal space?
			//c = get_byte(running->uss, p->buf+r);
			//c = p->buf+r;
			printf("got byte %c\n", c);
			//put_byte(c, running->uss, buf+r);
      	//n--; r++; p->data--; p->room++;
		
			if (n==0)  break;
    }
    if (r){ // has read some data
      kwakeup(&p->room);
      return r;
    }
    // pipe has no data
    if (p->nwriter){ // if pipe still has writer
      kwakeup(&p->room); // wakeup ALL writers, if any.
      ksleep(&p->data); // sleep for data
      continue;
    }
    // pipe has no writer and no data
    return 0;
  }
}

int write_pipe(int fd, char *buf, int n)
{
   int r = 0, i = 0;
	char c;
   OFT *oft = running->fd[fd];
   PIPE *p = oft->pipe_ptr;

	printf("WritePipe:\n");	
	printf("fd %d", fd);
	printf("buf passed: %x\n", buf);
	printf("p->buf %x\n", p->buf);
	for(i = 0; i<n; i++)
	{
		c = get_byte(running->uss, buf+i);
		printf("c=%c ", c);
	}


  	if (n<=0)
  	return 0;

	//validate fd; from fd, get OFT and pipe pointer p;	
 	//perform checks on pipe ptr and open file table
  	if (!oft) {
    	printf("write_pipe: not a valid fd\n");
    	return -1;
  	}
  	if (!p) {
    	printf("write_pipe: not a valid pipe_ptr\n");
   	return -1;
  	}	
	//make sure you are using the oft of the correct mode
	if(oft->mode != WRITE_PIPE)
	{
		printf("fd is of wrong mode \n");
		return -1;
	}

  while (n)
  {
    	if (!p->nreader) // no more readers
    	  kexit(BROKEN_PIPE); // BROKEN_PIPE error
	   while(p->room)
		{
			//write a byte from buf to pipe;
			printf("writing byte\n");
			p->buf[p->head++] = get_byte(running->uss, buf);
	      p->head %= PSIZE;
   	   p->data++; p->room--;
      	n--; r++; buf++;
			//c = get_byte(running->uss, buf+r);
			//put_byte(c, running->uss, p->buf + r);
			//p->buf[r] = c; 
      	//r++; p->data++; p->room--; n--;
      	if (n==0)
      	break;
    	}

    	kwakeup(&p->data); // wakeup ALL readers, if any.
    	if (n==0)
    	return r; // finished writing n bytes

    	// still has data to write but pipe has no room
    	ksleep(&p->room); // sleep for room
  }
}

int kpipe(int pd[2])
{
  int i = 0;
  PIPE *p;
  OFT *readFT, *writeFT;

  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  p = initPipe();
  printf("p->busy: %d\n", p->busy);
  printf("p->data: %d\n", p->data);
  readFT = initOFT(READ_PIPE, p);
  writeFT = initOFT(WRITE_PIPE, p);

  //  Allocate 2 free entries in the PROC.fd[] array,
  for (i=0; i < NFD-1; i++)
  {
		if (running->fd[i] == 0 && running->fd[i+1] == 0)
    {
		    running->fd[i]   = readFT;
        running->fd[i+1] = writeFT;
        break;
    }
  }

  // set indicies of running procs fd's to pd[]
  //pd[0] = i;
  //pd[1] = i+1;

  	/* fill user pipe[] array with i, i+1 */
 	put_word(i, running->uss, &pd[0]);
	put_word(i+1, running->uss, &pd[1]);

  	printf("returning from kpipe\n");
  	//getc();
  	return 0;
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  if (!op) // validate oft pointer
  {
    printf("sys: close_pipe(): invalid file descriptor\n");
    return -1;
  }

  running->fd[fd] = 0;                 // clear fd[fd] entry
  pp = op->pipe_ptr;

  if (op->mode == READ_PIPE){
      pp->nreader--;                   // dec n reader by 1

      if (--op->refCount == 0){        // last reader
	        if (pp->nwriter <= 0){         // no more writers
	           pp->busy = 0;             // free the pipe
             return;
           }
      }
      //was wakeup() not sure if necessary
      kwakeup(&pp->room);               // wakeup any WRITER on pipe
      return;
  }
  else
  { // YOUR CODE for the WRITE_PIPE case:
    pp->nwriter--;                   // dec n writers by 1

    if (--op->refCount == 0){        // last  writer
      if (pp->nreader <= 0){         // no more readers
           pp->busy = 0;             // free the pipe
           return;
      }
    }
    kwakeup(&pp->data);           // wakeup any READERS on pipe
    return;
  }
}

PIPE *initPipe()
{
  int i = 0;

  printf("Initializing pipe\n");
  for (i=0; i<NPIPE; i++)
  {
    printf("pipe[%d]: %d\n",i,&pipe[i]);
    if (pipe[i].busy == 0)
    break;
  }
  pipe[i].busy = 1;
  pipe[i].head = pipe[i].tail = pipe[i].data = 0;
  pipe[i].nwriter = pipe[i].nreader = 1;
  pipe[i].room = PSIZE;
  return &pipe[i];
}

OFT *initOFT(int mode, PIPE *p)
{
  int i = 0;

  for (i=0; i<NOFT; i++){
      if (oft[i].refCount == 0) break;
  }

	//printf("in initoft, mode = %d\n", mode);
	oft[i].mode = mode;
  	oft[i].refCount = 1;
  	oft[i].pipe_ptr = p;
  	return &oft[i];
}
