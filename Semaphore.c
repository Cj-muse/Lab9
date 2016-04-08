#include "header.h"

extern int int_off(), int_on();

int P(SEMAPHORE *s)
{
    int sr;

    sr = int_off();
    printf("in P\n");
    s->value--;
    if (s->value < 0)
    {
       //printf("proc %d blocked in P() at semaphore = %x\n",unning->pid, s);
       running->sem = s;   // blocked on this semaphore
       running->status = BLOCK;
        int_on(sr);
       enqueue(&(s->queue), running);

      // tswitch();
    }
    printf("in P\n");
   
}

int V(SEMAPHORE *s)
{
    int sr; PROC *p;
    sr = int_off();

    s->value++;
    if (s->value<=0)
    {
       p = dequeue(&(s->queue));
       p->status = READY;
       p->sem = 0;               // no longer blocked at this semaphore
       enqueue(&readyQueue, p);
       //tswitch();
       // printf("proc %d unblocked in V()\n", p->pid); 
    }
    int_on(sr);
}
