#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

#include "football.h"
#include "opponent.h"
#include "menu.h"

static struct scheme schemes[SCHEMES];
static currentscheme=0;

static char buf[80];

static void print(char *s, int lf)
{	int x, y;

	x = getx();
	y = gety();
	outtext(s);
	if (lf) 
		moveto(x, y + 3*textheight(s)/2);
}

static void rmnl(char *s)
{	char *t;

	t = s+strlen(s)-1;
	if (t>=s && *t == '\n')
		*t = 0;
}

static void help(void)
{	FILE *helpfile;

	clear(BLUE);
	moveto(0,0);
	helpfile = fopen("help", "r");
	if (!helpfile)
	{
		print("Helpfile not online!", 1);
		mygetch();
		return;
	}
	while(fgets(buf, (int)sizeof(buf), helpfile))
	{
		rmnl(buf);
		print(buf, 1);
	}
	fclose(helpfile);
	mygetch();
}

struct scheme *getscheme(void)
{
	return schemes+currentscheme;
}


static void init_data(struct data *d, int x, int y,
	enum typ typ, enum symbol symbol, int sleep, int fnr)
{
	d->active = 1;
	d->pos.x = x;
	d->pos.y = y;
	d->typ = typ;
	d->symbol = symbol;
	d->sleep = sleep;
	d->fnr = fnr;	
}

void loadschemes(void)
{	struct data *d;
	struct scheme *s;
	FILE *data;
	int i;
		
	for(i=1; i<SCHEMES; i++)
		schemes[i].active = 0;	
	schemes->active = 1;
	strcpy(schemes->name, "Default");
	d = schemes->data;
	init_data(d+0, 0, 0,  PLAYER,   PL1, 0, 0);
	init_data(d+1, 1, +1, BLOCKER,  PL2, 0, 0);
	init_data(d+2, 1, -1, BLOCKER,  PL2, 0, 0);
	init_data(d+3, 3, +1, OPPONENT, PL3, 6, 1); /* random_walk */
	init_data(d+4, 3, -5, OPPONENT, PL3, 0, 3); /* side_blocker */
	init_data(d+5, 4, 0,  OPPONENT, PL3, 10,1); /* random_walk */
	init_data(d+6, 4, +5, OPPONENT, PL3, 0, 3); /* side_blocker */
	init_data(d+7, 5, -1, OPPONENT, PL3, 12,4); /* terminator */
	init_data(d+8, 6, 0,  OPPONENT, PL3, 4, 2); /* ball_grabber */
	currentscheme = 0;

	data = fopen("data", "r");
	if (!data)
	{
		puts("Datafile not online!");
		mygetch();
		return;
	}
	
	while(fgets(buf, (int)sizeof(buf), data) && ++currentscheme<SCHEMES)
	{	
		s = getscheme();
		s->active = 1;
		rmnl(buf);
		strncpy(s->name, buf, sizeof(s->name)-1);
		s->name[sizeof(s->name)-1] = 0;
		d = s->data;
		for(i=0; i<PLAYERS; i++)
		{
			fscanf(data, "%d %d %d %d %d %d %d\n",
				&(d->active),
				&(d->pos.x),
				&(d->pos.y),
				&(d->typ),
				&(d->symbol),
				&(d->sleep),
				&(d->fnr)
			);
			d++;
		}
	}

	fclose(data);
	currentscheme = 0;
}

static void save(void)
{	FILE *data;
	struct scheme *s;
	int i;
	
	data = fopen("data", "w");
	if (!data)
	{
		print("Create error!", 1);
		mygetch();
		return;
	}
	for(i=1; i<SCHEMES; i++)
	{
		s = schemes+i;
		if (s->active)
		{	struct data *d;
			int i;

			fprintf(data, "%s\n", s->name);
			d = s->data;
			for(i=0; i<PLAYERS; i++)
			{
				fprintf(data, "%d %d %d %d %d %d %d\n",
					d->active,
					d->pos.x,
					d->pos.y,
					d->typ,
					d->symbol,
					d->sleep,
					d->fnr
				);
				d++;
			}
		}
	}
	if (fclose(data))
	{
		print("Write error!", 1);
		mygetch();
	}
}

static char *typ2string(enum typ t)
{
	switch(t)
	{
	case BLOCKER:
		return "Blocker";
	case OPPONENT:
		return "Opponent";
	}
	return "";
}

static void showdata(struct data *d, int intense)
{   struct player p;

	p.pos.x = d->pos.x + 20;
	p.pos.y = d->pos.y + FY/2;
	p.symbol = d->symbol;
	if (d->active)
		the_player(&p, SHOW);
	if (intense)
	{
		if (d->active)
		{
			sprintf(buf, "X %2d   Y %2d", d->pos.x, d->pos.y);
			print(buf, 1);
			sprintf(buf, "Typ %s", typ2string(d->typ));
			print(buf, 1);
			if (d->typ == OPPONENT)
			{
				sprintf(buf, "Sleeps %d", d->sleep);
				print(buf, 1);
				sprintf(buf, "Function %s", fnr2string(d->fnr));
				print(buf, 1);
			}
			p.symbol = INTENSE;
			the_player(&p, SHOW);
		}
		else
			print("<Inactive>", 1);
	}
}

