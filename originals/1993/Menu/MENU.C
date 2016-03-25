#include "version.h"
char SCCS_ID[]="@(#)"__FILE__" V"VERSION" \tCompiled "__DATE__" "__TIME__"\n";

/* eine kleine Shell fÅr MSDOS Benutzer.
 * File: menu.c
 *
 * autor: Martin Lercher, Schloesslgartenweg 40, D-8415 Nittenau
 * Tel. 09436/8475 
 * net: c3524@rrzc1.rz.uni-regensburg.de
 *
 * Dieses Programm wurde mit Turbo C++ von Borland International erzeugt,
 * sollte aber auch mit einem normalen ANSI-C  Compiler funktionieren, der
 * die Borland Spezialfunktionen versteht.
 *
 * Dies ist der vollstÑndige Quelltext der Version 1.1
 * Ich weise darauf hin, da· éanderungen zwar erwÅnscht sind, aber
 * bitte sowohl in Quelltext, als auch in der Startmeldung darauf hin-
 * gewiesen wird. Bitte beachten Sie auch das Manual menu.TeX
 *
 * Dieses Programm und alle(!) zugehîrigen Dateien sollen ruhig weit
 * verbreitet werden, dÅrfen aber nicht profitmÑ·ig verkauft und auch
 * nicht zerstÅckelt werden.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <dir.h>
#include <conio.h>
#include <process.h>
#define printf cprintf
#include "../spawno/spawno.h"

struct rechteck {
	unsigned x:8;
	unsigned w:8;
	unsigned y:5;
	unsigned h:5;
};


struct text {
	struct text *next;
	char *s;
};

struct cmd {
	struct cmd *next;
	char *s;
};


struct content {
	unsigned x:8;
	unsigned y:5;
	unsigned h:3;	/* vorder & hintergrundfarbe */
	unsigned v:4;
	unsigned r:2;	/* Rahmenart */
	char *title;
	struct text *text;
	struct cmd *cmd;
};

struct winlist {
	int redraw;
	struct rechteck size; /* actual size */
	struct content *inhalt;
};

enum mode { NORMAL, PICK };
enum      { ALTERNATIV = 1, IMMER, OPEN, CLOSE };

static struct winlist *winlist[20];
static int winlistptr=0;
static int Fpending;
static char startmenu = 'A';
char buf[201];
char *rootpath, *savefn;
char prompt[100]="Bitte wÑhlen: ";
char *hotkeys[10];
char *vars[36];
char *pick[26];
struct content *menus[26];


void clr(struct rechteck *r);
void rahmen(struct rechteck *r, char *title, unsigned mode);
void mark_all(void);
void redraw(int all);
void load_menu(const char *s, int such);
void taste(void);
void error(char *s);


void *xmalloc(size_t size)
{   void *p;

	p = malloc(size);
    if (!p)
    {
     	error("Kein Speicher mehr da!");
        exit(1);
    }
    return(p);
}
#define malloc xmalloc

char *xstrdup(char *s)
{   char *p;

	p = strdup(s);
    if (!p)
    {
     	error("Kein Speicher mehr da!");
        exit(1);
    }
    return(p);
}
#define strdup xstrdup

void save(char *s)
{	char var[] = "$A", pic[]="%A", nl[]="\n";
	FILE *f;
	int i;

	f = fopen(s, "w");
	for(i = 0; i<36; i++)
	{
    	if (i==26)	/* flag section starts */
        	var[1]='0';
		if (vars[i])
		{
			fputs(var, f);
			fputs(vars[i], f);
			fputs(nl, f);
		}
		var[1]++;
	}
	for(i = 0; i<26; i++) /* pick list */
	{
		if (pick[i])
		{
			fputs(pic, f);
			fputs(pick[i], f);
			fputs(nl, f);
		}
		pic[1]++;
	}
	fclose(f);
}

void error(char *s)
{
	gotoxy(1,24);	textcolor(RED);	textbackground(LIGHTGRAY);	clreol();
	printf(s);
	taste();	gotoxy(1,24);	textbackground(BLACK);	clreol();
}

