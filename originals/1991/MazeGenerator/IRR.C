/* Das wird ein Irrgarten
 * 1.0 vom 18.8.91
 *
 * Martin Lercher
 */

#ifdef __TOS__
#define TOS 1
#else
#define TOS 0
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int h = 26, v = 10;
static char *work;
static unsigned nr;
static printmode = 0;

#define w(x,y) work[(x)+(y)*h]

enum {PROCESSED = 1, RIGHT_OPEN = 2, DOWN_OPEN = 4};
enum dir {NORTH, SOUTH, EAST, WEST};

int processed(int x, int y)
{
	return (x<0 || y<0 || x>=h || y>=v || w(x,y)&PROCESSED);
}

void mk_irr(void)
{	int count;
	int max_count = h*v;
	int x, y;
	enum dir r;
	
	x = random(h);
	y = random(v);
	w(x,y) |= PROCESSED; /* einen Wegekeim setzen */
	count = 1;
	while(count < max_count)
	{
		r = random(4);
		switch(r)
		{
		case NORTH:
			if (!processed(x,y-1))
			{
				y--;
				w(x,y) |= PROCESSED|DOWN_OPEN;
				count++;
				break;
			}
		case SOUTH:
			if (!processed(x,y+1))
			{
				w(x,y) |= DOWN_OPEN;
				y++;
				w(x,y) |= PROCESSED;
				count++;
				break;
			}
		case EAST:
			if (!processed(x+1,y))
			{
				w(x,y) |= RIGHT_OPEN;
				x++;
				w(x,y) |= PROCESSED;
				count++;
				break;
			}
		case WEST:
			if (!processed(x-1,y))
			{
				x--;
				w(x,y) |= PROCESSED|RIGHT_OPEN;
				count++;
				break;
			}
			/* here are the other(old) directions */
			if (!processed(x,y-1))
			{
				y--;
				w(x,y) |= PROCESSED|DOWN_OPEN;
				count++;
				break;
			}
			if (!processed(x,y+1))
			{
				w(x,y) |= DOWN_OPEN;
				y++;
				w(x,y) |= PROCESSED;
				count++;
				break;
			}
			if (!processed(x+1,y))
			{
				w(x,y) |= RIGHT_OPEN;
				x++;
				w(x,y) |= PROCESSED;
				count++;
				break;
			}
			/* here is the point, where no direction works */
			/* suche einen wegpunkt mit mindestens einem
			 * fortsetzungspunkt 
			 */
			do {
				/* zeilenweiser scan */
				if (0 == (x = (x+1)%h))
					y = (y+1)%v;
			} while(0 == (w(x,y) & PROCESSED));
			break;
		}
	}
	/* mache unten eine ™ffnung. */
	w(random(h),v-1) |= DOWN_OPEN;
}

void disp_irr(void)
{	int x,y, r;
	
	r = random(h); /* ™ffnung nach oben. */
	printf("+");
	for(x=0; x<h; x++)
		if (x == r)
			printf("  +");
		else
			printf("--+");
	printf("\n");
	for(y=0; y<v; y++)
	{
		printf("|");
		for(x=0; x<h; x++)
			if (w(x,y)&RIGHT_OPEN)
				printf("   ");
			else
				printf("  |");
		printf("\n+");
		for(x=0; x<h; x++)
			if (w(x,y)&DOWN_OPEN)
				printf("  +");
			else
				printf("--+");
		printf("\n");
	}
}

#define ESC "\033"

#if TOS
void pline(int n)
{
static line = 1;

	printf("\rZeile %d von %d.", line++, v+1);
	fflush(stdout);
	/* 576iger Dichte */
	fprintf(stdprn, ESC"*\005%c%c", n&0xff, n>>8);
}

void put(int what)
{	int i;
	unsigned char x;

	x = what&DOWN_OPEN ? 0 : 1;
	for(i=0; i<7; i++)
		putc(x, stdprn);
	putc(what&RIGHT_OPEN ? 1 : 255, stdprn);
}


