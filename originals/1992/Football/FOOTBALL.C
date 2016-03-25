#define DOS 1

/* Das wird eine primitives Footballsimulation
 * (c) 1992 by Martin Lercher
 * Schloesslgartenweg 40
 * 8415 Nittenau
 * 09436/8475 oder 091/943-2793
 * September 1992
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if DOS
# include <conio.h>
# include "times.h"
# define DELTA 10
#else
# include <ext.h>
# define my_clock() clock()
# define CLKTCK CLK_TCK
# define DELTA 2
#endif
#include <ctype.h>
#include <graphics.h>

#include "football.h"
#include "opponent.h"
#include "menu.h"

int X, Y;	/* Bildschirmaufloesung */
struct position FIELD, BOX, SIZE, TEXT;
clock_t CLOCK;
int MaxColor, Color, NoColor, quit=0;
static clock_t	the_time=0;
int togo, down, direction, us, them, ticks, ticks1, ticks2;
struct info Yards, Togo, Down, Us, Them, Ticks1, Ticks2;
static saveme, savethis[8];
int kbrepeat;

static char SCCSID[]="@(#)"__FILE__" Compiled "__DATE__;

static void insert(int n)
{
	if (saveme<8)
	{	int i;
		for(i=saveme; i>0; i--)
			savethis[i] = savethis[i-1];
		*savethis = n;
		saveme++;
	}
}

static void unget(int n)
{
	if (saveme<8)
		savethis[saveme++] = n;
}

static int pop(void)
{
	if (saveme)
		return (savethis[--saveme]);
	return 0;
}

static void keyboard(void)
{	clock_t t;
	int ch;
	int d0, d1, d2;
static clock_t t1=0, t2=0, t3=0;
static int oldch = 0;
			
	if (!kbhit())
		return;
	t = my_clock();
	d0 = (int)(t -t1);
	d1 = (int)(t1-t2);
	d2 = (int)(t2-t3);
	d0 = d1-d0;
	d1 = d2-d1;
	ch = getch();
	if (!ch)
		ch = -getch();
	if (kbrepeat || ch != oldch || d0>DELTA || d0<-DELTA || d1>DELTA || d1<-DELTA)
		insert(oldch=ch);
	else
		d2 = d0; /* for debugger */
	t3 = t2;
	t2 = t1;
	t1 = t;
}

static int mykbhit(void)
{
	keyboard();
	return saveme;
}

int mygetch(void)
{
	while(!saveme)
		keyboard();
	return pop();
}

static void setinfo(struct info *info,
	int x, int y, char *text, char *format, int *number,
	int factor, int offset)
{
	info->pos.x = x * textwidth("M");
	info->pos.y = y * 3 * textheight("M") / 2;
	info->text = text;
	info->format = format;
	info->number = number;
	info->displayed = 32767;
	info->factor = factor;
	info->offset = offset;
	info->pos1.x = TEXT.x + info->pos.x + textwidth(info->text);
	info->pos1.y = TEXT.y + info->pos.y;
	outtextxy(TEXT.x + info->pos.x, TEXT.y + info->pos.y, info->text);
}

static void setconsts(void)
{
	MaxColor = getmaxcolor();
	NoColor = 0;
	if HIGHCOLOR
		Color = YELLOW;
	else
		Color = MaxColor;
	X = getmaxx()+1;
	Y = getmaxy()+1;
	FIELD.x = FIELD.y = Y/20;	/* Feldursprung */
	BOX.x = 19*X/FX/20; /* Groesse eines Feldpunkts */
	BOX.y = 8*Y/FY/20;
	SIZE.x = BOX.x * FX + FIELD.x;  /* Ausdehnung des Felds */
	SIZE.y = BOX.y * FY + 1 + FIELD.y;
	TEXT.x = FIELD.x;
	TEXT.y = FIELD.y + SIZE.y;
	
	srand((int)my_clock());
	CLOCK = CLKTCK / 20;
}


static void putinfo(struct info *info)
{	static char buf[40];
	int x, y, xx, yy;

	if (info->displayed == *info->number)
		return;
	info->displayed = *info->number;
	sprintf(buf, info->format, 
		*info->number * info->factor + info->offset);
	x = info->pos1.x;
	y = info->pos1.y;
	xx = textwidth(buf);
	yy = textheight(buf);
	if HIGHCOLOR
		setfillstyle(SOLID_FILL, BLUE);
	else
		setfillstyle(EMPTY_FILL, Color);
	bar(x, y, x+xx, y-yy);
	outtextxy(x, y, buf);
	keyboard();
}

