/* include file fÅr TETRIS.C. Stellt eine schnelle clock zur VerfÅgung */
#include <dos.h>

typedef long clock_t;
#define CLK_TCK	4658L

clock_t my_clock(void)
{	int h;
	unsigned long l;

	disable();
	l = (unsigned long) inportb(0x40);
	h = inportb(0x40);
	l = (*((unsigned long far*)MK_FP((0x40),(0x6c))));
	enable();
	return((l<<8) - (0xff & h));
}

#define clock()	my_clock()