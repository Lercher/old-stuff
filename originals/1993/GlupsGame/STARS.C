#include <graphics.h>
#include <time.h>
#include <conio.h>
#include <stdlib.h>

#define A 12

static void sss(int x, int y, int d)
{
	putpixel(x,y,d+getpixel(x,y));
	putpixel(x+1,y,d+getpixel(x+1,y));
	putpixel(x,y+1,d+getpixel(x,y+1));
	putpixel(x+1,y+1,d+getpixel(x+1,y+1));
}

static void starat(int x, int y, int radius, int d)
{	int r;
	clock_t t;

	t = clock()+3;
	for(r=0; r<=2*radius; r+=2)
	{
		sss(x+r,y,d);
		sss(x-r,y,d);
		sss(x,y+r,d);
		sss(x,y-r,d);
	}
	while(clock()<t)
		if(kbhit())
			break;
}


void stars(void)
{   int x, y, i;

	while(!kbhit())
	{
		x = rand()%getmaxx();
		y = rand()%getmaxy();

		if (getpixel(x,y))
		{
			for(i=0; i<A; i++)
				starat(x,y,i,+1);
			for(--i; i>=0; i--)
				starat(x,y,i,-1);
		}
	}
}