void rmnl(char *str)
{
	if (*str)
	{	char *s;

		s = str+strlen(str)-1;
		if (*s == '\n')
			*s = 0;
	}
}

char *xgets(FILE *f)
{
	if (!fgets(buf, sizeof(buf), f))
		return NULL;
	rmnl(buf);
	return(strdup(buf));
}



int getkey(void)
{   int x, y;
	time_t tim;
	int c;

	x = wherex();
	y = wherey();
	tim = 0;
	gotoxy(55, 25);
	clreol();
	gotoxy(x, y);
	while(!kbhit())
	{
		if (time(NULL) > tim)
		{   char *s;
			tim = time(NULL);
			s = asctime(localtime(&tim));
			rmnl(s);
			gotoxy(56, 25);
			printf(s);
			gotoxy(x, y);
		}
	}
	c = getch();
	if (c == 0)
		c = -getch();
    if (c == 3)
		exit(1);
	return c;
}

void taste(void)
{	int x, y;

	x = wherex();
	y = wherey();
	textcolor(YELLOW); textbackground(BLUE);
	gotoxy(66, 24);	printf(" Taste drÅcken \b");
	gotoxy(80, 24); getkey();
	textbackground(BLACK);
	gotoxy(66, 24); clreol();
	gotoxy(x, y);
}

void clrback(int x, int y)
{
	window(x,y,wherex(),y);
	clrscr();
	window(1,1,80,25);
	gotoxy(x,y);
}

int insert(char **pickptr, char *pick, int dir)
{   /* vorne und hinten ein ¯ */
	/* ¯nr1¯nr2¯nr3¯ */

	if (dir)
	{
		if (!*pickptr)
			*pickptr = pick;
		else
			*pickptr = strchr((*pickptr)+1, '¯');
		if ((*pickptr)[1])
			goto ok;
	}
	else
	{
		if (!*pickptr)
			*pickptr = pick + strlen(pick) -1;
		if ((*pickptr)!=pick)
		{
			do {
				(*pickptr)--;
			} while(**pickptr != '¯');
			goto ok;
		}
	}
	*pickptr = NULL;
	return 0; /* abort */
ok:
	return (int)(strchr((*pickptr)+1, '¯') - *pickptr -1);
}

void normalize(char **s)
{	char *p1, *p2;
	char buf1[sizeof(buf)+2], buf2[sizeof(buf)+2];
	int l, m;

	if (!*s)
		return;
	if (**s != '¯')
	{
		*buf2 = '¯';
		strcpy(buf2+1, *s);
	}
	else
	{
		strcpy(buf2, *s);
	}
	l = strlen(buf2);
	if (buf2[l-1]!='¯')
		strcpy(buf2+l, "¯");
	free(*s);
	/* nun ist *s in buf2 mit ¯*s¯ versehen oder = ¯ */

	strcpy(buf1, "¯");

	p2 = NULL;
	m = insert(&p2, buf2, 1);
	while(p2)
	{   int schonda;

		if (m)
		{
			schonda = 0;
			p1 = NULL;
			l = insert(&p1, buf1, 1);
			while(p1)
			{
				if (l==m && 0 == strncmp(p1+1, p2+1, l))
				{
					schonda = 1;
					break;
				}
				l = insert(&p1, buf1, 1);
			}
			if (schonda == 0)
			{
				l = strlen(buf1);
				if (l+m < 76)
				{
					strncpy(buf1+l, p2+1, m);
					buf1[l+m]='¯';
					buf1[l+m+1]=0;
				}
			}
		}
		m = insert(&p2, buf2, 1);
	}

	if (strlen(buf1)==1)
	{
		*s = NULL;
		return;
	}
	*s = strdup(buf1);
}


