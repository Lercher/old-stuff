/* @(#)TETRIS, ein geometrisches Spiel. */

char SCCS_ID[] = "@(#)"__FILE__" 1.3 MSDOS VERSION "__DATE__"  "__TIME__;

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "times.c"

#include <fcntl.h>
#include <conio.h>
#include <io.h>

#include "metris.h"

feld neu, alt;
FILE	*hsc;
long highscore;

char	rawin()
{
	if (kbhit())
		return( (char) getch());
	else
		return((char) 0);
}

void	clear_feld(a)
feld	a;
{	int i,j;

	foreach(i,j)
		a[i][j].c = 0;
}



#define	cls()		clrscr()
void goto_xy(int x, int y)   {	gotoxy((x)+1, (y)+1); }
#define eeol()		clreol()

#define textcolor(A)
#define textbackground(A)


#define BLUE 0
#define RED 0
#define CYAN 0
#define MAGENTA 0
#define YELLOW 0
#define GREEN 0
#define LIGHTBLUE 0
#define WHITE 0


/*
#undef gets
static char *gets(char *s)
{	char buf[30];

	*buf = 27;
	return strcpy(s, cgets(buf));
}
*/

/* void rev(i)
{
static last=0;
	if (last != i)
	{
		if (i)
		{
			textbackground(7);
			textcolor(0);
		}
		else
		{
			textbackground(0);
			textcolor(7);
		}
	last = i;
	}
} */



void pfat(int era, int vg, int hg, int x, int y, char *s, ...)
{	va_list pp;
	char out[200];

	textcolor(vg);
	textbackground(hg);
	goto_xy(x,y);
	va_start(pp, s);
	vsprintf(out, s, pp);
	printf(out);
	if (era) eeol();
	va_end(pp);
}

void	rein()
{
	if (super)
		pfat(FALSE, GREEN, BLUE, XS-10,YS-2,"TETRIS++,  ein nervenaufreibendes Spiel.");
	else
		pfat(FALSE, GREEN, BLUE, XS-10,YS-2,"          TETRIS, Das Original.         ");
}

void	raus()
{
	goto_xy(0,24);
	eeol();
	goto_xy(0,23);
	eeol();
}

void wechsel()
{
	super = 1-super;
	if (super)
		pfat(TRUE, CYAN, BLUE, ATinfo, "Die Idee stammt von Andreas Schmitt.");
	rein();
}

