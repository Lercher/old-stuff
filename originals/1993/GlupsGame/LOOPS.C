/* loops.c - Tetris mit Schleifen.
 * Martin Lercher, Schloesslgartenweg 40, 8415 Nittenau
 * Februar 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
#include <graphics.h>
#include <alloc.h>
#include <dos.h>
#include <string.h>
#include <time.h>
#include "myfont.h"

#define VERSION "(á1.9)"

enum { FELDX=20, FELDY=12, EMPTY=0,
	   OBEN=1, UNTEN=2, LINKS=3, RECHTS=4 };

struct Teil {
	int rein, raus;
	char picture;
	void far *image;
};

struct Teil TEIL[] = {
	0,0, '.',  NULL,
	LINKS, RECHTS, 205, NULL,
	OBEN, UNTEN, 186, NULL,
	OBEN, RECHTS, 200, NULL,
	OBEN, LINKS, 188, NULL,
	UNTEN, LINKS, 187, NULL,
	UNTEN, RECHTS, 201, NULL,
	0,0, '*', NULL
};
#define MAXTEIL ((int)(sizeof(TEIL)/sizeof(struct Teil)))

struct Bauteil {
	unsigned active; /* bitvector */
	int prob, center; /* Darum wird gedreht */
	char *what; /* - straight, r turn right, l turn left, * del */
};

int MAXBAUTEIL;
#define MAXBAU 101
struct Bauteil BAUTEIL[MAXBAU];

#define MAXLEN 10
struct display {
	int dx, dy, teil;
} theDisplay[MAXLEN];
int currentrotation, currentteil, currentx, currenty;

struct FeldElement {
	int teil, update, bonustime;
};

typedef struct FeldElement Feld[FELDX][FELDY];

Feld feld;
long score;
int loops, deleters, trys, level, ilevel, charset;
int SIZE, WIDTH, XOFFSET, YOFFSET;
IMAGES numbers, glups;
int newtime, bonustime;

extern void stars(void);
int fitsitnot(void);

void beep(void)
{
	sound(1000);
	delay(25);
	nosound();
}

int infeld(int x, int y)
{
	if (x>=0 && y>=0 && x<FELDX && y<FELDY)
		return 1;
	return 0;
}


void reinraus(const int teil, int *rein, int *raus)
{
	*rein = TEIL[teil].rein;
	*raus = TEIL[teil].raus;
}

void rot2delta(int rotation, int *dx, int *dy)
{	int x, y;

	x = *dx;
	y = *dy;
	switch(rotation)
	{
	case OBEN:
		y++;
		break;
	case UNTEN:
		y--;
		break;
	case RECHTS:
		x++;
		break;
	case LINKS:
		x--;
		break;
	}
	*dx = x;
	*dy = y;
}

int rr2teil(int rein, int raus)
{	int x, y, i;

	/* rein ist das letzte raus, also umdrehen */
	switch(rein)
	{
	case OBEN:
		rein=UNTEN;
		break;
	case UNTEN:
		rein=OBEN;
		break;
	case RECHTS:
		rein=LINKS;
		break;
	case LINKS:
		rein=RECHTS;
		break;
	}
	if (rein<raus)
	{
		x = rein;
		y = raus;
	}
	else
	{
		x = raus;
		y = rein;
	}

	for(i=1; i<MAXTEIL; i++)
		if (x == TEIL[i].rein && y == TEIL[i].raus)
			return i;
	return MAXTEIL-1; /* reached by deleter */
}

int turn(char direction, int rotation)
{
	if (direction=='*')
		return 0;
	if (direction=='-')
		return rotation;
	switch(rotation)
	{
	case OBEN:
		if (direction == 'r')
			return RECHTS;
		return LINKS;
	case UNTEN:
		if (direction == 'r')
			return LINKS;
		return RECHTS;
	case RECHTS:
		if (direction == 'r')
			return UNTEN;
		return OBEN;
	case LINKS:
		if (direction == 'r')
			return OBEN;
		return UNTEN;
	}
	return OBEN; /* not reached */
}