int getvar(int varnr, char *prompt, struct rechteck *r, enum mode mode)
{   char *s=buf;
	char *Default;
	int i=0;
	int x, y;
	int c, result = 0;
	char *pickptr = NULL;

	if (mode == PICK && !pick[varnr])
	{
		pick[varnr] = malloc(3 + (vars[varnr]? (strlen(vars[varnr])+1) : 0));
		strcpy(pick[varnr], "¯");
		if (vars[varnr])
			strcpy(pick[varnr]+1, vars[varnr]);
		strcat(pick[varnr]+1, "¯");
	}
	textcolor(YELLOW);
	textbackground(BLUE);
	clr(r);
	rahmen(r, mode==PICK ? pick[varnr] : NULL, 2);
	gotoxy(r->x +2, r->y +1);
	printf("%s", prompt);
	Default = vars[varnr];

	x = wherex();
	y = wherey();

	if (mode!=PICK && Default)
	{
		textcolor(RED);
		printf("%s", Default);
		textcolor(YELLOW);

		c = getkey();
		switch(c)
		{
		case '\r':
		case '\b':
			strcpy(buf, Default);
			i = strlen(buf);
			s = buf + i;
			gotoxy(x, y);
			printf("%s", buf);
			if (c == '\r')
				goto fertig;
			break; /* char c durchreichen */
		default:
			clrback(x, y);
			break; /* char c durchreichen */
		}
	}
	else
		c = getkey();
	for(;;)
	{
		switch(c)
		{
		case '\033':
			result = 1;
			goto abort;
		case '\r':
			*s = 0;
			if (i && mode == PICK)
			{	char *p;

				p = malloc(strlen(buf)+1 + strlen(pick[varnr])+1);
				strcpy(p, buf);
				if (pick[varnr])
				{
					strcat(p, pick[varnr]);
					free(pick[varnr]);
                }
				pick[varnr] = p;
				normalize(&pick[varnr]);
			}
			goto fertig;
		case '\b':
			if (i)
			{
				i--;
				s--;
				printf("\b \b");
			}
			break;
		case -72:
		case -80:
			if (mode == PICK)
			{
				if (0 != (i = insert(&pickptr, pick[varnr], c == -72 ? 0 : 1)))
				{
					clrback(x, y);
					pickptr[i+1]=0;
					printf("%s", pickptr+1);
					strcpy(buf, pickptr+1);
					pickptr[i+1]='¯';
					s = buf + i;
				}
				else
				{
					clrback(x,y);
					s = buf;
				}
			}
			break;
		default:
			if (x+i<80 && c>0)
			{
				i++;
				*s++ = (char) c;
				printf("%c", c);
			}
		}
		c = getkey();
	}
fertig:
	vars[varnr] = strdup(buf);
	if (Default)
		free(Default);
abort:
	textbackground(BLACK);
	clrscr();
	mark_all();
	return result;
}


void clr(struct rechteck *r)
{
	window(r->x, r->y, r->x + r->w -1, r->y + r->h -1);
	clrscr();
	window(1,1,80,25);
}


int canonic(char c, int isvar)
{
	if (isvar && '0'<=c && c<='9')
    	return (int)('Z'-'A'+c-'0'+1); /* 26-36 Flags */
    c = toupper(c);
    if ('A'<=c && c<='Z')
       return (int)(c-'A'); /* 0-25 Vars */
    return 'Z'-'A'; /* 25 fuer alle anderen Zeichen */
}

char *expand(char *s)
{	char *d, *p;
	enum {NORMAL, SKIPPING, READING} state;
    int which;

	if (!s)
		return NULL;
	d = buf;
    state = NORMAL;
	while(*s)
	{
		if (state!=SKIPPING && *s == '$')
		{
			s++;
			if (*s == 0)
				break;
			if (*s == '$')
				*d++ = *s;
			else
			{	char flag;

            	flag = canonic(*s,1);
				p = vars[flag];
				if (p)
				{
                	if (flag<26) /* eine Variable */
                    {
						strcpy(d, p);
						d += strlen(p);
                    }
                    else /* ein flag/selector */
                    {
                    	s++;
                        if (*s==0 || *s != '{')
                        	error("Syntax error in flag usage");
                        which = atoi(p);
                    	if (which==0)
                        	state = READING;
                        else
                        	state = SKIPPING;
                    }
				}
			}
		}
		else
        {
        	switch(state)
            {
            case READING:
            	if (*s == '|')
                {
                	which--;
                	state = SKIPPING;
                    break;
                }
                if (*s == '}')
                {
					state = NORMAL;
                    break;
                }
                /* fall through */
            case NORMAL:
				*d++ = *s;
                break;
            case SKIPPING:
            	if (*s == '|' && --which == 0)
                	state = READING;
                if (*s == '}')
					state = NORMAL;
                break;
        	}
        }
		s++;
	}
	*d = 0;
	return buf;
}





