#include "header.h"

int copyImage(u16 child_segment)
{
  u16 i;
  u16 pseg = running->uss, cseg = child_segment, size = 32*1024;

  for (i=0; i<size; i++)
  {
      // copy parents image to the given segments image
      put_word(get_word(pseg, 2*i), cseg, 2*i);
  }
}

int fork()
{
  int i, pid;  u16 segment;

  PROC *p = kfork(0);   // kfork() but do NOT load any Umode image for child
  if (p == 0)           // kfork failed
    return -1;

    //printf("in fork after kfork\n");
    //getc();

  segment = (p->pid+1)*0x1000;
  copyImage(segment);

  // YOUR CODE to make the child runnable in User mode
  p->uss = segment; // child’s own segment
  p->usp = running->usp; // same as parent's usp

  //*** change uDS, uES, uCS, AX in child's ustack ****
  put_word(segment, segment, p->usp); // uDS=segment
  put_word(segment, segment, p->usp+2); // uES=segment
  put_word(0, segment, p->usp+2*8); // uax=0
  put_word(segment, segment, p->usp+2*10); // uCS=segment

   /**** Copy file descriptors ****/
   for (i=0; i<NFD; i++)
   {
      p->fd[i] = running->fd[i];
      if (p->fd[i] != 0)
      {
        p->fd[i]->refCount++;
        if (p->fd[i]->mode == READ_PIPE)
            p->fd[i]->pipe_ptr->nreader++;
        if (p->fd[i]->mode == WRITE_PIPE)
            p->fd[i]->pipe_ptr->nwriter++;
      }
   }

   printf("fork(): forked child %d\n", p->pid);
   return(p->pid);
}

int exec(char *filename)
{
  // your exec function
}

int kexec(char *y) // y points at filenmae in Umode space
{
 int i, length = 0;
 char filename[64], *cp = filename;
 u16 segment = running->uss; // same segment

 /* get filename from U space with a length limit of 64 */
 while( (*cp++ = get_byte(running->uss, y++)) && length++ < 64 );
 if (!load(filename, segment)) // load filename to segment
 {return -1;} // if load failed, return -1 to Umode

 /* re-initialize process ustack for it return to VA=0 */
 for (i=1; i<=12; i++)
 put_word(0, segment, -2*i);
 running->usp = -24; // new usp = -24

 /* -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 ustack layout */
 /* flag uCS uPC ax bx cx dx bp si di uES uDS */
 put_word(segment, segment, -2*12); // saved uDS=segment
 put_word(segment, segment, -2*11); // saved uES=segment
 put_word(segment, segment, -2*2); // uCS=segment; uPC=0
 put_word(0x0200, segment, -2*1); // Umode flag=0x0200
}
