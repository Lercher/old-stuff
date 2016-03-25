/* maus.c - zu loops.c */

#include <conio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>

#define ILLEGAL -1000

static clock_t itstime;
extern int newtime, bonustime, level;

enum mousestatus { INIT=0, SHOW=1, HIDE=2 };

int ms_get(int* x,int*y)
{  struct REGPACK reg;
  reg.r_ax=3; intr(0x33,&reg);
  *x=reg.r_cx; *y=reg.r_dx; return(reg.r_bx);
}

void ms_do(enum mousestatus s)
{ struct REGPACK reg;
  reg.r_ax=(int)s; intr(0x33,&reg);
}

extern int  moveitto(int x, int y);
extern int  moveitrel(int x, int y);
extern int  moveittocenter(void);
extern void showtime(int time);

extern XOFFSET, YOFFSET, WIDTH;

static mx=ILLEGAL, my=0, mf=3;
static clock_t lclock;

void mausinit(void)
{
	ms_do(INIT);
	srand((int)clock());
}

void ms2int(int x, int y, int *i, int *j)
{
	x = x-XOFFSET;
	y = YOFFSET-y;
	*i= x/WIDTH;
	*j= (y+WIDTH)/WIDTH;
}

void waitforrelease(void)
{	int xx, yy, ff;

	ms_do(SHOW);
	ff = ms_get(&xx, &yy);
	if (ff)
		do
		{
			ff = ms_get(&xx, &yy);
		} while(ff);
	else
		do
		{
			ff = ms_get(&xx, &yy);
		} while(!ff);
	mf = 3;
	ms_do(HIDE);
}

int maus(void)
{	int xx, yy, ff, result;

	if (newtime)
	{
		itstime = clock() + (10-level+bonustime) * (int)CLK_TCK;
		newtime = 0;
	}
	ms_do(SHOW);
	result = 'n';
	while(itstime >= clock())
	{
	ff = ms_get(&xx, &yy);
	if ((ff&1) != (mf&1))
	{
		mf = ff;
		if (ff==1)
		{
			mx = ILLEGAL; /* force redraw */
			result = ' ';
			goto quit;
		}
		if (ff==3)
		{
			mx = ILLEGAL;
			result = 'd';
			goto quit;
		}
	}
	if ((ff&2) != (mf&2))
	{
		mf = ff;
		if (ff==2)
		{
			mx = ILLEGAL; /* force redraw */
			result = 'r';
			goto quit;
		}
	}
	if (xx != mx || yy != my)
	{   int i, j, k, l;

		ms2int(mx,my,&k,&l);
		ms2int(xx,yy,&i,&j);

		mx = xx;
		my = yy;
		if (k!=i || l!=j)
		{
			if (!moveitto(i,j))
			{
				while(!moveittocenter())
					/* idle */;
			}
			result = 0;
			goto quit;
		}
	}
	if (kbhit())
	{
		result = getch();
		goto quit;
	}
	if (clock() != lclock)
	{
		lclock = clock();
		showtime((int)(itstime - clock()));
	}
	}
	mx = ILLEGAL; /* force redraw */
quit:
	ms_do(HIDE);
	return result;
}