void ausdehnung(struct text *t, struct rechteck *r)
{   int i=0, l=0;

	while(t)
	{
		l = max(l, strlen(expand(t->s)));
		t = t->next;
		i++;
	}
	l += 4;
	i += 2;
	if (r->x == 0)
		r->x = (80 - l)>>1;
	if (r->y == 0)
		r->y = (25 - i)>>1;
	r->w = l;
	r->h = i;
}


void rahmen(struct rechteck *r, char *title, unsigned mode)
{   int i;
static char rtyp[]="      ⁄ø¿Ÿƒ≥…ª»ºÕ∫";

#define P(i) putch(rtyp[i+mode]);
	if (mode>2)
    	mode = 2;
	mode *= 6;
	gotoxy(r->x,r->y);	P(0);
	for(i=2; i<r->w; i++)  P(4);
	P(1);
	if (title)
	{
		gotoxy(r->x+((r->w - strlen(title))>>1), r->y);
		printf("%s", title);
	}
	for(i=1; i<r->h-1; i++)
	{
		gotoxy(r->x, r->y+i);	P(5);
		gotoxy(r->x+r->w-1, r->y+i);	P(5);
    }
	gotoxy(r->x,r->y+r->h-1);	P(2);
	for(i=2; i<r->w; i++)  P(4);
	P(3);
#undef P
}

void mark_all(void)
{	int i;

	for(i=0; i<winlistptr; i++)
		winlist[i]->redraw = 1;
}


void redraw(int all)
{   int i,xx,yy;

	xx = wherex();
	yy = wherey();
	for(i=0; i<winlistptr; i++)
	{	struct winlist *w;
	
		w = winlist[i];
		if (all || w->redraw)
		{	struct content *inhalt;
			struct text *t;
			int y;

			w->redraw = 0;
			inhalt = w->inhalt;
			w->size.x = inhalt->x;
			w->size.y = inhalt->y;
			ausdehnung(inhalt->text, &w->size);
			textcolor(inhalt->v);
			textbackground(inhalt->h);
			clr(&w->size);
			rahmen(&w->size, expand(inhalt->title), inhalt->r);
			t = inhalt->text;
			y = w->size.y+1;
			while(t)
			{
				gotoxy(w->size.x+2, y++);
				printf("%s", expand(t->s));
				t = t->next;
			}
		}
	}
	gotoxy(xx, yy);
}

void push_w(struct content *inhalt)
{   struct winlist *w;

	if (winlistptr<20)
	{
		winlist[winlistptr++] = w = malloc(sizeof(struct winlist));
		w->redraw = 1;
		w->inhalt = inhalt;
	}
}

void pop_w(void)
{	struct winlist *w;

	if (winlistptr)
	{
		w = winlist[--winlistptr];
		textbackground(BLACK);
		clr(&w->size);
		free(w);
		mark_all();
	}
}

int menu(char c)
{	struct content *w;

	w = menus[canonic(c,0)];
	if (w)
		push_w(w);
	else
	{
		sprintf(buf, ">>> Menu %c nicht definiert. <<<", 'A'+canonic(c,0));
		error(buf);
	}
	return w!=NULL;
}

