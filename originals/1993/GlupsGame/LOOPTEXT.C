/* loops.c - Tetris mit Schleifen.
 * Martin Lercher, Schloesslgartenweg 40, 8415 Nittenau
 * Februar 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
/* #include <ext.h> */

enum { FELDX=20, FELDY=12, EMPTY=0, 
       OBEN=1, UNTEN=2, LINKS=3, RECHTS=4 };

struct Teil {
	int rein, raus;
	char picture;
};

struct Teil TEIL[] = {
	0,0, '.',
	LINKS, RECHTS, 205,
	OBEN, UNTEN, 186,
	OBEN, RECHTS, 200,
	OBEN, LINKS, 188,
	UNTEN, LINKS, 187,
	UNTEN, RECHTS, 201,
	0,0, '*'
};
#define MAXTEIL ((int)(sizeof(TEIL)/sizeof(struct Teil)))

struct Bauteil {
	int center; /* Darum wird gedreht */
	int active; /* wird durch rand() ausgewaehlt */
	char *what; /* - straight, r turn right, l turn left, * del */
};

struct Bauteil BAUTEIL[] = {
	1, 1, "*", /* This is the deleter */
	1, 1, "-",
	1, 1, "r",
	1, 1, "--",
	2, 1, "---",
	2, 1, "r-r",
    3, 1, "-r-r-",
    3, 1, "-r-l-",
    3, 1, "-l-r-",
    2, 1, "-r-",
};
#define MAXBAUTEIL ((int)(sizeof(BAUTEIL)/sizeof(struct Bauteil))-1)

#define MAXLEN 10
struct display {
	int dx, dy, teil;
} theDisplay[MAXLEN];
int currentrotation, currentteil, currentx, currenty;

struct FeldElement {
	int teil, update;
};

typedef struct FeldElement Feld[FELDX][FELDY];

Feld feld;
int score;

int fitsitnot(void);
int infeld(int x, int y);

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
		feld[x][y].update=1;
}

void moveitto(int x, int y)
{	int xx, yy;

	tagit();
	xx = currentx;
	yy = currenty;
	currentx = x;
	currenty = y;
	if (fitsitnot()==2)
	{
		currentx = xx;
		currenty = yy;
	}
}

void moveitrel(int x, int y)
{
	tagit();
	currentx += x;
	currenty += y;
	if (fitsitnot()==2)
	{
		currentx -= x;
		currenty -= y;
	}
}

void newbauteil(int deleter)
{	int result;

	if (deleter)
		currentteil = 0;
	else
	{
		do {
			result = rand()%MAXBAUTEIL+1;
		} while(!BAUTEIL[result].active);
		currentteil = result;
	}
	currentrotation = RECHTS;
	currentx=FELDX>>1;
	currenty=FELDY>>1;
	bauteil2disp();
}


void turnit(char direction)
{	int rotation = currentrotation;

	tagit();
	currentrotation = turn(direction, currentrotation);
	bauteil2disp();
	if (fitsitnot()==2)
	{
		currentrotation	= rotation;
		bauteil2disp();
	}
}

int fitsitnot(void)
{	int x, y, xx, yy, teil;

	for(dispstart(); displist(&x, &y, &teil);)
		if (!infeld(x,y))
			return 2;
	if (currentteil==0) /* deleter */
		for(dispstart(); displist(&x, &y, &teil);)
			if (feld[x][y].teil != EMPTY)
				return 0;
	
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

void showit(void)
{	int x, y, teil;

	for(dispstart(); displist(&x, &y, &teil);)
	{
		gotoxy(x+2,FELDY-y+1);
		putchar(TEIL[teil].picture);
		feld[x][y].update=0;
 	}	
}

int infeld(int x, int y)
{
	if (x>=0 && y >=0 && x<FELDX && y<FELDY)
		return 1;
	return 0;
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

	for(y=0; y<FELDY; y++)
		for(x=0; x<FELDX; x++)
			if (feld[x][y].update & 2)
			{
				feld[x][y].update &= ~2;
				if (loop)
				{
					score += 1;
					feld[x][y].teil = EMPTY;
					feld[x][y].update = 1;
				}
			}
}

int testloop(void)
{	int x, y, xin, yin, xout, yout, x1, y1, res;

	x = currentx;
	y = currenty;

	feld[x][y].update |= 2;
	res = inout(x,y,&xin, &yin, &xout, &yout);
	if (res == 2)
		return 1;
	if (res == 0)
		return 0;
	if (feld[xout][yout].teil == EMPTY)
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
		feld[x][y].update = 0;
	}
	res = testloop();
	removeloop(0);
	return res;
}


void clear(void)
{	int i, j;

	for(j=0; j<FELDY; j++)
		for(i=0; i<FELDX; i++)
		{
			feld[i][j].teil = EMPTY;
			feld[i][j].update = 1;
		}
}

void show(void)
{	int i, j;

	for(j=0; j<FELDY; j++)
		for(i=0; i<FELDX; i++)
			if (feld[i][j].update)
			{
				gotoxy(i+2,FELDY-j+1);
				putchar(TEIL[feld[i][j].teil].picture);
				feld[i][j].update=0;
			}
}

void loop(void)
{	int ch;

	score = 0;
	clear();
	show();
	newbauteil(0);
	showit();

	while('q' != (ch = getch()))
	{	
		gotoxy(1,23);
		putchar(ch);
		switch(ch)
		{
		case 'c':
			clear();
			showit();
			show();
			break;
		case 'd':
			tagit();
			newbauteil(1);
			showit();
			show();
			break;
		case 'n':
			tagit();
			newbauteil(0);
			showit();
			show();
			break;
		case 'l':
			turnit('l');
			showit();
			show();
			break;
		case 'r':
			turnit('r');
			showit();
			show();
			break;
		case 'm':
			moveitto(rand()%FELDX, rand()%FELDY);
			showit();
			show();
			break;
		case ' ':
			if (enterit())
			{
				newbauteil(0);
				showit();
				show();
			}
			else
				putchar('\007');
			break;
		case '6':
			moveitrel(1,0);
			showit();
			show();
			break;
		case '4':
			moveitrel(-1,0);
			showit();
			show();
			break;
		case '8':
			moveitrel(0,1);
			showit();
			show();
			break;
		case '5':
			moveitrel(0,-1);
			showit();
			show();
			break;
		}
	}	
}

int main(void)
{	
	clrscr();
	gotoxy(1,18);
	
	printf("Loops - Ein anderes Tetris, eins mit Schleifen.\n");
	printf("Press 'q' to quit.\n");

	loop();
		
	return 0;
}