void bauteil2disp(void)
{	struct Bauteil *aBauteil;
	int center, x, y, newrot, disp, which, rotation;
	int savex, savey;
	char *what;

	which = currentteil;
	rotation = currentrotation;
	
	assert(0<=which && which<=MAXBAUTEIL); /* ! */

	aBauteil = BAUTEIL+which;
	center = aBauteil->center;
	what = aBauteil->what;
	
	x = y = 0;
	disp = 0;
	while(*what)
	{
		newrot = turn(*what, rotation); /* check rotation */
		theDisplay[disp].teil = rr2teil(rotation, newrot);
		theDisplay[disp].dx = x;
		theDisplay[disp].dy = y;
		disp++;
		if (disp==center)
		{
			savex = x;  savey = y;
		}
		rot2delta(newrot, &x, &y); /* increment (x,y) */
		rotation = newrot;
		what++;
	}
	theDisplay[disp].teil = EMPTY; /* ENDE Kriterium */

	/* das center erh„lt die Koordinaten (0,0) */
	for(disp=0; theDisplay[disp].teil != EMPTY; disp++)
	{
		theDisplay[disp].dx -= savex;
		theDisplay[disp].dy -= savey;
	}
}

static counter;
void dispstart(void)
{
	counter=0;
}

int displist(int *x, int *y, int *teil)
{
	if ((*teil = theDisplay[counter].teil) == EMPTY)
		return 0;
	*x = currentx + theDisplay[counter].dx;
	*y = currenty + theDisplay[counter].dy;
	counter++;
	return 1;
}

/* wenn's nicht eingetragen oder rotiert oder verschoben wird */
void tagit(void)
{	int x,y,teil;

	for(dispstart(); displist(&x, &y, &teil);)
		if (infeld(x,y))
			feld[x][y].update=1;
}

int moveitto(int x, int y)
{
/*	int xx, yy; */

	tagit();
/*	xx = currentx; */
/*	yy = currenty; */
	currentx = x;
	currenty = y;
	if (fitsitnot()==2)
	{
/*		currentx = xx; */
/*		currenty = yy; */
		return 0;
	}
	return 1;
}

int moveitrel(int x, int y)
{
	tagit();
	currentx += x;
	currenty += y;
	if (fitsitnot()==2)
	{
		currentx -= x;
		currenty -= y;
		return 0;
	}
	return 1;
}

int signum(int x, int to)
{
	x -= to/2;
	if (x>2)
		return -1;
	if (x<-2)
		return 1;
	return 0;
}

int moveittocenter(void)
{
	tagit();
	currentx += signum(currentx, FELDX);
	currenty += signum(currenty, FELDY);
	if (fitsitnot()==2)
	{
		return 0;
	}
	return 1;
}

void newbauteil(int deleter)
{	int result;

	if (deleter)
		currentteil = 0;
	else
	{
		do {
			result = rand()%MAXBAUTEIL+1;
		} while(0 == (charset&BAUTEIL[result].active)
		  || rand()%100 > BAUTEIL[result].prob);
		currentteil = result;
	}
	currentrotation = RECHTS;
	currentx=FELDX>>1;
	currenty=FELDY>>1;
	bauteil2disp();
	newtime = 1; /* flag to routine maus() to start new ticker; */
}


int turnit(char direction)
{	int rotation = currentrotation;

	tagit();
	currentrotation = turn(direction, currentrotation);
	bauteil2disp();
	if (fitsitnot()==2)
	{
		currentrotation	= rotation;
		bauteil2disp();
		return 0;
	}
	return 1;
}

int fitsitnot(void)
{	int x, y, xx, yy, teil;

	for(dispstart(); displist(&x, &y, &teil);)
		if (!infeld(x,y))
			return 2;
	if (currentteil==0) /* deleter */
	{
		for(dispstart(); displist(&x, &y, &teil);)
			if (feld[x][y].teil != EMPTY)
				return 0;
		return 1;
	}

	for(dispstart(); displist(&x, &y, &teil);)
		if (feld[x][y].teil != EMPTY)
			return 1;
	for(dispstart(); displist(&x, &y, &teil);)
	{	int rein, raus;

		/* teste an den Rand */
		reinraus(teil, &rein, &raus);
		xx = x; yy = y;
		rot2delta(rein, &xx, &yy);
		if (!infeld(xx, yy))
			return 1;
		xx = x; yy = y;
		rot2delta(raus, &xx, &yy);
		if (!infeld(xx, yy))
			return 1;
	}
	return 0;
}