/* used by 'T[0-9]': test to a flag */
int exec(char *s)
{	char *argv[20];
	char *env;
	int argc;
	enum {TEXT, SPACE} state;

	env = getenv("TESTPRG");
	argv[0]=env ? env : "test.exe";
	argc=1;
	state = SPACE;
	for(;*s;s++)
    {
     	if (state==TEXT && isspace(*s))
		{
			state=SPACE;
            *s=0;
            continue;
        }
        if (state==SPACE && !isspace(*s))
        {
        	state=TEXT;
            argv[argc++]=s;
            continue;
        }
    }
    argv[argc]=NULL;
    argc = spawnvp(P_WAIT, argv[0], argv);
    if (argc<0)
    	error("Kann test.exe nicht finden.");
    return argc;
}

void mysystem(char *s)
{   char *env;

	if (*s)
		system(s);
	else
	{
		env=getenv("COMSPEC");
		system(env ? env : "command");
	}
}


int execute(char *s)
{   int r=0;
	int FP = 0;
	struct rechteck re = {1,80,1,3};
	char **p;
	char *q;
	char *expd;
static int vord = YELLOW, hint = BLACK, tox = 1, toy = 1;
static char numbers[6];

	if (!s)
	{
		Fpending = 0;
		return r;
	}
	if (s[1])
		expd = expand(s+1);
	else
		expd = s+1; /* a nullstring */
	switch(*s)
	{
	case 'q':
		pop_w();
        r = 1;
		tox = toy = 1;
		break;
	case 'g':
		pop_w();
		tox = toy = 1;
		Fpending = 0;
		/* fall through */
	case 'm':
		menu(*expd);
        r = 1;
        FP = Fpending;	/* Funktionstatstenstatus erhalten! */
		break;
	case 'S':
		if (*expd)
			save(expd);
		else
			save(savefn);
		break;
	case 'L':
		if (*expd)
			load_menu(expd, 0);
		else
			load_menu(savefn, 0);
		mark_all();
		break;
    case 'D': /* load file on demand */
    	if (*expd)
        	load_menu(expd, 0);
        mark_all();
        s[1] = 0;	/* load this file only once */
        break;
    case 's': /* search replace string */
    	{	FILE *file;
        	char *komma;
            char variable = canonic(*expd,0);
            char tmpbuf[sizeof(buf)];

            strcpy(tmpbuf, expd+1);
			komma = strchr(tmpbuf, ',');
            if (komma)
            {
            	char *line, *found;
            	int leave=25;

            	*komma++=0;
	    	    file = fopen(tmpbuf, "r");
                if (!file)
					break;
                while(leave-- && NULL != (line=xgets(file)))
                {
                    found = strstr(line, komma);
                    if (found)
                    {
                    	found += strlen(komma);
						p = &vars[variable];
						q = *p;
						*p = strdup(found);
						if (q)
							free(q);
						mark_all();
                        leave = 0;
                    }
                	free(line);
                }
                fclose(file);
            }
            else
            	error("',<text>' in Befehl 's' erwartet.");
    	}
    	break;
    case '>':
    	textcolor(vord);
    	textbackground(hint);
    	gotoxy(tox, toy);
		mysystem(expd);
	tox = wherex();
	toy = wherey();
		mark_all();
	break;
	case 'e':
	case 'E':
		textbackground(BLACK);
		textcolor(WHITE);
		clrscr();
		mysystem(expd);
		tox = wherex();
		toy = wherey();
		if (*s != 'e')
			taste();
		mark_all();
		break;
	case '?':
		r = getvar(canonic(*expd,0), expd+1, &re, NORMAL);
		mark_all();
		break;
	case 'p':
		r = getvar(canonic(*expd,0), expd+1, &re, PICK);
		mark_all();
		break;
	case '!':
		textcolor(vord);
		textbackground(hint);
		gotoxy(tox, toy++);
		printf("%s", expd);
		mark_all();
		break;
	case 'c':
	case '*':
		textbackground(hint);
		clrscr();
		mark_all();
		toy = tox = 1;
		break;
	case 'R':
		textbackground(BLACK);
		clrscr();
		/* fall through */
	case 'r':
		redraw(1);
		toy = tox = 1;
		vord = YELLOW;
		hint = BLACK;
		break;
	case 'x':
		tox = atoi(expd);
		break;
	case 'y':
		toy = atoi(expd);
		break;
	case 't':
		taste();
		break;
	case 'v':
		vord = atoi(expd);
		break;
	case 'h':
		hint = atoi(expd);
		break;
	case '%':
		if (*expd == 0)
			break;
		p = &pick[canonic(*expd,0)];
		q = *p;
		*p = strdup(expd+1);
		if (q)
			free(q);
		normalize(p);
		break;
    case '+': /* incr flag */
    case '-': /* decr flag */
    case 'i': /* invert flag */
    	{ char flag;
		if (*expd == 0)
        	break;
        flag=canonic(*expd,1);
        if (flag<26)
        {
			error("Can only modify flags, not vars");
			break;
		}
		q = vars[flag];
		if (q)
		{	int result, maximum=0, wrap=0;

			if (*s != 'i')
			{
				if (expd[1])
					maximum = atoi(expd+1);
				maximum = maximum ? maximum : 20;
				if (maximum<0)
				{
					maximum = -maximum;
					wrap = 1;
				}
			}
			result = atoi(q);
            switch(*s)
            {
            case '+':
            	result++;
				if (result>maximum)
					result=wrap ? 0 : maximum;
				break;
			case '-':
				result--;
				if (result<0)
					result=wrap ? maximum : 0;
				break;
            case 'i':
            	result = !result;
                break;
            }
        	vars[flag]=strdup(itoa(result, numbers, 10));
            free(q);
        }
        else
        	vars[flag]=strdup("0");
        }
        mark_all();
    	break;
    case 'T': /* test a flag */
    	{ char flag;
		if (*expd == 0)
        	break;
        flag=canonic(*expd,1);
        if (flag<26)
        {
        	error("Can only test to a flag, not to a var");
            break;
        }
		p = &vars[flag];
		q = *p;
		*p = strdup(itoa(exec(expd+1), numbers, 10));
		if (q)
			free(q);
        }
        mark_all();
    	break;
	case '$':
	case '=':
		if (*expd == 0)
			break;
		p = &vars[canonic(*expd,1)];
		q = *p;
		*p = strdup(expd+1);
		if (q)
			free(q);
		mark_all();
		break;
	case 'Q':
		textbackground(BLACK);
		textcolor(WHITE);
		clrscr();
		if (*expd)
			exit(atoi(expd));
		else
			exit(0);
	default:
		sprintf(buf, "Unbekannter Befehl %c(%s)", *s, s+1);
		error(buf);
		break;
	}
	Fpending = FP;	/* immer 0 ausser 'm' */
	return r;
}