void get_name(ud_flg)
{	char in[100];
	struct entry *cp = high_start;
	int i,j, lastsco;

	if (ud_flg)
		pfat(TRUE, CYAN, BLUE, ATinput, "Sind Sie immer noch %s (RETURN, sonst NAME) ? ", player);
	else
		pfat(TRUE, YELLOW, BLUE, ATinput, "Ihr Name bitte: ");
	while( rawin() ) /**/;
	gets(in);
	if (*in == '\0') return;
	in[NAMELEN-1] = '\0';
	if ( 0 < (i = atoi(in)) )
	{
		show_score(i);
		return;
	}
	for(j=TRUE, i=0; in[i]; i++)
	{
		if (isspace(in[i]))
			j = TRUE;
		else
		{
			if (j)
				in[i]=toupper(in[i]);
			else
				in[i]=tolower(in[i]);
			j = FALSE;
		}
	}
 	strncpy(player, in, NAMELEN-1);
 	player[NAMELEN-1] = '\0';
	i = 0;
	while( cp )
	{
		i++;
		if ( 0 == strcmp(player, cp->name) )
		{
			pfat(TRUE, GREEN, BLUE, ATinfo, "Highscore fÅr %s: %ld Punkte, Platz %d.",
					player, cp->score, i);
			show_score(i);
			highscore = cp->score;
			return;
		}
		cp = cp->link;
	}
	lastsco = i;
	highscore = 800L;
	textbackground(BLUE);
	cls();
	pfat(TRUE, MAGENTA, BLUE, ATinput, "Ich kenne Sie noch nicht.");
	textcolor(YELLOW);
	puts("");
	printf("TETRIS ist ein geometrisches Spiel, das das rÑumliche Vorstellungs-");
	printf("vermîgen sehr gut fîrdert. FÅllen Sie die Zeilen mit den 'herunter-");
	printf("fallenden' Teilen einfach lÅckenlos aus.");
	printf("Die Bedienung beschrÑnkt sich auf wenige Tasten:");
	printf("  %-50s%s", "'7' ... Teil nach links schieben.",	"+-------+-------+-------+");
	printf("  %-50s%s", "'9' ... Teil nach rechts schieben.",	"| <---- | <---+ | ----> |");
	printf("  %-50s%s", "'8' ... Teil gegen die Uhr drehen.",	"|   7   |   8 | |   9   |");
	printf("  %-50s%s", "'5' ... Teil im Uhrzeigersinn drehen.",	"| <---- | ----+ | ----> |");
	printf("  %-50s%s", "SPC,'4' Teil fallen lassen.",		"+-------+-------+-------+");
	printf("  %-50s%s", "'6' ... Einen Level hîher.",		"| |   | | ----+ | Level |");
	printf("  %-50s%s", "ESC ... Quit.",				"| | 4 | |   5 | |   6   |");
	printf("  %-50s%s", "'p' ... Pause.",				"| V   V | <---+ | Level |");
	printf("  %-50s%s", "",					"+-------+-------+-------+");
	printf("  %-50s%s", "",					"|       |       |       |");
	puts("");
	printf("PS: Testen Sie auch TETRIS++ mit #4 im Hauptmenue.");
	printf("    Viel Spa· wÅnscht: Martin Lercher. August 1989");
	printf("\r\n\r\nDiesen Text sehen Sie (mit Ihrem Namen) nur einmal. (RETURN) ");
	fflush(stdout);
	gets(in);
	cls();
	puts("Punkte gibt es auch, und zwar:\r\n");
	puts("   1 Punkt   je Teil.");
	puts("   1 Punkt   je gefallene Zeile (das bringt viele Punkte.)");
	puts("   5 Punkte  fÅr eine aufgelîste Zeile.");
	puts("  15 Punkte  fÅr zwei aufgelîste Zeilen.");
	puts("  35 Punkte  fÅr drei aufgelîste Zeilen.");
	puts("  75 Punkte  fÅr vier aufgelîste Zeilen.\r\n");
	puts("Das Ganze wird mit der Levelnummer multipliziert, und falls TETRIS++");
	puts("gespielt wird, auch noch mit zwei.\r\n");
	puts("Beispiel: ein 'L' fÑllt 3 Zeilen auf Level 5 von TETRIS++, und");
	puts("lîst 2 Zeilen auf. Das sind (1 + 3 + 15) * 5 * 2 == 190 Punkte.\r\n");
	printf("Insgesammt gibt es %d Levels, mit den Fallzeiten:", MAXLEVEL);
	for(i=0; i<MAXLEVEL; i++)
	{
		if ((i%3) == 0) puts("");
		printf("Level%3d:%5ld ms        ", i, (1000L * levc[i]) / CLK_TCK);
	}
	printf("\r\n(RETURN) ");
	gets(in);
	cls();
	rahmen();
	show_score(lastsco < 3 ? 0 : lastsco-3);
}

int main_menu()
{	int i;
	clock_t lt;
static char line[] = "-------------------";
	for(;;)
	{
		i = 5;
		pfat(FALSE, CYAN , BLUE, 4,i++,line);
		pfat(FALSE, CYAN , BLUE, 5,i++,"1...Spielen");
		pfat(FALSE, CYAN , BLUE, 5,i++,"2...Level(%d) ", levl);
		pfat(FALSE, CYAN , BLUE, 5,i++,"3...Neuer Spieler");
		if (super)
			pfat(FALSE, CYAN , BLUE, 5,i++,"4...TETRIS  ");
		else
			pfat(FALSE, CYAN , BLUE, 5,i++,"4...TETRIS++");
		pfat(FALSE, CYAN , BLUE, 5,++i,"9...ENDE");
		pfat(FALSE, CYAN , BLUE, 4,++i,line);

		lt = clock();
		i = get(ATinput, "Bitte waehlen: ");
		pfat(TRUE, CYAN , BLUE, ATinfo, "");
		if (i != 3 && i != 9 && clock() > 30 * CLK_TCK + lt)
			get_name(TRUE);
		switch(i)
		{
		case 9:
			return 0;
		case 2:
			i = get(ATinput, "Neuer Level: ");
			if (i != dlevel(i-levl))
				pfat(TRUE, RED, BLUE, ATinfo, "Fehler.");
			break;
		case 3:
			get_name(FALSE);
			break;
		case 4:
			wechsel();
			break;
		default:
			return 1;
		}
	}
}

