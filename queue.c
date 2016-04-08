#include "header.h"

PROC *get_proc(PROC **list, int status)     // e.g. get_proc(&freeList);
{
   PROC *proc = *list, *previous = 0;

  //printf("get_proc(): status = %d \n\r", status);
	while (proc)
	{
		if(proc->status == status)
		{
			if(previous) //not at start
			{
				previous->next = proc->next;
				proc->next = 0;
				return proc;
			}
			*list = proc->next;
			proc->next = 0;
			return proc;
		}
		previous = proc;
		proc = proc->next;
	}
   printf("No proc with status %d available in given list\n\r", status);
   return 0;  // return 0 if no more FREE PROCs
}

int put_proc(PROC **list, PROC *p)  // e.g. put_proc(&freeList, p);
{
   PROC *temp = *list;

	//printf("put_proc(): pid %d status %d \n\r", p->pid, p->status);
   // printList("readyqueue", readyQueue);

	if(!temp)
	{
		//printf("adding proc to null list\n\r");
		p->next = 0;
		*list = p;
      return 1;
	}

	// traverse to end
   while(temp->next)
   {
   	temp = temp->next;
   }
   // enter p into *list;
	p->next =  temp->next;
   temp->next = p; // enter p into *list;
   //p->status = FREE;

    //printList("readyqueue", readyQueue);

   return 0;
}

int enqueue(PROC **queue, PROC *p) //: enter p into queue by priority
{
	PROC *proc = *queue;
	PROC *previous = 0;
  //printf("enqueue\n");
	//check for empty queue
	if (proc == 0)
	{
		// insert p as first proc
		*queue = p;
      printf("Enqueued proc %d\r\n", p->pid);
      return 1;
	}

	//must move at least past the first
	if (p->priority > proc->priority)
	{
		p->next = proc;
		*queue = p;
	}

	// make sure that we are not at the end of the queue
  //printf("proc->next = %d\n\r", proc->next);
  //getc();
	while (proc->next != 0)
	{
     // printf("enqueue while\n\r");
     //printf("proc->pid = %d\n\r", proc->pid);
     //getc();
		if(p->priority > proc->priority)	// Highest priority first
		{
			// insert p
			// P1->P2->P3
			// P1-> p ->P2->P3
			p->next = proc;
			previous->next = p;
			printf("enqueue successful\n\r");
			return 1;
		}
		previous = proc;
		proc = proc->next;
	}

	// p is the lowest priority so place on end of queue
  //printf("Proc is lowest priority: %d\n\r",p->priority);
	p->next = proc->next;
	proc->next = p;
}

PROC *dequeue(PROC **queue) //: return first element removed from queue
{
	// remove a PROC with the highest priority (the first one in queue)
	// return its pointer;
	PROC *proc = *queue;

	*queue = proc->next;
	proc->next = 0;

  printf("returning p%d\n\r",proc->pid);
	return proc;
}

int printList(char *name, PROC *list) //: print name=list contents
{
	/* print the queue entries in the form of [pid, prioirty]-> ... ->NULL*/
   int count = 0;
	printf("%s = ", name);

	while (list && (count < NPROC))
	{
		printf("[%d, %d]->", list->pid, list->priority);
      list = list->next;
      count++;
	}
	printf("NULL\n\r");
}

void showLists()
{
  printList("freeList", freeList);
  printList("readyQueue", readyQueue);
  printList("sleepList", sleepList);
}