char convert(char c)
{
	switch(c)
	{
		case 'a':
			return ALTERNATIV;
		case 'i':
			return IMMER;
        case '(':
        	return OPEN;
        case ')':
        	return CLOSE;
		case 'e':
			return '\033';
		case 'n':
			return '\n';
		case 'b':
			return '\b';
		case 't':
			return '\t';
		case 'r':
			return '\r';
	}
    return c;
}


void dispatch(void)
{	int c, passt_schon;
	struct winlist *w;
	struct cmd *cmd;
	int openstate, openexec;

	while(winlistptr)
	{
		redraw(0);
		textcolor(YELLOW); textbackground(BLUE); gotoxy(1, 25); clreol();
		printf("%s", expand(prompt));
		w = winlist[winlistptr-1];	/* top window */
		c = getkey();
		if (c < 0)
		{
			c = -c;
			if (59 <= c && c <=68)	/* F-Taste */
			{
				if (!Fpending)
				{
					Fpending = 1;
					execute(hotkeys[c - 59]);
				}
				continue;
			}
		}
		/* keine F-Taste */
		passt_schon = 0;
        openstate = 0;
        openexec = 0;
		for (cmd = w->inhalt->cmd; cmd; cmd = cmd->next)
		{	char cc;
		
			cc = *(cmd->s);  /* cc=Taste zum Befehl */
            if ( cc==OPEN )
			{   char *s;

            	openstate = 1;        /* OPEN found */

                /* test all the chars after OPEN */
                for(s=expand(cmd->s+1); *s; s++)
                {
					if (*s == '\\')	/* is it a char escape? */
    	            {
        	        	s++;
            	        if (!*s)
                	    	break/*for*/; 	/* end of line reached */
                    	*s = convert(*s);
               	 	}
					if ( *s==IMMER ||
					    (*s==ALTERNATIV && passt_schon==0) ||
			    		 *s==(char)c)
					{
						if (*s == (char) c)
							passt_schon = 1;
						openexec = 1;     /* richtige Taste gedrÅckt */
                    	break/*for*/;	  /* end of Scan */
					}
                }
                continue;  /* kein Befehl folgt */
            }
            if ( cc==CLOSE )
            {
				openexec = openstate = 0;   /* CLOSE found */
                continue; /* kein befehl folgt */
            }
            if ( openexec )
			{
            	if (execute(cmd->s))
                	break/*for*/;   /* ESC pressed */
                /* continue durch nÑchste Zeile */
            }
            if ( openstate )
            	continue;

        	/* normale Befehle */
			if ( cc==IMMER ||
			    (cc==ALTERNATIV && passt_schon==0) || 
			     cc==(char)c
			   )
			{
				if (cc == (char) c)
					passt_schon = 1;
				if (execute(cmd->s+1))
					break/*for*/;	/* ESC pressed */
				/* alle ausfÅhren */
			}
		}
	}
}