void showit(int mode)
{	int x, y, teil;

	for(dispstart(); displist(&x, &y, &teil);)
	{
/*		gotoxy(x+2,FELDY-y+1); */
/*		putchar(TEIL[teil].picture); */
		putimage(XOFFSET+x*WIDTH, YOFFSET-y*WIDTH, TEIL[teil].image, mode);
		feld[x][y].update=0;
	}
}

int inout(int x, int y, int *xin, int *yin, int *xout, int *yout)
{	int rein, raus, teil;

	if (!infeld(x,y))
		return 0;
	*xin = *xout = x;
	*yin = *yout = y;
	teil = feld[x][y].teil;
	if (teil != EMPTY)
	{
		reinraus(teil, &rein, &raus);
		rot2delta(rein, xin, yin);
		rot2delta(raus, xout, yout);
		if (!infeld(*xin, *yin))
			return 0;
		if (!infeld(*xout, *yout))
			return 0;
		return 1;
	}
	return 2;
}

void removeloop(int loop)
{	int x, y;
	int sc=0;

	for(y=0; y<FELDY; y++)
		for(x=0; x<FELDX; x++)
			if (feld[x][y].update & 2)
			{
				feld[x][y].update &= ~2;
				if (loop)
				{
					sc ++;
					feld[x][y].teil = EMPTY;
					feld[x][y].update = 1;
				}
			}
	if (loop)
	{
		score += sc*sc/16*(level+1);
		bonustime=0;
	}
}

int testloop(void)
{	int x, y, xin, yin, xout, yout, x1, y1, res, rr, teil;

	x = currentx;
	y = currenty;

	feld[x][y].update |= 2;
	res = inout(x,y,&xin, &yin, &xout, &yout);
	if (res == 2)
		return 1;
	if (res == 0)
		return 0;
	rr = turn('r', turn('r', TEIL[feld[x][y].teil].raus));
	teil = feld[xout][yout].teil;
	if (rr != TEIL[teil].rein && rr != TEIL[teil].raus)
	{
		xout = xin; /* fuer deleter noetig */
		yout = yin; /* andere Richtung einschlagen */
	}

	while(xout != currentx || yout != currenty)
	{
/*		gotoxy(xout+2, FELDY-yout+1); */
/*		putchar('?'); */

		res = inout(xout, yout, &xin, &yin, &x1, &y1);
		if (res == 2)
			return 1; /* OK, (x,y) zeigt ins Leere */
		if (res == 0)
			return 0; /* an den Rand, passiert aber nicht! */
		if (x==x1 && y==y1)
		{
			x = xout;
			y = yout;
			xout = xin;
			yout = yin;
		}
		else	
		if (x==xin && y==yin)
		{
			x = xout;
			y = yout;
			xout = x1;
			yout = y1;
		}
		else
			return 1; /* Kein loop aber ok */
		feld[x][y].update |= 2;	/* 2 is tag for a possible loop */
	}
	/* It's a loop! */
	loops++;
	removeloop(1);
	return 1; /* and ok */
}

int enterit(void)
{	int x, y, teil, res;

	if (fitsitnot())
		return 0;
	if (currentteil==0) /* deleter */
	{
		teil = feld[currentx][currenty].teil;
		testloop();
		removeloop(1);
		feld[currentx][currenty].teil = teil;
		testloop();
		removeloop(1);
		return 1;
	}
	/* it's ok here, to place it */
	for(dispstart(); displist(&x, &y, &teil);)
	{
		feld[x][y].teil = teil;
		feld[x][y].update = 1;
		bonustime += feld[x][y].bonustime;
		feld[x][y].bonustime = 0;
		bonustime = min(bonustime, 3);
	}
	res = testloop();
	removeloop(0);
	return res;
}


void timebonus(void)
{   int tim[] = {1,1,2,3};
	int i,x,y, dead;

	dead=0;
	for(i=0; i<(int)(sizeof(tim)/sizeof(int)); i++)
	{
		do {
			x = rand() % FELDX;
			y = rand() % FELDY;
			if(++dead>1000)
				return;
		} while(feld[x][y].teil != EMPTY || feld[x][y].bonustime);
		feld[x][y].bonustime = tim[i];
		feld[x][y].update = 1;
	}
}

