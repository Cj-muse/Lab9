/**********************************************************************
                         kbd.c file
***********************************************************************/
#include "header.h"
#define KEYBD 0x60 // I/O port for keyboard data
#define PORT_B 0x61 // port_B of 8255
#define KBIT 0x80 // ack bit
#define NSCAN 58 // number of scan codes
#define KBLEN 64
#define BELL 0x07
#define F1 0x3B // scan code of function keys
#define F2 0x3C
#define CAPSLOCK 0x3A // scan code of special keys
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define CONTROL 0x1D
#define ALT 0x38
#define DEL 0x53

/*************************** keymap ***********************************/

 /* Scan codes to ASCII for unshifted keys; unused keys are left out */
char unshift[NSCAN] = { 
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','[',']', '\r', 0,'a','s',
 'd','f','g','h','j','k','l',';',       0, 0, 0, 0,'z','x','c','v',
 'b','n','m',',','.','/', 0,'*',        0, ' '       
};

/* Scan codes to ASCII for shifted keys; unused keys are left out */
char shift[NSCAN] = {
 0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
 'Q','W','E','R','T','Y','U','I',      'O','P','{','}', '\r', 0,'A','S',
 'D','F','G','H','J','K','L',':',       0,'~', 0,'|','Z','X','C','V',
 'B','N','M','<','>','?',0,'*',         0, ' '  
};


typedef struct kbd{           // data struct representing the keyboard
   char buf[KBLEN];
   int head, tail;
   SEMAPHORE data; // semaphore between inth and process
}KBD;

KBD kbd;
int alt, capslock, esc, shifted, control, arrowKey; // state variables

int kbd_init()
{
   printf("kbinit()\n");

  kbd.head = kbd.tail = 0;

  // kb.data.value = kb.data.queue = 0;
  kbd.data.value = 0;
  kbd.data.queue = 0;
 
  printf("kbinit done\n");
}

/******************************************************************************
 kbhandler() is the kbd interrupt handler. The KBD generates 2 interrupts
 for each key typed; one when the key is pressed and another one when the
 key is released. Each key generates a scan code. The scan code of key
 release is 0x80 + the scan code of key pressed. When the kbd interrupts,the
 scan code is in the data port (0x60) of the KBD interface. Read the scan code
 from the data port. Then ack the key input by strobing its PORT_B at 0x61.
*****************************************************************************/

// need to update this function
int kbhandler()
{
   int scode, value, c;
   // Fetch scab code from the keyboard hardware and acknowledge it.
   printf("in kboard interupt handler\n");
   scode = in_byte(KEYBD); // get scan code
   value = in_byte(PORT_B); // strobe PORT_B to ack the char
   out_byte(PORT_B, value | KBIT);
   out_byte(PORT_B, value);
   
   if (scode == 0xE0) // ESC key
      esc++; // inc esc count by 1
   if (esc && esc < 2) // only the first ESC key, wait for next code
      goto out;
   if (esc == 2){ // two 0xE0 means escape sequence key release
      if (scode == 0xE0) // this is the 2nd ESC, real code comes next
      goto out;
      // with esc==2, this must be the actual scan code, so handle it
      scode &= 0x7F; // leading bit off
      if (scode == 0x38){ // Right Alt
         alt = 0;
         goto out;
      }
      if (scode == 0x1D){ // Right Control release
         control = 0;
         goto out;
      }
      if (scode == 0x48) // up arrow
         arrowKey = 0x0B;
      esc = 0;
      goto out;
   }
   if (scode & 0x80){// key release: ONLY catch shift,control,alt
      scode &= 0x7F; // mask out bit 7
      if (scode == LSHIFT || scode == RSHIFT)
         shifted = 0; // released the shift key
      if (scode == CONTROL)
      control = 0; // released the Control key
      if (scode == ALT)
      alt = 0; // released the ALT key
      goto out;
   }
   // from here on, must be key press
   if (scode == 1) // Esc key on keyboard
      goto out;
   if (scode == LSHIFT || scode == RSHIFT){
      shifted = 1; // set shifted flag
      goto out;
   }
   if (scode == ALT){
      alt = 1;
      goto out;
   }
   if (scode == CONTROL){
      control = 1;
      goto out;
   }
   if (scode == 0x3A){
      capslock = 1 - capslock; // capslock key acts like a toggle
      goto out;
   }
   if (control && alt && scode == DEL){
      printf("3-finger salute\n");
      goto out;
   }
   /************* Catch and handle F keys for debugging *************/
   //if (scode == F1){ do_F1(); goto out;}
   //if (scode == F2){ do_F2(); goto out;} // etc
   // translate scan code to ASCII, using shift[ ] table if shifted;
   c = (shifted ? shift[scode] : unshift[scode]);
    // Convert all to upper case if capslock is on
   if (capslock){
      if (c >= 'A' && c <= 'Z')
         c += 'a' - 'A';
      else if (c >= 'a' && c <= 'z')
         c -= 'a' - 'A';
   }
   if (control && (c=='c'||c=='C')){// Control-C on PC are 2 keys
      //Control-C Key; send SIGINT(2) signal to processes on console;
      c = '\n'; // force a line, let procs handle SIG#2 when exit Kmode
   }
   if (control && (c=='d'|| c=='D')){ // Control-D, these are 2 keys
      printf("Control-D: set code=4 to let process handle EOF\n");
      c = 4; // Control-D
   }
   /* enter the char in kbd.buf[ ] for process to get */
   if (kbd.data.value == KBLEN){
      printf("%c\n", BELL);
      goto out; // kb.buf[] already FULL
   }
   kbd.buf[kbd.head++] = c;
   printf("kbinth c = %c\n",c);
   kbd.head %= KBLEN;
   V(&kbd.data);
out:
   out_byte(0x20, 0x20); // send EOI
}

/***************** upper-half driver routine ****************/
int kbgetc()
{
   int c;
   printf("in kbgetc\n");
   P(&kbd.data);
   
   lock();
   c = kbd.buf[kbd.tail++] & 0x7F;
   printf("in kbgetc: c = %c \n",c);
   kbd.tail %= KBLEN;
   unlock();

   return c;
}




