#include "header.h"
/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/

int body(void)
{
   char c, str[64];

   printf("proc %d resumes to body()\n\r", running->pid);
   while(1)
   {
      printf("\rproc %d running : enter a key [s|f|z|a|w|q|u|p|l]: ", running->pid);
      c = getc();
      printf("%c\n\r", c);
      switch(c)
      {
         case 's': tswitch();  break;
         case 'q': do_exit();  break;
         case 'f': kfork("/bin/u1");    break;
         case 'z': do_sleep(); break;
         case 'a': do_wake();  break;
         case 'w': do_wait();  break;
         case 'u': goUmode();  break;
         case 'p': do_ps();    break;
         case 'l': showLists();break;
         default: break;
      }
   }
}

int init()
{
    PROC *p; int i, j;
    color = 0x0C;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];

        for (j=0; j<NFD; j++)
          p->fd[j] = 0;
    }

    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    for (i=0; i<NOFT; i++)
        oft[i].refCount = 0;
    for (i=0; i<NPIPE; i++)
        pipe[i].busy = 0;


    /**** create P0 as running ******/
    p = get_proc(&freeList, FREE);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    running->status = READY;
    printf("done\n\r");
}

int scheduler()
{
  printf("made it to scheduler\n");
  if (running->status == READY)
	{
   	enqueue(&readyQueue, running);
	}
   running = dequeue(&readyQueue);
   color = running->pid + 1;
   printf("leaving scheduler\n");
}

int tinth(); // declare timerInteruptHandler as a function 
int int80h();
int kbinth();

int set_vector(u16 vector , u16 handler)
{
	 put_word(handler, 0, vector<<2);
   put_word(0x1000,  0,(vector<<2) + 2);
}

int setRuntime()
{
   int i = 0;
   PROC *p;
   
   for (i = 0; i < NPROC; i++)
   {
      p = &proc[i];
      if(p->status == READY)
      {
         p->runtime = 5;
      }
   }
}

main()
{
    printf("MTX starts in main()\n\r");
    vid_init();
    init();      // initialize and create P0 as running
    set_vector(80, int80h);
   
   kfork("/bin/u1");     // P0 kfork() P1
	//kfork("/bin/u1");     // P0 kfork() P2
	//kfork("/bin/u1");     // P0 kfork() P3
	//kfork("/bin/u1");     // P0 kfork() P4	*/

   // install KBD interrupt handler, initilize kbd driver
   set_vector(9, kbinth); 
   kbd_init();

	lock();
	setRuntime();
	set_vector(8, tinth); // install address of tinth() to vector 8 
	timer_init();
	
	//ktest();
    while(1){
      printf("P0 running\n\r");
      while(!readyQueue);
      printf("P0 switch process\n\r");
      tswitch();         // P0 switch to run P1
   }
}