void clear(int color)
{
	if HIGHCOLOR
	{
		setfillstyle(SOLID_FILL, color);
		bar(0,0,X,Y);
		setcolor(Color);
	}
	else
		cleardevice();
}

static void line_at(int x, int type)
{
	if (type!=SOLID_LINE && (x<AREA || x>FX-AREA))
		return;
	setlinestyle(type, 0x2222, 1);
	line(FIELD.x + x*BOX.x, FIELD.y, FIELD.x + x*BOX.x, SIZE.y-1);
}

static void rahmen(void)
{	struct player *fp;


	clear(BLUE);
	settextjustify(LEFT_TEXT, BOTTOM_TEXT);
	if HIGHCOLOR
	{
		setfillstyle(SOLID_FILL, GREEN);
		bar(FIELD.x, FIELD.y, SIZE.x, SIZE.y);
	}
	rectangle(FIELD.x, FIELD.y-1, SIZE.x, SIZE.y);
#if DOS
	{ int i;
	for(i=1; i<=AREA; i++)
	{
		line_at(i, SOLID_LINE);
		line_at(FX-i, SOLID_LINE);
	}
	}
#else
	setfillstyle(HATCH_FILL, Color);
	bar(FIELD.x, FIELD.y, FIELD.x+AREA*BOX.x, SIZE.y-1);
	bar(SIZE.x-1, FIELD.y, SIZE.x-AREA*BOX.x, SIZE.y-1);
	line_at(AREA, SOLID_LINE);
	line_at(FX-AREA, SOLID_LINE);
#endif
	fp = first_player();
	setinfo(&Yards,  0, 0, "Yards ", "%2d", &fp->pos.x, 1, 1);
	setinfo(&Togo,  11, 0, "to go ", "%2d", &togo, 1, 0);
	setinfo(&Down,  23, 0, "down ",  "%d",  &down, 1, 0);
	setinfo(&Ticks2,39, 0, "Time ",  "%2d", &ticks2, 1, 0);
	setinfo(&Ticks1,46, 0, ",",      "%1d", &ticks1, 1, 0);
	setinfo(&Us,    55, 0, "Us ",    "%2d", &us, 1, 0);
	setinfo(&Them,  63, 0, "Them ",  "%2d", &them, 1, 0);
}

static void show_statistics(void)
{
	putinfo(&Yards);
	putinfo(&Togo);
	putinfo(&Down);
	putinfo(&Us);
	putinfo(&Them);
	putinfo(&Ticks2);
	putinfo(&Ticks1);
}

static void points(int n)
{
	if (direction>0)
		us += n;
	else
		them += n;
}

static void putsym(int x, int y, enum symbol symbol)
{	int xx, yy;

	x = x*BOX.x+FIELD.x;
	y = y*BOX.y+FIELD.y;
	xx = x + BOX.x/2;
	yy = y + BOX.y/2;
	
	switch(symbol)
	{
	case INTENSE:
		setcolor(HIGHCOLOR ? BLUE : Color);
		for(x=5; x<BOX.x+BOX.x/2; x+=2)
			circle(xx,yy,x);
		break;
	case CIRCLES:
		setcolor(HIGHCOLOR ? RED : Color);
		for(x=3; x<BOX.x+BOX.x/2; x+=2)
			circle(xx,yy,x);
		break;
	case CLEAR:
		if HIGHCOLOR
			setfillstyle(SOLID_FILL, GREEN);
		else
			setfillstyle(EMPTY_FILL, Color);
		bar(x+1, y+1, x+BOX.x-1, y+BOX.y-1);
		break;
	case PL1:
		putpixel(xx, yy, Color);
		/* Fall */
	case PL2:
		rectangle(x+1, y+1, x+BOX.x-1, y+BOX.y-1);
		break;
	case PL3:
		setfillstyle(SOLID_FILL, HIGHCOLOR ? BLACK : Color);
		bar(x+1, y+1, x+BOX.x-1, y+BOX.y-1);
		break;
	case BALL:
		setfillstyle(CLOSE_DOT_FILL, Color);
		bar(x+2, y+2, x+BOX.x-1, y+BOX.y-1);
		break;
	}
	keyboard();
}