void at(x,y,c)
ttyp *c;
{
	char ch;

	textbackground(BLUE);
	goto_xy(x*2+XS, y+YS);
	if (c->c == 0)
	{
		textbackground(BLUE);
		textcolor(BLUE);
		printf("  ");
	}
	else
	{
		textbackground(c->hg);
		textcolor(c->vg);
		ch = c->c>0 ? c->c : -c->c;
		printf("%c%c", ch, ch);
	}
}

void atx(int x, int y)
{
	goto_xy(x*2+XS, y+YS);
	textbackground(BLUE);
	textcolor(WHITE);
	putch(177);
	textbackground(WHITE);
	textcolor(BLUE);
	putch(177);
}

void rahmen()
{	int i;

	rein();
	for(i=0; i<HEIGHT; i++)
		atx(-1, i);
	for(i=0; i<HEIGHT; i++)
		atx(WIDTH, i);
	for(i=-1; i<WIDTH+1; i++)
		atx(i, HEIGHT);
	pfat(FALSE, LIGHTBLUE, BLUE, ATlevel,  "Level:");	dlevel(0);
	pfat(FALSE, LIGHTBLUE, BLUE, ATlines,  "Zeilen:");	dlines(0);
	pfat(FALSE, LIGHTBLUE, BLUE, ATpoints, "Punkte:"); dpoints(0);
	for(i=0; i<PICS+super*PICSE; i++)
		pfat(FALSE, piccolor[i], picback[i], (i&1)*10+ATSTATS+(i&-2), "%c%c", picc[i], picc[i]);
}

void dstats(i)
{
	pfat(FALSE, YELLOW, BLUE, (i&1)*10+2+ATSTATS+(i&-2), "%3d", ++mystat[i]);
}

int  dlevel(dl)
{	int i;

	i = levl + dl;
	if (0<=i && i<MAXLEVEL)	levl = i;
	i = levl;
	pfat(FALSE, YELLOW, BLUE, ATLEVEL, "%2d/%ldms  ", levl, (1000L * levc[i]) / CLK_TCK);
	if (i<3)
		pfat(TRUE, RED, BLUE, ATinfo,"Das ist ein Trainigs-level!  ");
	else
		if (i>8)
			pfat(TRUE, RED, BLUE, ATinfo, "Das ist ein Profi-level!  ");
		else
			pfat(TRUE, RED, BLUE, ATinfo, "");
	return(i);
}

long dpoints(dp)
{
	points += (long) (levl*dp*(1+super));
	pfat(FALSE, YELLOW, BLUE, ATPOINTS,"%6ld", points);
	return(points);
}

int dlines(dl)
{
	lines += dl;
	pfat(FALSE, YELLOW, BLUE, ATLINES, "%3d", lines);
	if (lines-10>=10*levl)	dlevel(1);
	return(lines);
}

void display()
{	int i,j; ttyp *a,*n;

	foreach(i,j)
	{
		a = &alt[i][j];
		n = &neu[i][j];
		if ( a->c != n->c )
		{
			*a = *n;
			at(i,j,n);
		}
	}
	goto_xy(0,0);
}

void delpic()
{	int i,j;

	foreach(i,j)
		if (neu[i][j].c < 0) neu[i][j].c = 0;
}

void fix()
{	int i,j,k,delline[HEIGHT];
	ttyp *n;

	for(j=0; j<HEIGHT; j++)
		delline[j] = TRUE;
	foreach(i,j)
	{
		n = &neu[i][j];
		n->c = n->c<0 ? -n->c : n->c;
		delline[j] &= (0 != n->c);
	}
	for(j=i=0; i<HEIGHT; i++)
		if (delline[i])
		{
			dlines(1);
			j+=j+5;
		}
	dpoints(j+1);
	for(i=0; i<WIDTH; i++)
		for(k = j = HEIGHT-1; j>=0; )
		{
			if (k<0)
				neu[i][j--].c = 0;
			else
				if (delline[k])
					k--;
				else
					neu[i][j--] = neu[i][k--];
		}
}


