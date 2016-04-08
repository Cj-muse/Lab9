#include "header.h"

// 's' command
do_tswitch()
{}

// 'f' command
do_kfork()
{}

// 'p' command
do_ps()
{
		kprintstatus();
}

// 'q' command
do_exit()
{
	char str[50];
	int value = 0;

	//ask for an exitValue (value), e.g. 123
	printf("enter exit value: ");
	gets(str);
	value = strtoint(str);
	kexit(value);
}

// 'z' command
do_sleep()
{
	char str[64];
	int value = 0;

   // ask for an event (value), e.g. 123;
	printf("enter event value: ");
	gets(str);
	value = strtoint(str);

	printf("P%d going to sleep on event: %d \n\r", running->pid, value);
   ksleep(value);
}

// 'a' command
do_wake()
{
	char str[64];
	int value = 0;

	//ask for an event (value);
	printf("enter event value: ");
	gets(str);

	//string to int
	value = strtoint(str);
	printf("P%d waking proc sleeping onssevent: %d \n\r", running->pid, value);
	kwakeup(value);
}

// 'w' command
do_wait()
{
	int pid, status;
   pid = kwait(&status);

	if(pid == -1)
	{
		printf("do_wait: no child process..\n\r");
	}
	else
	{	//print pid and status;
		printf("do_wait(): pid = %d  status = %d \r\n", pid, status);
	}
}
