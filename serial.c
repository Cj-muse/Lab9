#include "header.h"
/**************** CONSTANTS ***********************/
#define INT_CTL     0x20
#define ENABLE      0x20

#define INBUFLEN    80
#define OUTBUFLEN   80
#define EBUFLEN     10
#define NULLCHAR      0
#define BEEP          7
#define BACKSPACE     8
#define ESC          27
#define SPACE        32

#define BUFLEN       64
#define LSIZE        64

#define NR_STTY       2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */

/**** The serial terminal data structure ****/
struct stty {
   /* input buffer */
   char inbuf[BUFLEN];
   int inhead, intail;
   struct semaphore inchars, inmutex;

   /* output buffer */
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outspace, outmutex;
   int tx_on;
   
   /* echo buffer */
   char ebuf[EBUFLEN];
   int ehead, etail, e_count;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;
   
   /* I/O port base address */
   int port;
} stty[NR_STTY];


/********  bgetc()/bputc() by polling *********/
int bputc(int port, int c)
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

int bgetc(int port)
{
	//still having trouble understanding 
   while ((in_byte(port+LSR) & 0x01) == 0); 
   return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(unsigned irq_nr)
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}
   
/************ serial ports initialization ***************/
char *p = "\n\rSerial Port Ready\n\r\007";

int sinit()
{
  int i;  
  struct stty *t;
  char *q; 

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++){
    q = p;

		printf("sinit : port #%d\n",i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
          t->port = 0x3F8;    /* COM1 base address */
      else
          t->port = 0x2F8;    /* COM2 base address */
  
      t->inhead = t->intail = 0;
      t->inchars.value = 0;  t->inchars.queue = 0;

      t->outhead = t->outtail = t->tx_on = 0;
      t->outspace.value = BUFLEN; t->outspace.queue = 0;

      // initialize control chars; NOT used in MTX but show how anyway
      t->ison = t->echo = 1;   /* is on and echoing */
      t->erase = '\b';
      t->kill  = '@';
      t->intr  = (char)0177;  /* del */
      t->quit  = (char)034;   /* control-C */
      t->x_on  = (char)021;   /* control-Q */
      t->x_off = (char)023;   /* control-S */
      t->eof   = (char)004;   /* control-D */

    lock();  // CLI; no interrupts

      out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */ 
      out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */
      out_byte(t->port+DIVH, 0x00);
      out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */

      /******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/ 
      out_byte(t->port+LCR, 0x03); 

      /*******************************************************************
        Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        let modem respond as a DCE.  Here we must let the (crossed)
        cable tell the TVI terminal that the "DCE" has DSR and CTS.  
        So we turn the port's DTR and RTS on.
      ********************************************************************/
      out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    unlock();
    
    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    //USE bputc() to PRINT MESSAGE ON THE SERIAL PORT: serial port # ready
		while (*q)
		{
      	bputc(t->port, *q);
      	q++;
    	}
  }
}  
         

//======================== LOWER-HALF ROUTINES ===============================

int shandler(int port)
{  
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;
   
   printf("in shandler\n");
   t = &stty[port];            /* IRQ 4 interrupt : COM1 = stty[0] */

   IntID     = in_byte(t->port+IIR);       /* read InterruptID Reg */
   LineStatus= in_byte(t->port+LSR);       /* read LineStatus  Reg */    
   ModemStatus=in_byte(t->port+MSR);       /* read ModemStatus Reg */

   intType = IntID & 7;     /* mask out all except the lowest 3 bits */
   //printf("intType = %d \n", intType);
   
   switch(intType){
      case 6 : do_errors(t);  break;   /* 110 = errors */
      case 4 : do_rx(t);      break;   /* 100 = rx interrupt */
      case 2 : do_tx(t);      break;   /* 010 = tx interrupt */
      case 0 : do_modem(t);   break;   /* 000 = modem interrupt */
   }
   
   out_byte(INT_CTL, ENABLE);   /* reenable the 8259 controller */ 
   printf("leaving shandler\n");
}

int s0handler(){ shandler(0);}
int s1handler(){ shandler(1);}

int do_errors()
{ printf("ignore error\n"); }

int do_modem()
{  printf("don't have a modem\n"); }


/* The following show how to enable and disable Tx interrupts */
enable_tx(struct stty *t)
{
  lock();
  out_byte(t->port+IER, 0x03);   /* 0011 ==> both tx and rx on */
  t->tx_on = 1;
  unlock();
}