void enter(piece, x, y, rot)
{	signed char c;

	c = set_pos(piece, x, y, rot);
	while( next(&x,&y) )
	{
		if (0<=x && x<WIDTH && 0<=y && y<HEIGHT)
		{
			neu[x][y].c = -c;
			neu[x][y].hg = picback[piece];
			neu[x][y].vg = piccolor[piece];
		}
	}
}

int fits(piece, x, y, rot)
{
	set_pos(piece, x, y, rot);
	while( next(&x,&y) )
	{
		if (x<0 || x>=WIDTH)	return(FALSE);
		if (y>=HEIGHT) return(FALSE);
		if (y>=0 && neu[x][y].c>0) return(FALSE);
	}
	return(TRUE);
}

char set_pos(piece, x, y, rot)
{
	pptr = &pics[piece][rot&(ROTS-1)][0];
	cnt = DOTS + (piece>=PICS ? DOTSE : 0) - 1;
	xofs = x;
	yofs = y;
	return( picc[piece] );
}

int next(x, y)
int *x,*y;
{
	if (cnt < 0)	return(FALSE);
	*x = xofs + pptr[cnt].dx;
	*y = yofs + pptr[cnt].dy;
	cnt--;
	return(TRUE);
}

void compu()
{
	if (on_line)
	{
		if ( fits(cp, cx, cy+1, cr) )
		{
			delpic();
			cy++;
			enter(cp, cx, cy, cr);
			return;
		}
		fix();
		on_line = FALSE;
	}
	cx = WH;
	cr = 0;
	cy = 1;
	cp = rnd[rand() % (RND + super * RNDE)];
	if ( !fits(cp, cx, cy, cr) )
		game_on = FALSE;
	else
	{
		enter(cp, cx, cy, cr);
		on_line = TRUE;
		dstats(cp);
	}
}

int get(x, y, s)
char *s;
{	char in[20];

	pfat(TRUE, YELLOW, BLUE, x,y,s);
	while( rawin() ) /**/;
	gets(in);
	return(atoi(in));
}

void user()
{	char c;
	clock_t	lastc;
	int cont = TRUE;

	if (!on_line)	return;
	lastc = clock() + levc[levl];
	while ( clock()<lastc && cont )
	{
		if (0 != (c = rawin()) )
		{	int	x = cx, y = cy, r = cr;

			switch(c)
			{
			case 71:
			case '7': --x;  break;
			case 72:
			case '8': --r;	break;
			case 73:
			case '9': ++x;	break;
			case '5': ++r;	break;
			case 77:
			case '6':
				dlevel(1);
				break;
			case 75:
			case ' ':
			case '4':
				while( fits(cp, x, y+1, r) )
				{
					dpoints(1);
					y++;
				}
				cont = FALSE;
				break;
			case 27 :
				cont = game_on = FALSE;
				break;
			case 'p':
			case 'P':
				get(ATinput, "--- (RETURN) --- ");
				pfat(TRUE, YELLOW, BLUE, ATinput, "");
				break;
/*
 *			default:
 *				pfat(TRUE, RED, BLUE, ATinput, "%d", (int) c);
 */
			}
			if ( fits(cp, x,y, r) )
			{
				cx = x; cy = y; cr = r;
				delpic();
				enter(cp, x, y, r);
				display();
			}
		}
	}
}

void tetris()
{	int i;

	game_on = TRUE;
	on_line = FALSE;
	lines = 0;
	points = 0L;
	for(i=0; i<PICS+PICSE; mystat[i++] = 0) /**/;
	textbackground(BLUE);
	textcolor(YELLOW);
	cls();
	rahmen();
	clear_feld(neu);
	srand((int)clock());
	while(game_on)
	{
		compu();
		display();
		user();
	}
}