void clear(void)  /* loescht das Spielfeld */
{	int i, j;

	for(j=0; j<FELDY; j++)
		for(i=0; i<FELDX; i++)
		{
			feld[i][j].teil = EMPTY;
			feld[i][j].update = 1;
			feld[i][j].bonustime = 0;
		}
	for(j=0; j<level; j++)
		timebonus();
}

void show(void)
{	int i, j, b;
	char text[]="0s";

	settextjustify(CENTER_TEXT, CENTER_TEXT);

	for(j=0; j<FELDY; j++)
		for(i=0; i<FELDX; i++)
			if (feld[i][j].update)
			{
/*				gotoxy(i+2,FELDY-j+1); */
/*				putchar(TEIL[feld[i][j].teil].picture); */
				putimage(XOFFSET+i*WIDTH, YOFFSET-j*WIDTH,
					TEIL[feld[i][j].teil].image, COPY_PUT);
				feld[i][j].update=0;
				*text = '0'+(b=feld[i][j].bonustime);
				if (b)
				{
					setcolor(4*b);
					outtextxy(XOFFSET+i*WIDTH+WIDTH/2, YOFFSET-j*WIDTH+WIDTH/2,
						text);
				}
			}
}

void showint(long num, int n, int x, int y, IMAGES image)
{	int r;
	clock_t t;

	t = clock() + 3*n;
	while(n>0)
	{   int i;
		r = (int)(num%10);
		num = num/10;

		n--;
		setviewport(x+n*(X+1), y, X+x+n*(X+1), Y+y, 1);
		for(i=10; i>=0; i-=1)
			putimage(0,-i, image[r], COPY_PUT);
		setviewport(0,0,getmaxx(), getmaxy(), 0);
	}
	while(clock()<t)
		if (kbhit())
			break;
}

static lloops, ldeleters;
static long lscore, nextdel;
void showscore(void)
{

	if (loops>nextdel)
	{
		nextdel += 5;
		deleters++;
		if (deleters>9)
			deleters = 9;
	}
	if (score!=lscore)
		showint(score, 5, getmaxx()-5*(X+1), getmaxy()-Y, numbers);
	if (loops!=lloops)
	{
		showint(loops, 3, 0, getmaxy()-Y, numbers);
		if (loops >= 10*(1+level) && level<9)
		{
			level++;
			beep();
			delay(40);
			beep();
			timebonus();
		}
	}
	if (ldeleters != deleters)
		showint(deleters, 1, 240, getmaxy()-Y, numbers);
	lscore = score;
	lloops = loops;
	ldeleters  = deleters;
}


/* 0-100 */
void showtime(int time)
{	int x, y;
	if (time>100)
		time = 100;
	if (time<0)
		time = 0;
	setcolor(1);
	x = getmaxx()/2-50;
	y = YOFFSET+WIDTH+10;
	rectangle(x-1,y-1,x+101,y+11);
	if (time != 100)
	{
		setfillstyle(SOLID_FILL, 13);
		bar(x,y,x+100-time,y+10);
	}
	if (time == 0)
		return;
	setfillstyle(SOLID_FILL, 14);
	bar(x+101-time,y,x+100,y+10);
}


char *name = NULL;
struct high {
	long score;
	int loops;
	char name[22];
};

int high_cmp(const void *aa, const void *bb)
{   long a, b;

	a = ((struct high *)aa)->score;
	b = ((struct high *)bb)->score;
	if (a == b)
		return 0;
	return (a < b ? +1 : -1);
}

