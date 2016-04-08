#include "header.h"

/*int rpu(u32 x)
{
  char c;
  if (x)
    {
      c = table[x % BASE];
      rpu(x / BASE);
      putc(c);
    }
}
int rpu16(u16 x)
{
  char c;
  if (x)
    {
      c = table[x % BASE];
      rpu16(x / BASE);
      putc(c);
    }
}

int rpl(u32 x)
{
  char c;
  if (x)
    {
      c = table[x % BASE];
      rpl(x / BASE);
      putc(c);
    }
}

int printu(u32 x)
{
  if (x==0)
    putc('0');
  else
    BASE = 10;
    rpu(x);
}

int printd(int x) //prints an integer
{
  if (x == 0)
    putc('0');
  else
   BASE = 10;
   rpu16(x);
}
int printl(u32 x)
{
  if (x == 0)
    putc('0');
  else
   BASE = 10;
   rpl(x);
}

int printo(u32 x) //prints x in OCTal
{
  if (x == 0 )
    putc(' ');
  else
    BASE = 8;
    rpu(x);
}

int printx(u32 x) //prints x in hex
{
  if (x == 0 )
    putc('0');
  else
    BASE = 16;
    rpu(x);
}

void printf(char *fmt, ...)
{
	//u16 *ip
	int *ip = (u16 *)&fmt + 1;
  	int i = 0;
  	char *cp = fmt;

  	//asm("mov ebp, FP");
  	//ip = FP + 3;

	for(i = 0; fmt[i]; i++)
  	{
		if (fmt[i] == '%')
		{
	  	i++;
			switch(fmt[i])
			{
				case'c': putc(*ip);   ip++;break;
				case's': prints(*ip); ip++;break;
				case'u': printu(*ip); ip++;break;
			  case'd': printd(*ip); ip++;break;
        case'l': printl(*(u32*)ip++); ip++;break;
				case'o': printo(*ip); ip++;break;
				case'x': printx(*ip); ip++;break;
				default: putc(fmt[i]);ip++;break;
		  	}
	 	}
      else
		{
			putc(fmt[i]);
		}
	}
}

int prints(char *s)
{
	int i = 0;
	while(s[i])
	{
		putc(s[i]);
		i++;
	}
}

//returns a string using the assembly function getc()
int gets(char s[64])
{
	int i = 0;
	char c = '0';

	// put carrige return and newline on front of string
//	s[0] = '\n';
//	s[1] = '\r';
//	i = 2;

	do // iterate over user keystrokes until newline is encounterd
	{
		c = getc();
		s[i] = c;
		putc(s[i]);// print out the char entered for the user to see
		i++;
	}  while (c != 13 && i < 63);

	//add \n
	s[i] = '\n';
	s[i+1] = '\r';
	s[i+2] = 0;
}*/

int strtoint(char *str)
{
	int integer = 0, i = 0, mul = 0;

	//traverse to the end of the string
	while(str[i] != '\n')
	{i++;	}
	i -= 2;

	while(isDigit(str[i]) != -1)
	{
		integer += power(10, mul) * isDigit(str[i]);
		mul++;
		i--;
	}
	return integer;
}

int isDigit(char c)
{
	switch(c)
	{
		case '0': return 0; break;
		case '1': return 1; break;
		case '2': return 2; break;
		case '3': return 3; break;
		case '4': return 4; break;
		case '5': return 5; break;
		case '6': return 6; break;
		case '7': return 7; break;
		case '8': return 8; break;
		case '9': return 9; break;
		default: return -1;
	}
}

int power(int x, int y)
{
	int i = 0, ret = 0;

	if (y == 0)
	{
		return 1;
	}

	ret = x;
	for(i = 1; i < y; i++)
	{
		ret = ret * x;
	}
	//printf("power(): returning %d \n\r", ret);
	return ret;
}