static void movethe(struct scheme *s, struct data *d)
{	int done = 0;

	while(!done)
	{
		if (d->symbol>PL3 || d->symbol<PL2)
			d->symbol = PL2;
		if (d->symbol == PL2)
			d->typ = BLOCKER;
		if (d->symbol == PL3)
			d->typ = OPPONENT;
		if (d->typ == BLOCKER)
			d->fnr = 0;
		clear(GREEN);
		moveto(X/2, Y/2);
		{	struct data *d;
			int i;

			d = s->data;
			for(i=0; i<PLAYERS; i++)
			{
				showdata(d, 0);
				d++;
			}
		}
		showdata(d, 1);
		if (d->typ == OPPONENT)
			print("(Cursor, t, +, -, f, q-quit)", 1);
		else
			print("(Cursor, t, q-quit)", 1);
		switch(mygetch())
		{
		case -'H':
		case '8':
			d->pos.y--;
			break;
		case -'P':
		case '5':
			d->pos.y++;
			break;
		case '4':
		case -'K':
			d->pos.x--;
			break;
		case '6':
		case -'M':
			d->pos.x++;
			break;
		case 't':
			d->symbol++;
			break;
		case '+':
			d->sleep++;
			break;
		case '-':
			d->sleep--;
			break;
		case 'f':
			if (++d->fnr>MAXFUNCS)
				d->fnr = 0;
			break;
		case '\r':
		case 'q':
			done = 1;
			break;
		}
	}
}

static int edit(void)
{	int i, this, done=0;
	struct data *d;
static ed = 1;

	this = -1;
	while(!done)
	{
		clear(GREEN);
		moveto(X/32,Y/2);
		for(i=0; i<SCHEMES; i++)
		{
			if (i==ed)
				print("> ", 0);
			if (schemes[i].active)
				print(schemes[i].name, 1);
			else
				print("<Free>", 1);
			moveto(X/32, gety());
		}
		if (schemes[ed].active)
		{
			d = schemes[ed].data;
			init_data(d, 0, 0, PLAYER, PL1, 0, 0);
			for(i=0; i<PLAYERS; i++)
			{
				showdata(d, 0);
				d++;
			}
			if (this>=0)
			{
				d = schemes[ed].data+this;
				moveto(X/3, Y/2);
				sprintf(buf, "Nr. %2d  ", this);
				print(buf, 0);
				showdata(d, 1);
			}
		}
		moveto(X/2, 19*Y/20);
		if HIGHCOLOR
			setcolor(BLUE);
		print("(Cursor, D, U, d, u, n, Enter, q-quit)", 1);
		setcolor(Color);
		switch(mygetch())
		{
		case -'H':
		case '8':
			if (--ed < 0)
				ed=0;
			this = -1;
			break;
		case -'P':
		case '5':
			if (++ed >= SCHEMES)
				ed = SCHEMES-1;
			this = -1;
			break;
		case '4':
		case -'K':
			if (--this<0)
				this=0;
			break;
		case '6':
		case -'M':
			if (++this>=PLAYERS)
				this=PLAYERS-1;
			break;
		case '\r':
			if (ed && this>0)
			{
				schemes[ed].data[this].active = 1;
				movethe(schemes+ed, schemes[ed].data+this);
			}
			break;
		case 'n':
			if (ed)
			{
				print("Enter Name (max. 19 Chars)", 1);
				gotoxy(1, 24);
				gets(buf);
				strncpy(schemes[ed].name, buf, sizeof(schemes[ed].name)-1);
				schemes[ed].name[sizeof(schemes[ed].name)-1] = 0;
				schemes[ed].active = 1;
			}
			break;
		case 'd':
			if (ed && this>=0)
				schemes[ed].data[this].active = 0;
			break;
		case 'u':
			if (ed && this>=0)
				schemes[ed].data[this].active = 1;
			break;
		case 'D':
			if (ed)
				schemes[ed].active = 0;
			break;
		case 'U':
			schemes[ed].active = 1;
			break;
		case 'q':
			done = 1;
			break;
		}
	}
	return ed;
}

void menu(void)
{	int done;
	struct scheme *s;
	int n;
	
	settextjustify(LEFT_TEXT, TOP_TEXT);
	for/*ever*/(;;) 
	{
		clear(BLUE);
		moveto(X/3,Y/4);
		if (kbrepeat)
			print("k - forbid keyboardrepeat", 1);
		else
			print("k - allow keyboardrepeat", 1);
		print("h - help", 1);
		s = getscheme();
		print("n - next scheme (", 0); 
		print(s->name, 0); print(")", 1);
		moveto(X/3, gety());
		print("e - edit action scheme", 1);
		print("s - save all action schemes", 1);
		print("p - play", 1);
		print("q - quit game", 1);
		print("......................................", 1);
		print("This game is (c)1992 by Martin Lercher", 1);
		print("''''''''''''''''''''''''''''''''''''''", 1);
		do {
			done = 1;
			switch(mygetch())
			{
			case 'q':
				quit = 1;
				return;
			case 'p':
				return;
			case 'h':
				help();
				break;
			case 'e':
				n = edit();
				if (schemes[n].active)
					currentscheme = n;
				if (schemes[currentscheme].active)
					break;
				/* else fall through */
			case 'n':
				do {
					if (++currentscheme >= SCHEMES)
						currentscheme = 0;
				} while(schemes[currentscheme].active == 0);
				break;
			case 's':
				save();
				break;
			case 'k':
				kbrepeat = !kbrepeat;
				break;
			default:
				done = 0;
				break;
			}
		} while(!done);
	}
}