void highscore(int onlyshow)
{   FILE *f;
	int i,j;
	struct high { long score; int loops; char name[22]; } high[21];

	f = fopen("glups.sco", "rb");
	if (f)
	{
		fread(high, sizeof(struct high), 20, f);
		fclose(f);
	}
	else
	{	/* Neue Highscoreliste generieren */
		for(i=0; i<20; i++)
		{
			high[i].score = 250*(20-i);
			high[i].loops = 2*(20-i);
			strcpy(high[i].name, "Glups");
		}
	}
	i = -1;
	if (!onlyshow)
	{	int numberofentries, leastentry;
		long leastscore=0x7fffffffl;

		high[20].score = score;
		high[20].loops = -loops; 	/* it's a tag */
		strncpy(high[20].name, name, 21);
		high[20].name[21] = 0;
		numberofentries = 0;
		for(i=0; i<21; i++)
		{
			if (0==strncmp(name, high[i].name, 21))
			{
				if (high[i].score<leastscore)
				{
					leastentry = i;
					leastscore = high[i].score;
				}
				numberofentries++;
			}
		}
		if (numberofentries>3)
			high[leastentry].score = 0;
		qsort(high, 21, sizeof(struct high), high_cmp);
		for(i=0; i<20; i++)
		{
			if (high[i].loops < 0)
			{
				high[i].loops = -high[i].loops;
				break;
			}
		}
		if (i>=20)
			return; /* kein highscore */
#if 0
		if (score<=high[19].score)
			return;
		for(i=19; i>=-1; i--)
		{
			if (i<0 || score<=high[i].score)
			{
				i++;
				high[i].score = score;
				high[i].loops = loops;
				strncpy(high[i].name, name, 21);
				high[i].name[21] = 0;
				break;
			}
			if (i)
			{
				high[i].loops = high[i-1].loops;
				high[i].score = high[i-1].score;
				strcpy(high[i].name, high[i-1].name);
			}
		}
#endif
		f = fopen("glups.sco", "wb");
		if (f)
		{
			fwrite(high, sizeof(struct high), 20, f);
			fclose(f);
		}
	}
	cleardevice();
	settextjustify(LEFT_TEXT, CENTER_TEXT);
	outtextxy(220, 20, "Name");
	outtextxy(130, 20, "Score");
	outtextxy(60, 20, "Loops");
	for(j=0; j<20; j++)
	{	char buf[20];

		setcolor(i==j ? 15 : 1);
		outtextxy(220, j*16+40, high[j].name);
		outtextxy(130, j*16+40, ltoa(high[j].score, buf, 10));
		outtextxy(60,  j*16+40, itoa(high[j].loops, buf, 10));
		outtextxy(30,  j*16+40, itoa(j+1, buf, 10));
	}
	getch();
}


void loop(void)
{	int ch;
	extern int maus(void);
	extern void waitforrelease(void);

	loops = 0;
	score = 0;
	deleters = 2;
	trys = 3;
	level = ilevel%10;
	bonustime = 0;
	charset = 1<<(ilevel/10);

	lscore=-1;
	lloops=-1;
	ldeleters=-1;
	nextdel=5;

	setrgbpalette(LOOPPAL, 20, 63, 20);
	cleardevice();
	clear();
	show();
	showscore();
	newbauteil(0);
	showit(NOT_PUT);

	while(trys>0 && 'q' != (ch = maus()))
	{
/*		gotoxy(1,23); */
/*		putchar(ch); */
		switch(ch)
		{
		case 'c':
			clear();
			break;
		case 'd':
			tagit();
			if (deleters>0)
			{
				newbauteil(1);
				deleters--;
			}
			else
				beep();
			break;
		case 'n':
			tagit();
			if (trys>0)
			{
				trys--;
				setrgbpalette(LOOPPAL, trys<=1?63:0, 0, trys==2?63:0);
				if (trys)
					waitforrelease();
				newbauteil(0);
			}
			break;
		case '+':
			level++;
			timebonus();
			break;
		case 'l':
			while (!turnit('l'))
				moveittocenter();
			break;
		case 'r':
			while (!turnit('r'))
				moveittocenter();
			break;
		case ' ':
			if (enterit())
				newbauteil(0);
			else
				beep();
			break;
		case '6':
			(void) moveitrel(1,0);
			break;
		case '4':
			(void) moveitrel(-1,0);
			break;
		case '8':
			(void) moveitrel(0,1);
			break;
		case '5':
			(void) moveitrel(0,-1);
			break;
		}
		if (trys>0)
			showit(NOT_PUT);
		show();
		showscore();
	}
	if (!trys)
	{
		showtime(0);
		beep();
		delay(40);
		beep();
		delay(40);
		beep();
		getch();
		highscore(0);
	}
}

