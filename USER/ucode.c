// ucode.c file
char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", 
	"fork", "exec", "pipe", "pfd", "read", "write", "close", "sleep", 0};

int show_menu()
{
   printf("******************** Menu ***************************\n");
   printf("*  ps  chname  kmode  switch  wait  exit  fork  exec *\n");
          //   1     2      3       4      5     6    7     8
   printf("*  pipe  pfd   read   write   close   sleep           *\n");
	  	  //   9     10    11      12     13      14
   printf("*****************************************************\n");
}

int find_cmd(name) char *name;
{
   int i = 0;
   char *p = cmd[0];

   while (p){
     if (!strcmp(p, name))
        return i;
     p = cmd[++i];
   }
   return(-1);
}

char getc()
{
   char c;
    printf("in user getc\n");
   c = syscall(90,0,0,0);
   return c;
}


int putc(char c)
{
	return syscall(91,c,0,0);
}

int sleep()
{
	char s[32];
	int t, totaltime;
	
	printf("enter number of seconds to sleep: ");
	gets(s);
	sscanf(s, "%d", &t);
	printf("going to sleep for %d seconds\n", t);
   syscall(36,t,0,0);
	
	//call kernal funtion here
	syscall(37, getpid(), 0, 0);
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   syscall(1, 0, 0);
}

int chname()
{
    char s[64];
    //printf("\ninput new name : ");
    //gets(s);
    syscall(2, s, 0);
}

int kmode()
{
    printf("kmode : enter Kmode via INT 80\n");
    printf("proc %d going K mode ....\n", getpid());
        syscall(3, 0, 0);
    printf("proc %d back from Kernel\n", getpid());
}

int kswitch()
{
    printf("proc %d enter Kernel to switch proc\n", getpid());
        syscall(4,0,0);
    printf("proc %d back from Kernel\n", getpid());
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n");
    return child;
}

int exit()
{
   char exitValue;
   printf("enter an exitValue (0-9) : ");
   exitValue=(getc()&0x7F) - '0';
   printf("enter kernel to die with exitValue=%d\n",exitValue);
   _kexit(exitValue);
}

int _kexit(int exitValue)
{
  syscall(6,exitValue,0);
}

int fork()
{
  int child;
  child = syscall(7,0,0,0);
  if (child)
    printf("parent %d return form fork, child=%d\n", getpid(), child);
  else
    printf("child %d return from fork, child=%d\n", getpid(), child);
}

int exec()
{
  int r;
  char filename[32];
  printf("enter exec filename : ");
  gets(filename);
  r = syscall(8,filename,0,0);
  printf("exec failed\n");
}

int pd[2];

int pipe()
{
  int child;
   printf("pipe syscall\n");
   syscall(30, pd, 0, 0);
   printf("proc %d created a pipe with fd = %d %d\n", getpid(), pd[0], pd[1]);

}

int pfd()
{
  syscall(34,0,0,0);
}

int read_pipe()
{
  char fds[32], buf[1024];
  int fd, n, nbytes;
  pfd();

  printf("read : enter fd nbytes : ");
  gets(fds);
  sscanf(fds, "%d %d",&fd, &nbytes);
  printf("fd=%d  nbytes=%d\n", fd, nbytes);

  n = syscall(31, fd, buf, nbytes);

  if (n>=0){
     printf("proc %d back to Umode, read %d bytes from pipe : ",
             getpid(), n);
     buf[n]=0;
     printf("%s\n", buf);
  }
  else
    printf("read pipe failed\n");
}

int write_pipe()
{
  char fds[16], buf[1024];
  int fd, n, nbytes;
  pfd();
  printf("write : enter fd text : ");
  gets(fds);
  sscanf(fds, "%d %s", &fd, buf);
  nbytes = strlen(buf);

  printf("fd=%d nbytes=%d : %s\n", fd, nbytes, buf);

  n = syscall(32,fd,buf,nbytes);

  if (n>=0){
     printf("\nproc %d back to Umode, wrote %d bytes to pipe\n", getpid(),n);
  }
  else
    printf("write pipe failed\n");
}

int close_pipe()
{
  char s[16];
  int fd;
	pfd();
  printf("enter fd to close : ");
  gets(s);
  fd = atoi(s);
  syscall(33, fd);
}

int invalid(name) char *name;
{
    printf("Invalid command : %s\n", name);
}