disable_tx(struct stty *t)
{ 
   lock();
  out_byte(t->port+IER, 0x01);   /* 0001 ==> tx off, rx on */
  t->tx_on = 0;
  unlock();
}

int secho(struct stty *tty, int c)
{
   /* insert c into ebuf[]; turn on tx interrupt */
   tty->ebuf[tty->ehead++] = c;
   tty->ehead %= EBUFLEN;
   tty->e_count++;
   if (!tty->tx_on)
        enable_tx(tty);     /* tx handler will flush them out */
}

// ============= Input Driver ==========================
int do_rx(struct stty *tty)
{
	int c;
  c = in_byte(tty->port) & 0x7F;  /* read the ASCII char from port */

  printf("port %x interrupt:c=%c ", tty->port, c,c);
  printf("rx:c=%c ", c);

  // cook \b 
  if (c==BACKSPACE){
      secho(tty, '\b'); secho(tty,' '); secho(tty,'\b');
      if (tty->inchars.value > 0){
          tty->inchars.value--;
          tty->inhead--;
          tty->inhead %= INBUFLEN;
      }
      return;
  }
  if (tty->inchars.value >= INBUFLEN){
      secho(tty, BEEP);
      return;
  }

  secho(tty, c);

  if (c == '\r'){
       c = '\n';                   /* change CR to LF */
       secho(tty,c);
       putc(c);
       printf("rx: has a line ");
  }
  tty->inbuf[tty->inhead++] = c;   /* put char into inbuf[] */
  tty->inhead %= INBUFLEN;         /* advance inhead */

  V(&tty->inchars);          /* inc inchars and wakeup sgetc() */
}


int sgetc(struct stty *tty)
{ 
  int c;
  P(&tty->inchars);   /* wait if no input char yet */
    lock();             /* disable interrupts */
      c = tty->inbuf[tty->intail++];
      tty->intail %= INBUFLEN;
    unlock();
    //printf("sgetc : c=%x\n", c);
  return(c);
}

int sgetline(struct stty *tty, char *line)
{
   printf("sgetline ");

   P(&tty->inmutex);       /* one process at a time */
   
   while ( (*line = sgetc(tty)) != '\n')
   {
   	line++;
   }
   *line = 0;
   V(&tty->inmutex);

   return strlen(line);
}

//****************** Output driver *****************************
int do_tx(struct stty *tty)
{
	int c;
  //  printf("tx interrupt ");
  if (tty->e_count == 0 && tty->outspace.value == OUTBUFLEN){ // nothing to do 
    disable_tx(tty);                // turn off tx interrupt
      return;
  }

  if (tty->e_count) {                 // there are chars to echo
        c = tty->ebuf[tty->etail++];  tty->etail %= EBUFLEN;
        tty->e_count--;
        out_byte(tty->port, c);       // write the char out
        return;                       // that's all for now
  } 

  if (tty->outspace.value < OUTBUFLEN){ // has something to output
        c = tty->outbuf[tty->outtail++];
        tty->outtail %= OUTBUFLEN;
        out_byte(tty->port, c);
        if (c=='\n')
	    printf("tx: done with a line\n");
        V(&tty->outspace);
        return;
  }
}

//--------------- UPPER half functions -------------------
int sputc(struct stty *tty, char c)
{
	P(&tty->outspace);     /* wait for room in outbuf[] */  
   lock();               /* disalble interrupts */
   tty->outbuf[tty->outhead++] = c;
   tty->outhead %= OUTBUFLEN;
   unlock();

   if (!tty->tx_on)  /* enable tx interrupt if not on */      
   	enable_tx(tty);
   
   return(1);
}

int sputline(struct stty *tty, char *line)
{	
   P(&tty->outmutex);                  // one process at a time
	while (*line)
	{
		sputc(tty, *line);
  		line++;
   }
   V(&tty->outmutex);
}



//**************** Syscalls from Umdoe ************************
int usgets(int port, char *y)
{  
	int c, n;
   struct stty *tty = &stty[0];      /* Our only serial terminal */
   n = 0;      
	
	while ( (c = sgetc()) != '\n')
	{
   	put_byte(c,running->uss, y);
      n++; y++;
   }
   put_byte(0, running->uss, y);
   return(n);
}

int uputs(int port, char *y)
{
  // output line y in U space to serail port
}

