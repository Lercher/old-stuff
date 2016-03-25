#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

static unsigned x[100],y[100];

main()
{       unsigned l,h, i;
   disable();
   for(i =0 ; i<100 ; i++)
   {
	x[i] = inportb(0x40);
	y[i] = inportb(0x40);
   }
   enable();
   for(i =0 ; i<100 ; i++)
   {
	cprintf("%4x ", (x[i]*0x100)+y[i]);
   }
}