void pipe(int dir)
{   int x, y;

	if (!dir)
		return;

	x = (WIDTH>>1) + 1;
	y = (WIDTH>>1) + 1;

	setfillstyle(SOLID_FILL, 15);
	switch(dir)
	{
	case OBEN:
		bar(x-4,y+4,x+4,1);
		break;
	case UNTEN:
		bar(x-4,y-4,x+4,WIDTH);
		break;
	case RECHTS:
		bar(x-4,y-4,WIDTH,y+4);
		break;
	case LINKS:
		bar(x+4,y-4,1,y+4);
		break;
	}
}


void showbauteile(void)
{   int i, x, y;
	unsigned set;
	char buf[10];

	x = 1;
	y = -2;
	set = 1<<(ilevel/10);
	cleardevice();
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	currentrotation = RECHTS;
	for(i=1; i<=MAXBAUTEIL; i++)
	{
		if (BAUTEIL[i].active & set)
		{
			currentteil = i;
			currentx = x;
			currenty = y;
			bauteil2disp();
			showit(COPY_PUT);
			outtextxy(XOFFSET+x*WIDTH+WIDTH/2,
					  YOFFSET-y*WIDTH+WIDTH/2,
					  itoa(BAUTEIL[i].prob, buf, 10));
			x += 4;
			if (x>FELDX-2)
			{
				x = 1;
				y += 4;
			}
		}
	}
	settextjustify(CENTER_TEXT, TOP_TEXT);
	outtextxy(getmaxx()/2, 0, "The numbers are probabilities for the preglups to appear.");
	stars();
	getch();
}


void generate(int i)
{	int j, e;
	void far *p;

	e = TEIL[i].picture == '.';

	setfillstyle(SOLID_FILL, e?1:12);
	bar(1,1,WIDTH,WIDTH);
	for(j=0;j<3;j++)
	{
		setcolor(e?4:11);
		line(1+j,1+j,WIDTH-j,1+j);         /*oben */
		setcolor(e?10:3);
		line(1+j,WIDTH-j,WIDTH-j,WIDTH-j); /*unten */
		setcolor(e?6:9);
		line(1+j,1+j,1+j,WIDTH-j);         /*links */
		setcolor(e?8:5);
		line(WIDTH-j,1+j,WIDTH-j,WIDTH-j); /*rechts */
	}
	pipe(TEIL[i].rein);
	pipe(TEIL[i].raus);
	if (TEIL[i].picture == '*')
	{
		for(j=1; j<(WIDTH>>2); j++)
		{
			setcolor((j%12)+1);
			circle((WIDTH>>1)+1,(WIDTH>>1)+1,j<<1);
		}
	}

	p = farmalloc(SIZE);
	assert(p);
	getimage(1,1,WIDTH,WIDTH, p);
	TEIL[i].image = p;
/*	getch(); */
}

void constants(void)
{   int x, y;

	x = getmaxx();
	y = getmaxy();

	x /= FELDX;
	y /= FELDY;

	WIDTH = min(x,y);

	XOFFSET = (getmaxx()-(FELDX*WIDTH)) >> 1;
	YOFFSET = XOFFSET + WIDTH*(FELDY-1);

	mypalette();

	SIZE = imagesize(1,1,WIDTH,WIDTH);
	for(x=0; x<MAXTEIL; x++)
		generate(x);

	cleardevice();
	setcolor(1);
	settextjustify(CENTER_TEXT, TOP_TEXT);
	settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
	outtextxy(getmaxx()/2, getmaxy()/2, "Glups "VERSION);
	outtextxy(getmaxx()/2, 3*getmaxy()/4, "Programmed by Martin Lercher, Feb 1993");
}


void showhelp(void)
{   FILE *f;
	int h;
	char buf[120];

	cleardevice();
	h = textheight("M")*3/2;
	f = fopen("glups.hlp", "r");
	if (f)
	{   int y;

		settextjustify(LEFT_TEXT, TOP_TEXT);
		y = 0;
		while(fgets(buf, sizeof(buf)-1, f))
		{
			if (*buf)
				buf[strlen(buf)-1]=0;
			outtextxy(0, y, buf);
			y += h;
		}
	}
	else
	{
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		outtextxy(getmaxx()>>1, getmaxy()>>1, "*** glups.hlp offline ***");
	}
	getch();
}

int MAXLEVEL, anzlevels;