void the_player(struct player *p, int mode)
{
	if (mode == SHOW) 
		putsym(p->pos.x, p->pos.y, p->symbol);
	if (mode == HIDE) 
		putsym(p->pos.x-p->delta.x, p->pos.y-p->delta.y, CLEAR);
}

int testinfield(int x, int y)
{
	return (x>=0 && y>=0 && x<FX && y<FY);
}

static void infield(struct player *p)
{	int x, y;

	x = p->pos.x + p->delta.x;
	y = p->pos.y + p->delta.y;
	if (testinfield(x, y))
	{
		p->pos.x = x;
		p->pos.y = y;
	}
	else
	{
		p->delta.x = 0;
		p->delta.y = 0;
	}
}

static void busy(void)
{
	while(my_clock()<the_time)
		keyboard();
	the_time = my_clock()+CLOCK;
}

static void get_delta(struct player *p)
{	int ch;
	int x=0, y=0;
	
	if (!mykbhit())
		return;
	ch = mygetch();
	switch(ch)
	{
	case -'H':
	case '8':
		y = -1;
		break;
	case -'P':
	case '5':
		y = 1;
		break;
	case -'K':
	case '4':
		x = -1;
		break;
	case -'M':
	case '6':
		x = 1;
		break;
	case 'p':
	case '9':
		p->typ = BALLKEEPER;
		p->sleep = 1;
		break;
	case 'k':
	case '7':
		p->sleep = p->pos.x + direction*intervall(40,60);
		x = direction;
		p->typ = THE_BALL;
		p->symbol = BALL;
		break;
	case 'q':
		quit = 1;
		break;
	}
	if (x == -direction)
		x = 0;
	p->delta.x = x;
	p->delta.y = y;
}

static void move_player(struct player *p)
{	struct player *qb; /* Der Ballfuehrer */

	if (!p->active)
		return;
	qb = first_player();
	switch(p->typ)
	{
	case THE_BALL:
		if (p->pos.x != p->sleep)
			p->delta.x = direction;
		else
			p->deactivate = 1;
		break;
	case RUNNER:
		if (qb->typ == BALLKEEPER)
		{
			p->delta.x = qb->ticks;
			p->delta.y = qb->delta.y;
		}
		break;
	case BALLKEEPER:
		get_delta(p);
		p->ticks = p->delta.x;
		p->delta.x = 0;
		break;
	case PLAYER:
		get_delta(p);
		break;
	case BLOCKER:
		if (qb->typ == BALLKEEPER && qb->sleep == 1)
		{
			qb->sleep = 0; /* Flag fuer PASS zuruecksetzen */
			p->delta.y = qb->pos.y - p->pos.y;
			p->typ = RUNNER;
			break;
		}
		if (qb->typ != THE_BALL) /* not the Ball */
			p->delta = qb->delta;
		break;
	case OPPONENT:
		if (p->ticks-- <= 0)
		{	/* awake */
			if (p->doit)
				(*(move_func *)(p->doit))(p, qb);
			p->ticks = p->sleep;
		}
		break;
	}
	infield(p); /* updates deltas */
}

static void fix_pass(void)
{	struct player *qb, *runner;

	qb = first_player();
	if (qb->typ != THE_BALL)
		return;
	foreach(runner)
	{
		if (runner->active && runner->typ == RUNNER
			  && runner->pos.x == qb->pos.x
			  && runner->pos.y == qb->pos.y)
		{
			runner->active = 0;
			qb->pos.x = runner->pos.x;
			qb->pos.y = runner->pos.y;
			qb->typ = PLAYER;
			qb->symbol = PL1;
		}
	}
}

static int test_kollision(struct player *p)
{	struct player *o;
	int x,y;

	x = p->pos.x;
	y = p->pos.y;
	foreach(o)
		if (o!=p && o->active &&
		  o->pos.x == x &&
		  o->pos.y == y)
			return 1;
	return 0;
}

static void kollision(void)
{	struct player *p;
	
	foreach(p)
	{
		keyboard();
		if (p->active && test_kollision(p))
			p->deactivate = 1;
	}
}