void print_irr(void)
{	int x,y, r;
	int n;
	
	printf("Drucke Labyrinth Nr. %u  Gr”že %dx%d.\n", nr, h, v);
	n = 8*(h+1);
	fprintf(stdprn, ESC"@"ESC"1"); /* reset & 7/72 Zoll */
	pline(n);
	put(DOWN_OPEN|RIGHT_OPEN);
	r = random(h); /* ™ffnung nach oben. */
	for(x=0; x<h; x++)
		if (x == r)
			put(DOWN_OPEN|RIGHT_OPEN);
		else
			put(RIGHT_OPEN);
	fprintf(stdprn, "\n");
	for(y=0; y<v; y++)
	{
		pline(n);
		put(DOWN_OPEN);
		for(x=0; x<h; x++)
			put (w(x,y) & (RIGHT_OPEN|DOWN_OPEN));
		fprintf(stdprn, "\n");
	}
	fprintf(stdprn, ESC"@\n"); /* reset */
	fflush(stdprn);
	printf("\n");
}

#else

#define SCALE 100

void pline(void)
{
static line = 1;

	printf("\rZeile %d von %d.", line++, v);
	fflush(stdout);
}

void put(int x, int y, int what)
{	int i;
	unsigned char c;

	x++; y++;
	x*=SCALE; y*=SCALE;
	if (0== (what & RIGHT_OPEN))
		fprintf(stdprn, "PU%d,%d;PD%d,%d;", x+SCALE, y, x+SCALE, y+SCALE);
	if (0== (what & DOWN_OPEN))
		fprintf(stdprn, "PU%d,%d;PD%d,%d;", x, y+SCALE, x+SCALE, y+SCALE);
}

void print_irr(void)
{	int x,y, r;

	printf("Drucke Labyrinth Nr. %u  Gr”áe %dx%d.\n", nr, h, v);
	fprintf(stdprn, ESC"E"ESC"*t300R"ESC"%0BIN;PW0.1;");
	/* reset & 300dpi & HP-GL */
	put(-1,-1,DOWN_OPEN|RIGHT_OPEN);
	r = random(h); /* ™ffnung nach oben. */
	for(x=0; x<h; x++)
		if (x == r)
			put(x, -1, DOWN_OPEN|RIGHT_OPEN);
		else
			put(x, -1, RIGHT_OPEN);
	for(y=0; y<v; y++)
	{
		pline();
		put(-1, y, DOWN_OPEN);
		for(x=0; x<h; x++)
			put (x, y, w(x,y) & (RIGHT_OPEN|DOWN_OPEN));
	}
	fprintf(stdprn, "ESC%0A"ESC"E"); /* HP-GL-Graphik aus & reset */
	fflush(stdprn);
	printf("\n");
}
#endif /* TOS */

int main(int argc, char *argv[])
{
/*	freopen("xxx.pcl", "w", stdprn); */
#if TOS
	printf("Irrgarten TOS Version. Printer = EPSON.\n");
#else
	printf("Irrgarten MSDOS Version. Printer = HP-LJ III in HPGL\n");
#endif
	if (argc == 1)
		printf("Argumente: [-] [#Xsize [#Ysize [#Labyrinth]]]\t\t - ist Druckerausgabe.\n");
	if (argc>1 && *(argv[1]) == '-')
	{
		printmode = 1;
#if TOS
		h=70;
		v=90;
#else
		h=76;
		v=107;
#endif
		argc--;
		argv = &(argv[1]);
	}
	if (argc>1)
		h = atoi(argv[1]);
	if (argc>2)
		v = atoi(argv[2]);
	if (argc>3)
		srand(nr = atoi(argv[3]));
	else
		srand(nr = (unsigned)(time(NULL) & 0xffffUL));

	work = calloc(v, h*sizeof(char));

	if (!work)
	{
		printf("Das Labyrinth ist zu groá.");
		return 1;
	}

	mk_irr();
	if (printmode)
		print_irr();
	else
		disp_irr();
	
	free(work);
#if TOS
	if (!printmode)
	{
		fprintf(stderr, "Press <Return> ");
		fflush(stderr);
		(void)getchar();
	}
#endif
	return 0;
}