unsigned xint(FILE *f, int *line)
{   int r = 0;
	unsigned char c;

	while(!feof(f) && isdigit(c=fgetc(f)))
		r = 10*r + c - '0';
	if (c == '\n')
		(*line)++;
	return r;
}


void load_menu(const char *s, int such)
{   FILE *f;
	int line = 1;

	textcolor(RED);
	f = fopen(s, "r");
	if (such == 1)
	{	char dummy[MAXPATH];

		fnsplit(s, dummy, dummy, buf, dummy);
		fnmerge(buf, "", "", buf, ".sav");
		savefn = strdup(buf);
		if (f == NULL)
		{
			strcat(rootpath, s);
			f = fopen(rootpath, "r");
			if (f == NULL)
			{
				printf("Menue-Datei %s nicht vorhanden.", s);
				exit(1);
			}
		}
		free(rootpath);
	}
	else /* such != 1 */
	{
		if (f == NULL)
			return;
	}
	while(!feof(f))
	{   signed char c;
		char **p;
		int men;
		struct content **q;
		struct text **txt;
		struct cmd **cmd;

        c = fgetc(f);
        switch(c)
		{
		case '!':
			fgets(buf, sizeof(buf), f);
			rmnl(buf);
			textcolor(LIGHTGRAY);
			printf("%s\n\r", buf);
			textcolor(RED);
			break;
		case ':':
			fgets(prompt, sizeof(prompt), f);
			rmnl(prompt);
			break;
		case 'g':
			fgets(buf, sizeof(buf), f);
			startmenu = 'A'+canonic(*buf,0);
			break;
		case '$':
			c = fgetc(f);
			p = &vars[canonic(c,1)];
			if (*p)
				free(*p);
			*p = xgets(f);
			break;
		case '%':
			c = fgetc(f);
			p = &pick[canonic(c,0)];
			if (*p)
				free(*p);
			*p = xgets(f);
			normalize(p);
			break;
		case '#':
			fgets(buf, sizeof(buf), f);	/* Kommentar */
		case -1:
		case 'Z'-'@': /*^Z*/
		case '\n':
			break;
		case '0':	/* Funktionstasten */
			c += 10;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			p = &hotkeys[c - '1'];
			if (*p)
				free(*p);
			*p = xgets(f);
			break;
		case 'm':
		case 'M':
			c = fgetc(f);
			men = canonic(c,0);
			q = &menus[men];
			if (*q)
			{
				printf("Menue %c in Zeile %d schon definiert. Wird Åbersprungen.\r\n", men+'A', line);
				do {
					fgets(buf, sizeof(buf), f); line++;
				} while(!feof(f) && (*buf != '}' || buf[1]!='\n'));
				line--;
				break;
			}
			fgets(buf, sizeof(buf), f); line++;
			/* Rest der Zeile verwerfen */
			*q = malloc(sizeof(struct content));
			(*q)->x = 0;	/* zentrieren */
			(*q)->y = 0;	/*     "      */
			(*q)->r = 2;	/* Doppelrahmen */
			(*q)->v = YELLOW;
			(*q)->h = BLUE;
			(*q)->title = NULL;
			while(!feof(f))
			{
				c = fgetc(f);
				switch(tolower(c))
				{
				case 'x':
					(*q)->x = xint(f, &line);
					break;
				case 'y':
					(*q)->y = xint(f, &line);
					break;
				case 'v':
					(*q)->v = xint(f, &line);
					break;
				case 'h':
					(*q)->h = xint(f, &line);
					break;
				case 'r':
					(*q)->r = xint(f, &line);
					break;
				case 't':
					(*q)->title = xgets(f);
					line++;
					break;
				case '#':
					fgets(buf, sizeof(buf), f);
					line++;
					break;
				case '{':
					fgets(buf, sizeof(buf), f);
					/* NL verwerfen */
					line++;
					goto out;
				default:
					fgets(buf, sizeof(buf), f);
					rmnl(buf);
					printf("Unbekannter Parameter %c(%s) in Menu %c in Zeile %d.\r\n", c, buf, men+'A', line);
					line++;
					break;
				}
			}
out:
			txt = &(*q)->text;
			while(!feof(f))
			{   char *s;

				s = xgets(f); line++;
				if (*s == '|' && s[1] == 0)
				{
					free(s);
					break;
				}
				*txt = (struct text *)malloc(sizeof(struct text));
				(*txt)->s = s;
				(*txt)->next = NULL;
				txt = &(*txt)->next;
			}

			cmd = &(*q)->cmd;
			while(!feof(f))
			{   char *s;

				s = xgets(f); line++;
				if (*s == '}' && s[1] == 0)
				{
					free(s);
					break;
				}
				if (*s == '\\')
				{
					*s = convert(s[1]);
					if (s[1])
						strcpy(s+1, s+2);
				}
				*cmd = (struct cmd *)malloc(sizeof(struct cmd));
				(*cmd)->s = s;
				(*cmd)->next = NULL;
				cmd = &(*cmd)->next;
			}
            line--; /* einmal zuviel */
			break;
		default:
			fgets(buf, sizeof(buf), f);
			rmnl(buf);
			printf("Unbekanntes Kommando %c(%s) in Zeile %d.\r\n", c, buf, line);
		}
		line++;
	} /* of while */
    fclose(f);
	textcolor(LIGHTGRAY);
}