static int moved(struct player *p)
{
	if (!p->active)
		return 0;
	if (p->deactivate)
	{
		p->active = 0;
		return 1;
	}
	return (p->delta.x || p->delta.y);
}

static void show_board(void)
{	struct player *p;

	foreach(p)
		if (moved(p))
			the_player(p, HIDE);
	foreach(p)
		if (moved(p))
			the_player(p, SHOW);
	show_statistics();
}

static void dotheclock(void)
{
	ticks++;
	if (ticks==22)
	{
		ticks = 0;
		ticks1++;
		if (ticks1==10)
		{
			ticks1=0;
			ticks2++;
		}
	}
}

static int move_players(int goal)
{	struct player *p;

	foreach(p)
		p->delta.x = p->delta.y = 0;
	foreach(p)
		move_player(p);
	fix_pass();
	kollision();
	p = first_player();
	togo = goal - p->pos.x;
	togo = direction*togo<0 ? 0 : direction*togo;
	dotheclock();
	show_board();
	busy();
	return p->pos.x;
}

struct player players[PLAYERS];

struct player *first_player(void)
{
	return players;
}

struct player *next_player(struct player *p)
{
	if (p && p+1<players+PLAYERS)
		return p+1;
	return NULL;
}

#if 0
void init(struct player **p, int x, int y,
	enum type typ, enum symbol symbol, int sleep, move_func *doit)
{
	(*p)->active = testinfield(
	(*p)->pos.x = x,
	(*p)->pos.y = y);
	(*p)->typ = typ;
	(*p)->sleep = sleep;
	(*p)->ticks = sleep;
	(*p)->deactivate = 0;
	(*p)->symbol = symbol;
	(*p)->doit = doit;
	*p = next_player(*p);
}
#endif

static void message(char *s)
{
	settextjustify(CENTER_TEXT, TOP_TEXT);
	outtextxy(FIELD.x+SIZE.x/2, 2, s);
}

static void pause(void)
{	struct player *p;

	if (quit)
		return;
	p = first_player();
	putsym(p->pos.x, p->pos.y, CIRCLES);
	message("<Enter>");
	while(mygetch() != '\r')
		;
}

static int touchdown(int x)
{
	return (x<AREA || x>=FX-AREA);
}


static int a_down(int start)
{	struct player *p;
	int pos = start;
	
	rahmen();
	the_time = 0;
	setup(start-2*direction, FY/2);
	foreach(p)
		if (p->active)
			the_player(p, SHOW);
	show_statistics();
	line_at(start+((1+direction)>>1), USERBIT_LINE);
	start += direction*togo;
	line_at(start+((1-direction)>>1), USERBIT_LINE);
	setlinestyle(SOLID_LINE, 0, 1);
	unget(mygetch());
	while(players->active && !touchdown(pos) && ticks2<15 && !quit)
	{
		pos = move_players(start);
	}
	down++;
	pause();
	return pos;
}

static int one_side(int start)
{	struct player *p;

	p = first_player();
	togo = 0;
	while(!quit)
	{
		if (togo == 0)  
		{
			down = 1;
			togo = 10;
		}
		if (down>4 || ticks2>=15)
			break;
		start = a_down(start);
		if (p->typ != PLAYER) /* Ball gekickt */
			break;
		if (touchdown(start))
		{
			points(3);
			break;
		}
	}
	if (touchdown(start))
	{
		start = direction<0 ? 20 : FX-20;
		points(2);
	}
	direction = -direction;
	return start;
}

static void playagain(void)
{
	quit = 0;
	message("Play, Menu or quit? <pymnq>");
	while(1)
	{	int ch;
	
		ch = tolower(mygetch());
		if (ch == 'y' || ch == 'j' || ch == 'p')
			break;
		if (ch == 'm')
		{
			menu();
			break;
		}
		if (ch == 'n' || ch == 'q')
		{
			quit = 1;
			break;
		}
	}
}

int main(void)
{	int driver = DETECT, mode, start;

	puts(SCCSID+4);
	loadschemes();
	initgraph(&driver, &mode, "");
	setconsts();
	kbrepeat = 0;
	rahmen();
	playagain();
	while(!quit)
	{
		us = them = 0;
		direction = 1;
		start = 20;
		ticks = ticks1 = ticks2 = 0;
		while(!quit && ticks2<15)
			start = one_side(start);
		playagain();
	}
	closegraph();
	return 0;
}

