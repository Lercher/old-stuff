/* font - generate a Font for glups
 * Martin Lercher
 * Feb 1993
 */

#include <graphics.h>
#include <conio.h>
#include <mem.h>
#include <stdio.h>
#include <alloc.h>
#include "maus/mouse.h"
#include "myfont.h"

static vx, vy;
IMAGES image;
char *filename;

void save(void)
{   int i;
	FILE *f;

	f = fopen(filename, "wb");

	for(i=0; i<10; i++)
	{
		if (!image[i])
			image[i] = malloc(ISIZE);
		getimage(1+i*W,10,1+i*W+X, 10+Y, image[i]);
		fwrite(image[i], ISIZE, 1, f);
	}
	fclose(f);
}

void init(void)
{	int x;

	cleardevice();
	vx=0;
	vy=0;
	setcolor(1);
	for(x=0; x<10; x++)
	{
		rectangle(x*W,9,2+x*W+X, 11+Y);
		if (image[x])
			putimage(1+x*W, 10, image[x], COPY_PUT);
	}
}

int viewp(int x, int y)
{
	if (y>11+Y)
	{
		setviewport(0,12+Y, getmaxx(), getmaxy(), 1);
		vx=0;
		vy=12+Y;
	}
	else
	{
		x = ((x-1)/W*W)+1;
		setviewport(x, 10, x+X, 10+Y, 1);
		vx = x;
		vy = 10;
	}
	return 1;
}

void doit(void)
{   int x,y,f;
	int fl, fr, siz;
	int vp;

	fl = 1;
	fr = 0;
	siz = 3;
	vp = 0;

	init();
	for(;;)
	{
	ms_do(SHOW);
	f = ms_poll(&x, &y, kbhit);
	ms_do(HIDE);
	if (kbhit())
	{	int ch;

		ch = getch();
		if (ch == 'q')
			return;
		if (ch == '!')
			clearviewport();
		setviewport(0,0,getmaxx(),getmaxy(),0);
		if (ch == '*')
			init();
		if ('a'<=ch && ch <='a'+15)
			fl=ch - 'a';
		if ('A'<=ch && ch <='A'+15)
			fr=ch - 'A';
		if ('1'<=ch && ch <='9')
			siz=ch - '0';
		if (ch=='$')
			save();
		vp = 0;
		setcolor(1);
		setfillstyle(SOLID_FILL, 0);
		bar(getmaxx()-40, getmaxy()-20, getmaxx(), getmaxy());
		setfillstyle(SOLID_FILL, fl);
		fillellipse(getmaxx()-30, getmaxy()-10, siz,siz);
		setfillstyle(SOLID_FILL, fr);
		fillellipse(getmaxx()-10, getmaxy()-10, siz,siz);

	}
	if (f&1)
	{
		if (!vp)
			vp = viewp(x,y);
		setfillstyle(SOLID_FILL, fl);
		setcolor(fl);
		fillellipse(x-vx,y-vy, siz,siz);
	}
	if (f&2)
	{
		if (!vp)
			vp = viewp(x,y);
		setfillstyle(SOLID_FILL, fr);
		setcolor(fr);
		fillellipse(x-vx,y-vy, siz,siz);
	}
	if (f==0)
	{
		vp = 0;
	}

	}
}



int main(int argc, char **argv)
{	int gdriver = VGA, gmode = VGAHI;

	if (argc<2)
	{
		puts("Datei muss angegeben werden");
		return 1;
	}
	filename = argv[1];

	initgraph(&gdriver, &gmode, "");

	load(filename, image);
	mypalette();
	ms_do(INIT);

	doit();
	closegraph();

	return 0;
}