void fer()
{
	goto_xy(ATinput);
	perror(HIGHSCORE);
	exit(1);
}

/* Falls fÅr name NULL öbergeben wird, wird player benutzt, und
 * dieser Name dann abgespeichert.
 */
int eintrag(score, name)
long score;
char *name;
{	struct entry *cp, *cpo;
	int pos = 0;

	if (score < highscore)
		return 0;
	cpo = (struct entry *) &high_start;
	cp = high_start;
	while( cp )
	{
		pos++;
		if (cp->score == score && name && 0 == strcmp(cp->name, name))
			return 0; /* Eintrag schon da */
		if (score > cp->score)
		{	struct entry *neu;

			neu = (struct entry *) calloc(1, sizeof(struct entry));
			if (NULL == neu)
			{
				pfat(TRUE, RED, GREEN, ATinfo, "Speicher voll, versuche 'TETRISP -s'.");
				return 0; /* speicher voll */
			}
			neu->link = cp;
			neu->score = score;
			cpo->link = neu;
			if (NULL == name)
			{
				highscore = score;
				pfat(TRUE, GREEN, BLUE, ATinfo, "%s erreicht Platz Nr. %d.",
						player, pos);
				name = player;
				hsc = fopen(HIGHSCORE, "a");
				if (hsc == NULL)
					fer();
				if ( EOF == fprintf(hsc, "%ld %s\n", score, name ) )
					fer();
				if( fclose(hsc) )
					fer();
			}
			strncpy(neu->name, name, NAMELEN);
			return(pos);
		}
		cpo = cp;
		cp = cp->link;
	}
	/* not reached */
	pfat(TRUE, RED, BLUE, ATinfo, "Internal error in Highscore list. Refer to Martin Lercher.");
	return(0);
}

void friss()
{	long along;
	char name[NAMELEN];

	rewind(hsc);
	while( !feof(hsc) )
	{	int i = 0;
		char c;

		if ( EOF == fscanf(hsc, "%ld", &along) )
			return;
		while(isspace( c = fgetc(hsc) ))
			if ( feof(hsc) )
			{
				pfat(TRUE, RED, BLUE, ATinput, "Invalid Hiscore file.");
				exit(3);
			}
		while(i<NAMELEN-1 && c!='\n')
		{
			name[i++] = c;
			c = fgetc(hsc);
		}
		name[i] = '\0';
		while( c != '\n' && !feof(hsc) )
			c=fgetc(hsc);
		eintrag(along, name);
	}
}


void show_score(pos)
{	struct entry *cp = high_start;
	int i,j=0;

	if (pos == 0)	pos = 4;
	i = 1;
	while( cp && i<pos+3 )
	{
		if ( (i>pos-3 || i == 1) && cp->score )
		{
			pfat(FALSE, GREEN, BLUE, AThigh+j++, "%2d.%6ld %-16s", i, cp->score, cp->name);
			if (i==1 && 4<pos)
			{
				pfat(FALSE, GREEN, BLUE, AThigh+j++, "%-27s","");
			}
		}
		cp = cp->link;
		i++;
 	}
 	while( j<8 )
		pfat(FALSE, GREEN, BLUE, AThigh+j++, "%-27s","");
}

