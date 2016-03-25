/* mosaic.c - a game of assembling colors
 *
 * Dipl. Math. Martin Lercher
 * Schloesslgartenweg 40
 * 93149 Nittenau
 */

#ifdef __MSDOS__
# define ATARI 0
# ifndef __SMALL__
#  error "Small model please"
# endif
#else
# define ATARI 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <graphics.h>
#include <assert.h>

#if ATARI
# include <ext.h>
# include <vdi.h>
extern int graf_handle(int *, int *, int *, int *); /* from gem.h */
extern void graf_mouse(int, void *);
#else
# include <dos.h>
# include <conio.h>
#endif

#define VERSION "V1.1a"

enum {
	X = 24,
	Y = 24,
	EMPTY = -1,
	EXTERNAL = -2,
	PIECES = 81,
	COUNTIT = 0,
	DONE = 1,
};

struct Feld {
	int col;
	int flood;
};

int piece[PIECES];
int thepiecenr, thex, they;
struct Feld feld[X+2][Y+2];
int nextn;
int onboard[3];
long score, thisscore;
int XS,YS,XO,YO;
int thescore[] = { 4,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
#if ATARI
int thecolor[] = { SOLID_FILL, XHATCH_FILL, CLOSE_DOT_FILL };
# define A
#else
int thecolor[] = { RED, BLUE, GREEN };
# define A +2
#endif

int event(int *rx, int *ry);

enum mousestatus { INIT=0, SHOW=1, HIDE=2 };
#if ATARI
static handle;
int ms_get(int* x,int*y)
{	int s;

	vq_mouse(handle, &s, x, y);
	return s;
}

void ms_do(enum mousestatus s)
{	int i;

	switch(s)
	{
	case INIT:
		handle = graf_handle(&i, &i, &i, &i);
		graf_mouse(0, NULL);
		break;
	case SHOW:
		v_show_c(handle, 0);
		break;
	case HIDE:
		v_hide_c(handle);
		break;
	}
}
#else
int ms_get(int* x,int*y)
{  struct REGPACK reg;
  reg.r_ax=3; intr(0x33,&reg);
  *x=reg.r_cx; *y=reg.r_dx; return(reg.r_bx);
}

void ms_do(enum mousestatus s)
{ struct REGPACK reg;
  reg.r_ax=(int)s; intr(0x33,&reg);
}
#endif

void init(void)
{	int i, j;

	/* Feld vorbereiten: E=EXTERNAL
	   EEEEEEEEE
	   E       E
	   E  XxY  E
	   E       E
	   EEEEEEEEE
	*/
	for(j=0; j<Y; j++)
	{
		feld[0][j+1].col = EXTERNAL;
		feld[X+1][j+1].col = EXTERNAL;
		for(i=0; i<X; i++)
			feld[i+1][j+1].col = EMPTY;
	}
	for(i=0; i<X+2; i++)
	{
		feld[i][0].col = EXTERNAL;
		feld[i][Y+1].col = EXTERNAL;
	}
	/* Feld initialisiert */

	/* Teile mischen */
	srand((int)time(NULL));
	for(i=0; i<PIECES; i++)
		piece[i]=EMPTY;
	for(i=0; i<PIECES; i++)
	{
		while(piece[j=rand()%PIECES] != EMPTY)
			putchar('.');
		piece[j] = i;
	}
	puts("\nTeile gemischt.");
	/* Teile gemischt. */
	
	thepiecenr = 0;
	thex = X >> 1;
	they = Y >> 1;
	onboard[0] = 0;
	onboard[1] = 0;
	onboard[2] = 0;
	score = 0;
}

#define x2g(x) (XO+(x)*XS)
#define y2g(y) (YO-(y)*YS)
#define g2x(x) ( ((x)-(XS>>1)-XO)/XS)
#define g2y(y) (-((y)+(YS>>1)-YO)/YS)

void drawatom(int x, int y, int c)
{
	if (c==EMPTY)
		setfillstyle(EMPTY_FILL, 0);
	else
#if ATARI
		setfillstyle(thecolor[c], BLACK);
	bar(x2g(x)+1, y2g(y)+1, x2g(x+1)-1, y2g(y+1)-1);
#else
		setfillstyle(SOLID_FILL, thecolor[c]);
	bar(x2g(x)+1, y2g(y)-1, x2g(x+1)-1, y2g(y+1)+1);
#endif
}

#if ATARI
# define min(x,y) ((x)<(y)?(x):(y))
#endif

#pragma warn -eff
void initgr(void)
{	int i, j;
	char buf[30];

	ms_do(INIT);

	XS = YS = min(19*getmaxx()/X/20, 19*getmaxy()/Y/20);
	XO = XS << 2;
	YO = (getmaxy() + YS * Y) >> 1;

#if !ATARI
	setpalette(BLACK, WHITE);
	setpalette(WHITE, BLACK);
#endif

	settextjustify(CENTER_TEXT, CENTER_TEXT);
	outtextxy(getmaxx()>>1, getmaxy()>>1, "Mosaic "VERSION);
	outtextxy(getmaxx()>>1, 10*getmaxy()/9>>1, "Martin Lercher 1994");
	while(300==event(&i, &j))
		/* */;
	cleardevice();


	for(i=0; i<X+1; i++)
		line(x2g(i), y2g(0), x2g(i), y2g(Y));
	for(j=0; j<Y+1; j++)
		line(x2g(0), y2g(j), x2g(X), y2g(j));
	for(i=1; i<5; i++)
		rectangle(x2g(0)-i, y2g(0)+i, x2g(X)+i, y2g(Y)-i);

	settextjustify(LEFT_TEXT, BOTTOM_TEXT);
	drawatom(X+2, Y-2, 0);
	drawatom(X+2, Y-4, 1);
	drawatom(X+2, Y-6, 2);
	outtextxy(x2g(X+1)-10, y2g(Y-8)-2, "Score:");
	outtextxy(x2g(X+1)-10, y2g(Y-9)-2, "Piece:");
	if (thescore[nextn] != 1)
	{
		sprintf(buf, "Score *%d for", thescore[nextn]);
		outtextxy(x2g(X+1)-10, y2g(Y-12)-2, buf);
		outtextxy(x2g(X+1)-10, y2g(Y-13)-2, "Preview");
	}
	outtextxy(x2g(X+1), y2g(1)-2, "Press 'q'");
	outtextxy(x2g(X+1), y2g(0)-2, "to quit.");
}
#pragma warn +eff

void drawpieceat(int x, int y, int i)
{
	drawatom(x,  y,   i%3); i /= 3;
	drawatom(x+1,y,   i%3); i /= 3;
	drawatom(x,  y+1, i%3); i /= 3;
	drawatom(x+1,y+1, i%3);
}

void drawitat(int x, int y)
{
	drawpieceat(x, y, piece[thepiecenr]);
}

void drawfeld(int x, int y)
{
	drawatom(x, y, feld[x+1][y+1].col);
}

void removeitat(int x, int y)
{
	drawfeld(x,   y);
	drawfeld(x+1, y);
	drawfeld(x,   y+1);
	drawfeld(x+1, y+1);
}

void show(int i)
{
	if (i+1+thepiecenr < PIECES)
		drawpieceat(-3, 3*i, piece[thepiecenr+i+1]);
	else
	{
		drawatom(-3, 3*i,   EMPTY);
		drawatom(-2, 3*i,   EMPTY);
		drawatom(-3, 3*i+1, EMPTY);
		drawatom(-2, 3*i+1, EMPTY);
	}
}

void shownext(int n)
{	int i;

	assert(3*(n-1)<Y);
	for(i=0; i<n; i++)
		show(i);
}

void moveitto(int x, int y)
{
	if (x == thex && y == they)
		return;
	if (!(x<0 || x >= X-1 || y < 0 || y >= Y-1))
	{
		removeitat(thex, they);
		thex = x;
		they = y;
		drawitat(thex, they);
	}
}

void moveit(int x, int y)
{
	x += thex;
	y += they;
	moveitto(x, y);
}

int ccolor;

/* x und y sind um 1 verschoben! (Performance) */
void flood(int x, int y)
{	struct Feld *f;

	f = &feld[x][y];
	if (f->flood != COUNTIT || f->col != ccolor)
		return;
	score += thescore[nextn];
	f->flood = DONE;

	flood(x+1, y);
	flood(x-1, y);
	flood(x, y+1);
	flood(x, y-1);
}

void enterat(int x, int y, int c)
{
	feld[x+1][y+1].col = c;
	onboard[c]++;
}

int testat(int x, int y)
{
	return feld[x+1][y+1].col == EMPTY;
}

int enterit(int x, int y)
{	int i, j;

	if (!(testat(x,y) && testat(x+1, y) &&
	    testat(x,y+1) && testat(x+1, y+1)))
	    return 0;
	    
	for(j=0; j<Y+2; j++)
		for(i=0; i<X+2; i++)
		  feld[i][j].flood = COUNTIT;

	i = piece[thepiecenr];
	enterat(x  ,y  ,i%3); i /= 3;
	enterat(x+1,y  ,i%3); i /= 3;
	enterat(x  ,y+1,i%3); i /= 3;
	enterat(x+1,y+1,i%3);

	thisscore = score;
	i = piece[thepiecenr];
	ccolor = i%3; flood(x+1, y+1); i /= 3;
	ccolor = i%3; flood(x+2, y+1); i /= 3;
	ccolor = i%3; flood(x+1, y+2); i /= 3;
	ccolor = i%3; flood(x+2, y+2);
	thisscore = score - thisscore;

	return 1;
}

void showscores(void)
{	char buf[20];

	setfillstyle(EMPTY_FILL, 0);
	bar(x2g(X+3), y2g(Y-1), x2g(X+6), y2g(Y-9));
	sprintf(buf, "%d/108", onboard[0]);
	outtextxy(x2g(X+3) A, y2g(Y-2)-2, buf);
	sprintf(buf, "%d/108", onboard[1]);
	outtextxy(x2g(X+3) A, y2g(Y-4)-2, buf);
	sprintf(buf, "%d/108", onboard[2]);
	outtextxy(x2g(X+3) A, y2g(Y-6)-2, buf);
	outtextxy(x2g(X+3) A, y2g(Y-8)-2, ltoa(score, buf, 10));
	outtextxy(x2g(X+3) A, y2g(Y-9)-2, ltoa(thisscore, buf, 10));
}


int event(int *rx, int *ry)
{	int mx, my, mf, ch;
static int x=-1000, y, f=-1;

	while(1)
	{
		mf = ms_get(&mx, &my);
		mx = g2x(mx);
		my = g2y(my);

		if (kbhit())
		{
			ch = getch();
			if (!ch)
				ch = getch();
			return ch;
		}
		if (f != mf)
		{
			f = mf;
			if (f)
				return ' ';
		}
		if (x != mx || y != my)
		{
			*rx = x = mx;
			*ry = y = my;
			return 300;
		}
	}
}

void gameloop(void)
{	int cont=1, ch, i;
	int x, y;

	shownext(nextn);
	drawitat(thex, they);
	while(cont)
	{
		ms_do(SHOW);
		ch = event(&x, &y);
		ms_do(HIDE);

		switch(ch)
		{
		case 300:
			moveitto(x, y);
			break;
		case 0x4b:
		case '4':
			moveit(-1,0);
			break;
		case 0x4d:
		case '6':
			moveit(1,0);
			break;
		case 0x48:
		case '8':
			moveit(0,1);
			break;
		case 0x50:
		case '5':
		case '2':
			moveit(0,-1);
			break;
		case ' ':
		case '\r':
			for(i=0; i<3; i++)
			{
				delay(80);
				removeitat(thex, they);
				delay(80);
				drawitat(thex, they);
			}
			if (!enterit(thex, they))
			{
				putchar('\a');
				break;
			}
			showscores();
			if (++thepiecenr < PIECES)
			{
				shownext(nextn);
				drawitat(thex, they);
			}
			else
			{
				cont = 0; /* hurra */
				while(event(&x, &y)==300)
					/* */;
			}
			break;
		case 'q':
			cont = 0;
			break;
		}	
	}
}

void usage(void)
{
	puts("\nusage: mosaic [#preview]\n");
	puts("Ziel des Spieles ist es, moeglichst grosse, zusammenhaengende,");
	puts("einfarbige Gebiete zu legen. Dafuer gibt es Punkte. Und zwar umso mehr, je");
	puts("kleiner die Vorschau auf der linken Seite ist.\n");
	puts("Bedienung:");
	puts("Mit den Cursortasten hin und her fahren, mit der Leertaste oder mit");
	puts("der Eingabetaste das Teil platzieren. Mit der Taste 'q' kann man das Spiel");
	puts("vorzeitig beenden.                                             Viel Spass.");
}

void highscore(void)
{	FILE *f;
	int i, j;
	char n[10][40];
	long s[10];
	char buf[100], *name;
	
#if ATARI
	printf("\033E");
#endif

	f = fopen("mosaic.hsc", "r");
	if (!f)
	{
		for(i=0; i<10; i++)
		{
			s[i]=100*(10-i);
			s[i]=0;
			strncpy(n[i], "Mosaic "VERSION" player", sizeof(n[i]));
		}
	}
	else
	{
		fread(s, sizeof(s), 1, f);
		fread(n, sizeof(n), 1, f);
		fclose(f);
	}
	for(i=0; i<10; i++)
	{
		if (score && score>=s[i])
		{
			printf("You made it into the top ten! Place %d with %ld points.\n\n", i+1, score);
			for(j=9; j>i; j--)
			{
				s[j]=s[j-1];
				strncpy(n[j], n[j-1], sizeof(n[j]));
			}
			s[i] = score;
			name = getenv("MYNAME");
			if (!name)
			{
				name = buf;
				fputs("Please enter your name: ", stdout);
				gets(name);
			}
			strncpy(n[i], name, sizeof(n[i]));
			f = fopen("mosaic.hsc", "w");
			fwrite(s, sizeof(s), 1, f);
			fwrite(n, sizeof(n), 1, f);
			fclose(f);
			break;
		}
	}

		printf("\nThe top ten of MOSAIC\n~~~~~~~~~~~~~~~~~~~~~\n");
	for(i=0; i<10; i++)
		printf("%2d%9ld %s\n", i+1, s[i], n[i]);
#if ATARI
	while(300 == event(&i, &i))
		/* */;
#endif
}

int main(int argc, char **argv)
{	int a, b;

        puts("This is "__FILE__" "VERSION", compiled "__DATE__" "__TIME__".");
	puts("Coded by Martin Lercher.");

	nextn=2;

	if (argc==2)
	{
		nextn = atoi(argv[1]);
		if (!isdigit(*argv[1]) || nextn<0 || 3*(nextn)>Y)
		{
			usage();
			return 1;
		}
	}

	init();

	a = DETECT;
	b = 0;
#if !ATARI
	registerbgidriver(EGAVGA_driver);
#endif
	initgraph(&a, &b, "");
	initgr();
	gameloop();
	closegraph();
	highscore();
	return 0;
}