int main(int argc, char **argv)
{	char drv[3];
    time_t tim;

    tim = time(NULL);
	rootpath = malloc(MAXPATH+2); /* free in rootpath */
	fnsplit(argv[0], drv, rootpath+2, buf, buf);
	strncpy(rootpath, drv, 2);
	*buf = 0;
    if (argc>1 && strcmp(argv[1],"-k") == 0) /* Option -k */
    {
    	if (!kbhit())
        	return 0; /* Menu nur starten, wenn eine Taste gedrÅckt wurde. */
        argc--; /* -k entfernen */
        argv++;
    }
	textcolor(LIGHTGRAY);
	textbackground(BLACK);
	clrscr();
	printf("Mini-Menue V"VERSION" vom "__DATE__". Copyright by Dipl. Math. Martin Lercher.\n\r");
	if (argc>1)
		load_menu(argv[1], 1);
	else
		load_menu("menu", 1);
	load_menu(savefn, 0);
	Fpending = 0;
	if (menu(startmenu))
		dispatch();
	textcolor(LIGHTGRAY);
	textbackground(BLUE);
	gotoxy(1,25);
    if (time(NULL)-tim<90)
    {
		printf("\nBitte lesen Sie die Shareware Bedingungen in Nr. 5 der Dokumentation menu.TeX.");
        clreol();
    }
	return 0;
}