void pflegen()
{	char cmd[100];
	int i,p;
	struct entry *cp, *cpo;
	int maxpos = 32767;
	long 	minsc = 800L;
	signed char mult='-';

	cls();
	for(;;)
	{
		pfat(TRUE, YELLOW, BLUE, 0,0,"s)how-all  #)cut-pos(%d)  $)cut-score(%ld)  "
				"d)el-single  c)ount     \r\n"
				"M)ultiname(%c)  S)ave  m)ix-load q)uit"
				, maxpos, minsc, mult);
		pfat(TRUE, WHITE, BLUE, 0,2, "Command> ");
		gets(cmd);
		cp = high_start;
		cpo = (struct entry *) &high_start;
		i = 1;
		switch(cmd[0])
		{
		case 'q':
			return;
		case 'M':
			mult = '-' + '+' - mult;
			break;
		case 'm':
			pfat(TRUE, WHITE, BLUE, 0,2, "Give filename to load: ");
			gets(cmd);
			if (*cmd == '\0')
				continue;
			hsc = fopen( cmd, "r");
			if (hsc)
			{
				friss();
				fclose(hsc);
				pfat(TRUE, WHITE, BLUE, 0,3, "Loaded.");
			}
			else
			{
				pfat(TRUE, WHITE, BLUE, 0,3, "");
				perror(cmd);
			}
			break;
		case '#':
			textcolor(WHITE);
			i = get(0,2, "Last valid position on save: ");
			if (i) maxpos = i;
			break;
		case '$':
			pfat(TRUE, WHITE, BLUE, 0,2,"Minimal score on save: ");
			gets(cmd);
			if (atol(cmd)) minsc = atol(cmd);
			break;
		case 's':
			textcolor(YELLOW);
			textbackground(BLUE);
			eeol();
			printf("\r\n");
			eeol();
			printf("\r\n");
			while (kbhit())
				rawin();
			while (cp && !rawin())
			{
				if (cp->score)
				{
					printf("%3d.%7ld %-25s  ", i++, cp->score, cp->name);
					if (1 & i)
					{
						eeol();
						printf("\r\n");
					}
				}
				cp = cp->link;
			}
			eeol();
			break;
		case 'c':
			while(cp)
			{
				cp = cp->link;
				i++;
			}
			textcolor(YELLOW);
			printf("%d Entries.", i-2);
			eeol();
			break;
		case 'd':
			if (cp == &lastman)
			{
				pfat(TRUE, RED, BLUE, 0,3, "Cannot delete 'Last man'.");
				break;
			}
			pfat(TRUE, WHITE, BLUE, 0,3, "");
			p = get(0,2,"Position to delete: ");
			if (p)
			{
				while(cp)
				{
					if ( i==p )
					{
						cpo->link = cp->link;
						free(cp);
						break;
					}
					i++;
					cpo = cp;
					cp = cp->link;
				}
				if (cp)
					pfat(TRUE, RED, BLUE, 0,3, "Deleted. List is not up to date (Type 's').");
			}
			break;
		case 'S':
			hsc = fopen(HIGHSCORE, "w");
			if (hsc == NULL) fer();
			while(cp && cp->score >= minsc && i<=maxpos)
			{
				i++;
				if (EOF == fprintf(hsc,"%ld %s\n", cp->score, cp->name))
						fer();
				do {
					cpo = cp;
					cp = cp->link;
				}	while( mult == '+' && cp && 0 == strcmp(cpo->name, cp->name) );
			}
			if ( fclose(hsc) )
					fer();
			break;
		}
	}
}

main(argc, argv)
char *argv[];
{
	textbackground(BLUE);
	textcolor(YELLOW);
	cls();
	printf("Metris Version: 1.3 mit Farbe. benutze -bw fÅr S/W.\n\r");
	high_start = &lastman;
/*	if ( access(HIGHSCORE, 0) )
	{
		hsc = fopen(HIGHSCORE, "w");
		if ( NULL == hsc )
			fer();
		if ( EOF == fprintf(hsc, "%ld %s\n", lastman.score, lastman.name) )
			fer();
		if ( fclose(hsc) )
			fer();
	}
*/	hsc = fopen(HIGHSCORE, "a+");
	if ( NULL == hsc )
		fer();
	friss();
	if ( fclose(hsc) )
		fer();
	if (argc == 2 && 0 == strcmp(argv[1], "-bw"))
	{       int i;

		for(i=0; i<PICS+PICSE; i++)
		{
			piccolor[i]=BLUE;
			picback[i]=WHITE;
		}
	}
	else
	if (argc == 2 && 0 == strcmp(argv[1], "-s"))
	{
		pflegen();
		raus();
		return 0;
	}
	else
		if (argc > 1)
		{
			printf("usage: %s [-s] [-bw]\r\n  -s process Highscore list\r\n  -bw B/W mode", argv[0]);
			exit(3);
		}
	levl = 5;
	lines = 0;
	points = 0L;
	super = FALSE;
	rahmen();
	get_name(FALSE);
	while( main_menu() )
	{
		tetris();
		show_score(eintrag(points, NULL));
	}
	raus();
	return 0;
}