int sellevel(void)
{	int ch, stellen;

	stellen = anzlevels>9 ? 3 : 2;
	MAXLEVEL = 9+10*anzlevels;
restart:
	setcolor(1);
	cleardevice();
	settextjustify(CENTER_TEXT, TOP_TEXT);
	outtextxy(getmaxx()/2, 0,  "Press 'q' to quit, ' ' to start or '+', '-' to select Level.");
	outtextxy(getmaxx()/2, 10, "Press 's' to show the preglups, 'CsrUp', 'CsrDn' to change them.");
	outtextxy(getmaxx()/2, 21, "Press 'h' to see the highscore list, 'i' to see the instructions.");

	showint(12349l,6,(getmaxx()-(X+1)*6)/2,getmaxy()/3,glups);
	showint(567618l,6,0,getmaxy()-Y, glups);
	do {
		showint(ilevel, stellen, (getmaxx()+X)>>1, getmaxy()-Y, numbers);
		stars();
		ch = getch();
		if (!ch)
			ch = getch();
		if (ch == 'h')
		{
			highscore(1);
			goto restart;
		}
		if (ch == 's')
		{
			showbauteile();
			goto restart;
		}
		if (ch == 'i')
		{
			showhelp();
			goto restart;
		}
		if (ch == 72 && ilevel < MAXLEVEL-9)
			ilevel +=10;
		if (ch == '+' && ilevel < MAXLEVEL)
			ilevel++;
		if (ch == 80 && ilevel > 9)
			ilevel -= 10;
		if (ch == '-' && ilevel > 0)
			ilevel--;
	} while(ch != '\r' && ch !=' ' && ch != 'q');
	return ch != 'q';
}

void loadbauteile(void)
{	unsigned active;
	int i, center, prob;
	char buf[132];
	FILE *f;

	/* Here comes the deleter */
	BAUTEIL[0].active = 0xffffu;  /*don't care */
	BAUTEIL[0].prob = 100;        /*don't care */
	BAUTEIL[0].center = 1;
	BAUTEIL[0].what = "*";

	for(i=1; i<=MAXBAU; i++)
		BAUTEIL[i].active = 0;

	f = fopen("glups.bau", "r");
	if (f)
	{	int ch;

		while ((ch = fgetc(f)) == '#')
			fgets(buf, sizeof(buf)-1, f); /* ignore first line */
		ungetc(ch, f);
		fscanf(f, "%d\n", &anzlevels);  /* 2nd line is anzlevels */
		anzlevels--;
		for(i=1; i<=MAXBAU; i++)
		{
			if (fscanf(f, "%x%d%d%s\n", &active, &prob, &center, buf) != 4)
				break;
			BAUTEIL[i].active = active;
			BAUTEIL[i].prob = prob;
			BAUTEIL[i].center = center;
			assert((BAUTEIL[i].what = strdup(buf)) != NULL);
		}
		MAXBAUTEIL = i-1;
		fclose(f);
	}
	else
	{
		puts("*** glups.bau offline, aborting.");
		exit(1);
	}
}

int main(int argc, char **argv)
{   int gdriver = VGA, gmode=VGAHI, errorcode;
	char buf[80];
	extern void mausinit(void);

	srand((int)time(NULL));
	loadbauteile();
	if (argc>1)
		name = argv[1];
	if (!name)
		name = getenv("NAME");
	if (!name)
	{
		printf("(Argument empty, EnvVar NAME empty) Your name please: ");
		gets(buf);
		if (*buf)
			name = buf;
	}
	if (!name)
		name = "Glupsplayer";

	(void) registerbgidriver(EGAVGA_driver);
	initgraph(&gdriver, &gmode, "");
	errorcode = graphresult();
	if (errorcode != grOk)  /* an error occurred */
	{
	  printf("Graphics error: %s\n", grapherrormsg(errorcode));
	  printf("I need a VGA to work.");
	  getch();
	  return 1;             /* return with error code */
	}

	mausinit();
	constants();
	load("numbers", numbers);
	load("glups", glups);
	getch();

	ilevel = 4;

	if (argc>2 && strcmp(argv[1], "Martin")==0)
	{
		score = atol(argv[2]);
		loops = 111;
		highscore(0);
	}
	else
	{
		while(sellevel())
			loop();
	}

	closegraph();
	return 0;
}

