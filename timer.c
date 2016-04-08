#include "header.h"

#define LATCH_COUNT     0x00	/* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	/* ccaammmb, a = access, m = mode, b = BCD */
				/* 11x11, 11 = LSB then MSB, x11 = sq wave */
#define TIMER_FREQ   1193182L	/* clock frequency for timer in PC and AT */
#define TIMER_COUNT ((unsigned) (TIMER_FREQ/60)) /*initial value of counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

#define INT_CNTL     0x20       /* 8259 interrupt control register */       
#define INT_MASK     0x21       /* bit i = 0 means enable interrupt i */ 

int tick,sec,min,hr, totalTime;                    /* Wall clock */

typedef struct tq{
	struct tq *next;      // next element pointer
	int       time;       // requested time
	struct PROC *proc;    // pointer to PROC
	int    (*action)();   // 0|1|handler function pointer
}TQE;

TQE tq, tqe[NPROC];           // tq = timer queue pointer

int enable_irq(irq_nr) u8 irq_nr;
{
  /* Assume IRQ 0-7 ONLY. Clear the corresponding 8259 interrupt mask bit. */
  out_byte(INT_MASK, in_byte(INT_MASK) & ~(1 << irq_nr));
}

int timer_init()
{
 /* Initialize channel 0 of the 8253A timer to 60 Hz. */

  tick = sec = min = hr = 0;            /* initialize wall clock to 0 */
  out_byte(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
  out_byte(TIMER0, TIMER_COUNT);         /* load timer low byte */
  out_byte(TIMER0, TIMER_COUNT >> 8);	/* load timer high byte */
  enable_irq(TIMER_IRQ);                /* enable timer interrupts */
}

int getTime()
{
	return totalTime;
}

int setSleepTime(int t)
{
   running->sleeptime = t;
}

int tickSleepingProcs()
{
   int i =0;
   PROC *p;
   
   for (i=0; i < NPROC; i++)
   {
      p = &proc[i];
      if(p->status == SLEEP)
      {
         p->sleeptime--;
         if(p->sleeptime == 0)
         {
            kwakeup(p->pid);
         }
      }
   }
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
int thandler()
{
  tick++; 
  tick %= 60;
  //tick %= 10;
  //tick %= 1;
	
  if (tick == 0){                      // at each second

		if (running->runtime > 0)   
		{
			running->runtime--;
		}
		tickSleepingProcs();    // tick a second off each sleeping proc

		totalTime++;
		sec++;
		if (sec >= 60)
		{
			sec = 0;
			min++;
			if (min >= 60)
			{
				min = 0;
				hr++;			
			}
		}
		
		timeStamp(sec, min, hr);
  }
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI
	/*if(running->runtime == 0) // switch process
	{
	   running->runtime = 5;
		tswitch();	
	}*/

}
