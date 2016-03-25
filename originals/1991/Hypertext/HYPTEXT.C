/* ht.c - Das soll ein minimales residentes Hypertextsystem werden.
 *
 * 2.8.91 Martin Lercher, Schlî·lgartenweg 40, 93149 Nittenau, Tel. 09436/8475
 * 7.5.95 Martin Lercher, Westendstr. 232, 80686 Muenchen, Tel. 089/578880
 *
 */

#define VERS "1.8"

#ifdef __TOS__
# define TOS 1
# define TSR 0
#else
# define TOS 0
# ifdef ASTSR
#  define TSR 1
# else
#  define TSR 0
# endif
# if TSR
#  ifndef __SMALL__
#   error "Small model!"
#  endif
# else
#  ifndef __COMPACT__
#   error "Compact model!"
#  endif
# endif
#endif


#if TSR
# define VERSION "TSR "VERS" (small)"
#else
# define VERSION "V "VERS" (compact)"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <dir.h>
#include <ctype.h>
#if TOS
# include <ext.h>
# define textcolor(A)
# define textbackground(A)
# define puts(s) fputs(s, stdout)
#else
# include <dos.h>
# include <sys/stat.h>
# define puts cputs
# define highvideo()
# define lowvideo()
# define Grau BLACK
# define Schwarz LIGHTGRAY
#endif

#if TSR
# define NDEBUG
# include <alloc.h>
# include <bios.h>
# include "tess.h"
static char id_string[]="HyprText";	/* TesSeRakt Identifikation */
#else
static char stuff[80]="";
#endif
static unsigned id_num=0x6c6d;
#include <assert.h>

#define LABELMARK "@@@"
#if TOS
# define LINKMARK 127
#else
# define STUFFMARK 240
# define LINKMARK 254
#endif
#if TSR
# define MAXINDEX 2000		/* may be 2000, maximum */
#else
# define MAXINDEX 7200		/* may be 7500, maximum */
#endif
#define MAXPP 100			/* maximum number of links per page */
#define MAXSPP 16			/* maximum page backtrace */
#define MAXPAGEARRAY 50			/* maximum minipages */

static char *HTFILE, *hyperfile;
static char *HTINDEX, *hyperindex;
static unsigned long search_index;
static verbose=0;

#if !TOS
static char *screen, *screen2;
static struct text_info screen_mode;
#endif

struct index {
	unsigned long index;
	char *label;
	/* Die Seite mit dem Namen LABEL steht an Position
	 * INDEX in der Datei. Das ist eine Liste.
	 */
} index[MAXINDEX];
static int max_page; /* maximale Seite+1 */
static int page; /* aktuelle(angezeigte) Seite */

static struct pusharray {
	int at_x,at_y,to_page;
	/* eine LINKMARKe an Bildschirmpasition(AT_X, AT_Y)
   * zeigt auf Seite TO_PAGE. Das ist ein Stack.
   */
} pusharray[MAXPP];
static int max_link_on_page;

static struct s_pusharray {
	int current_link, on_page;
	/* Hier wird die Seitenhistorie auf einem Ringpuffer-Stack gespeichert.
	 * CURRENT_LINK hÑlt die Linkposition der Seite ON_PAGE
	 */
} s_pusharray[MAXSPP];
static int spp_start, spp_end;

static unsigned long pagearray[MAXPAGEARRAY];
static pagearray_index;
#define pagerm() pagearray_index = 0;

static FILE *F, *IF; /* Das Datenfile, Das Indexfile */

static char buf[512];	/* a scratch buffer */

void putmark(int to_page);

void nl(void)
{
#if TOS
	puts("\n");
#else
	puts("\r\n");
#endif
}

void warnung(const char *s)
{
	puts(s);
	nl();
}

void xfree(void *p)
{
	assert(p);
	free(p);
}

void error(const char *s)
{
	warnung(s);
	puts("Programm wird abgebrochen.");
	nl();
	exit(1);
}

void *xmalloc(size_t size)
{	char *d;

	d = malloc(size);
	if (!d) error("Out of general memory.");
	return d;
}

char *xstrdup(const char *s)
{   char *d;

	d = strdup(s);
	if (!d) error("Out of string memory.");
	return(d);
}

#if TSR
/* simuliere getch() durch einen BIOSaufruf. */
int xgetch(void)
{
static unsigned c=1;
	if (0==(c&0xff))
		return (c++>>8)&0xff;
	c = bioskey(0);
	return c&0xff;
}
#else
# define xgetch getch


unsigned TsStuffKeyboard(int id, char *s, int len, int mode)
{  int l;
	l = mode;
	l = id;

	l = strlen(stuff);
	if (l+len>78)
		return 1;
	strncat(stuff, s, len);
	return 0;
}


void Unstuff(void)
{  char *s=stuff;

	textbackground(BLACK);
	if (*s)
		puts(">");
	while(*s)
	{
		putch(*s);
		if (*s == '\r')
		{
			nl();
			puts(">");
		}
		s++;
	}
}
#endif

#if TOS
# define saveall()	0
# define restoreall()
#else
/* Sichere den Bildschirm und dessen Status */
int saveall(void)
{
	gettextinfo(&screen_mode);
	movedata(0xb800,0,FP_SEG(screen),FP_OFF(screen),4000);
	gettext(1,1,80,25,screen);
	return 0;
}

#define S(A) screen_mode.A
void restoreall(void)
{
	movedata(FP_SEG(screen),FP_OFF(screen),0xb800,0,4000);
	gotoxy(S(curx),S(cury));
	textattr(S(attribute));
}
#endif

/* remove the trailing \n character */
char *rmnl(char *s)
{	int l;

	assert(s!=NULL);
	l = (int)strlen(s);
	if (l && s[l-1]=='\n')
		s[l-1] = 0;
	return s;
}

struct wort {
	struct wort *link;
	char *data;
	int size;
	int ref;
	int *refs;
	int lastref;
};

struct wort *wort_anfang = NULL, *wort_ende = (struct wort *) &wort_anfang;


struct wort *such_wort(const char *s)
{   struct wort *w;

	w = wort_anfang;
	while(w)
	{
		if (0 == strcmp(s, w->data))
			return w;
		w = w->link;
	}
	return NULL;
}

void add_ref(struct wort *w, int pg)
{
	assert(w);
	if (pg == w->lastref)
		return;
	if (!w->size)
	{
		gotoxy(15, wherey());
		puts("New word ");
		puts(w->data);
		clreol();
		w->size = 8;
		w->refs = malloc(sizeof(int) * w->size);
		if (!w->refs)
			error("Speichermangel (new word).");
	}
	if (w->ref == w->size)
	{
		gotoxy(15, wherey());
		puts("Frequent ");
		puts(w->data);
		clreol();
		w->size *= 2;
		assert(w->size < 1025);
		w->refs = realloc(w->refs, w->size * sizeof(int));
		if (!w->refs)
			error("Speichermangel (realloc).");
	}
	w->lastref = w->refs[w->ref] = pg;
	w->ref++;
}

void new_ref(const char *s, int pg)
{	struct wort *w;

	w = (struct wort *)malloc(sizeof(struct wort));
	if (!w)
		error("Speichermangel (new word).");
	wort_ende->link = w;
	wort_ende = w;
	w->link = NULL;
	w->data = xstrdup(s);
	w->lastref = -1;
	w->size = 0;
	w->ref = 0;
	w->refs = NULL;
	add_ref(w, pg);
}

void add_a_word(const char *s, int pg)
{	struct wort *w;

	w = such_wort(s);
	if (w)
		add_ref(w, pg);
	else
		new_ref(s, pg);
}

typedef void (do_words_f)(const char *s, int pg);

int ispartofword(char c)
{
	if (c == '¯' || c == '~' || c == '^')
		return 0;
	return isalnum(c);
}
	
void do_words(const char *buf, int pg, do_words_f *dof)
{   char s[100], *ss;

	while(*buf)
	{
		ss = s;
		while(*buf && !ispartofword(*buf))
			buf++;
		while(ispartofword(*buf))
		{
			*ss++ = tolower(*buf++);
			assert(ss<s+99);
		}
		*ss='\0';
		if (ss-s > 1)
			(*dof)(s, pg);
	}
}

void write_words(FILE *f)
{	struct wort *w, *ww;
	int i;

	for(w = wort_anfang; w; w = ww)
	{
		fprintf(f, "%s %d", w->data, w->ref);
		for(i=0; i < w->ref; i++)
			fprintf(f, " %d", w->refs[i]);
		if (EOF == fprintf(f, "\n"))
		{
			perror(HTINDEX);
			error("");
		}
		ww = w -> link;
		xfree(w->data);
		xfree(w->refs);
		xfree(w);
	}
}

int report1st;

int *report_array = NULL, rep_arr_len;

void intmalloc(void)
{   int i, n;

	fscanf(IF, "%d", &n);
	if (report_array)
		xfree(report_array);
	report_array = calloc(n, sizeof(int));
	for(i=0; i<n; i++)
		fscanf(IF, "%d", report_array+i);
	rep_arr_len = n;
}

void intreduce(void)
{  	int n, i, try, tryi;

	fscanf(IF, "%d", &n);
	tryi = 0;
	for(i=0; i<n; i++)
	{
		fscanf(IF, "%d", &try);
		while(tryi < rep_arr_len)
		{
			if (try < report_array[tryi])
				break;
			if (try == report_array[tryi])
			{
				tryi++;
				break;
			}
			report_array[tryi] = -1;
			tryi++;
		}
	}
	for(; tryi<rep_arr_len; tryi++)
		report_array[tryi] = -1;
}

void mk_report(const char *s, int dummy)
{	static char buf1[512];
	unsigned long pos;

	*buf1 = (char) dummy;
	dummy = strlen(s);

	if (fseek(IF, search_index, SEEK_SET))
	{
		perror(HTINDEX);
		error("");
	}
	while(!feof(IF))
	{
		pos = ftell(IF);
		fgets(buf1, sizeof(buf1), IF);
		if (0 == strncmp(buf1, s, dummy) && buf1[dummy] == ' ')
		{
			fseek(IF, pos+dummy+1, SEEK_SET);
			if (report1st || NULL == report_array)
				intmalloc();
			else
				intreduce();
			report1st = 0;
			break;
		}
	}
}

int i_suche(const char *s, int mode)
{	int i, j;

	report1st = mode;
	do_words(s, 0, mk_report);
	if (!report_array)
		return -1;
	j = 0;
	for(i=0; i<rep_arr_len; i++)
		if (report_array[i] >= 0)
			report_array[j++] = report_array[i];
	rep_arr_len = j;
	if (rep_arr_len)
	{
		report_array = realloc(report_array, rep_arr_len*sizeof(int));
		if (!report_array)
			error("Speichermangel (shrink).");
		return 0;
	}
	xfree(report_array);
	report_array = NULL;
	return -1;
}


/* Lese die Indexdatei, erzeuge Sie wenn nîtig. */
void read_index(void)
{	FILE *f;
	time_t t;
	struct stat statbuf;
	char buf1[20];
	int thepage;

	stat(HTINDEX, &statbuf); /* fehler ignorieren. */
	t = statbuf.st_ctime;
	if (stat(HTFILE, &statbuf))
		error("Hypertext nicht gefunden.");
	t -= statbuf.st_ctime;

	f = fopen(HTINDEX, "r");
	if (f==NULL || t<0)
	{	FILE *src;

generate:
		if (f!=NULL) /* Index-Datei ex., aber Zeiten passen nicht */
			fclose(f);
		puts("Bitte warten, Indexdatei wird erzeugt.");
		nl();
		puts("Pass 1: Seitenseparation.");
		nl();
		src = fopen(HTFILE, "r");
		f = fopen(HTINDEX, "w"); /* Index schreiben */
		if (f == NULL)
			error("Fehler beim Erstellen der Indexdatei.");
		fputs("HyperText "VERSION": Index-Datei zum Hypertext in ", f);
		fputs(HTFILE, f);
		fputs("\nFormat: Zahl ist Datei-Offset der Seite mit dem nachfolgenden Namen.\n", f);
		max_page = 0; /* zÑhle die Seiten */
		while(max_page<MAXINDEX)
		{
			if (NULL == fgets(buf, (int)sizeof(buf), src))
				break; /* at EOF */
			if (0 == strncmp(buf, LABELMARK, sizeof(LABELMARK)-1))
			{	/* Das ist eine Seitenmarkierung */
				rmnl(buf);
				index[max_page].index = ftell(src); /* Position nach dem Label */
				ltoa(index[max_page].index, buf1, 10);
				fputs(buf1, f);
				fputs("\n", f);
				index[max_page].label = xstrdup(buf+sizeof(LABELMARK)-1);
				fputs(index[max_page].label, f); /* Seitenname */
				fputs("\n", f);
				max_page++;
			}
		}
		fputs("0\n", f);
		if (!max_page)
			error("Keine Seite Hypertext definiert.");
		fseek(src, index[0].index, SEEK_SET);
		puts("Pass 2: Suchindex erstellen.");
		nl();
		thepage=0;
		for(;;)
		{
			if (NULL == fgets(buf, (int)sizeof(buf), src))
				break; /* at EOF */
			if (0 == strncmp(buf, LABELMARK, sizeof(LABELMARK)-1))
			{
				puts("\r");
				thepage++;
				puts(itoa(thepage+1, buf, 10));
				puts("/");
				puts(itoa(max_page, buf, 10));
				continue;
			}
			do_words(buf, thepage, add_a_word);
		}
		puts("\rPass 3: Suchindex schreiben.");
		clreol();
		write_words(f);
		fclose(src);
	}
	else
	{
		fgets(buf, (int)sizeof(buf), f); /* Kommentar Åberlesen. */
		if (strncmp(buf, "HyperText "VERSION, 9+sizeof(VERSION)))
			goto generate;
		fgets(buf, (int)sizeof(buf), f);
		for(max_page = 0; max_page<MAXINDEX; max_page++)
		{   unsigned long l;

			/* Jede Doppelzeile lesen. */
			if (NULL == fgets(buf, (int)sizeof(buf), f))
				break;
			l = atol(rmnl(buf));
			if (!l)
				break;
			index[max_page].index = l;
			if (NULL == fgets(buf, (int)sizeof(buf), f))
				break;
			index[max_page].label = xstrdup(rmnl(buf));
		}
		search_index = ftell(f);
	}
	fclose(f);
	if (max_page == 0)
		error("Keine Seite Hypertext definiert.");

}

void help(void)
{	int x, y;

	x = wherex();
	y = wherey();
	movedata(0xb800,0,FP_SEG(screen2),FP_OFF(screen2),4000);
	clrscr();
	nl();
	puts("  F1        - help"); nl();
	puts("  F2, i     - index"); nl();
	puts("  Csr       - move to other link"); nl();
	puts("  Tab       - next link"); nl();
	puts("  Bkspc     - previous link"); nl();
	puts("  Ret       - follow link"); nl();
	puts("  PgDn, Spc - next minipage"); nl();
	puts("  PgUp      - previous minipage"); nl();
	puts("  n, f      - next page"); nl();
	puts("  p, b      - previous page"); nl();
	puts("  h         - home"); nl();
	puts("  g         - goto named link"); nl();
	puts("  S         - search index (Words are combined by AND)"); nl();
	puts("  a         - add new words to search index"); nl();
	puts("  +, -      - next and previous match."); nl();
	puts("  s         - show all matches."); nl();
	puts("  ESC, u    - up link"); nl();
	puts("  q         - quit"); nl();
	nl();
	puts("  <Any key to return>");
	if (0 == getch())
		getch();
	movedata(FP_SEG(screen2),FP_OFF(screen2),0xb800,0,4000);
	gotoxy(x,y);
}

/* gib index der Seite mit Namen S. */
int suche(const char *s)
{	int i;

	assert(s!=NULL);
	for(i=page; i<max_page; i++)
		if (0 == strcmp(s, index[i].label))
			return i;
	for(i=0; i<page; i++)
		if (0 == strcmp(s, index[i].label))
			return i;
	return -1;
}

void pagepush(unsigned long p)
{
	assert(pagearray_index<MAXPAGEARRAY);
	pagearray[pagearray_index++] = p;
}

unsigned long pagepop(void)
{
	if (pagearray_index)
		return pagearray[--pagearray_index];
	return *pagearray;
}

/* speichere die xy Position der LINKMARKS */ 
#define pushrm()	max_link_on_page=0
void push(int x, int y, int pg)
{
	if (max_link_on_page<MAXPP)
	{
		pusharray[max_link_on_page].at_x=x;
		pusharray[max_link_on_page].at_y=y;
		pusharray[max_link_on_page].to_page=pg;
		max_link_on_page++;
	}
	else
		assert(0); /* Zuviele Referenzen. */
}


/* Den Index generieren */
int mk_index(int i)
{	int x, y, j;
	char *s;

	s = index[i].label;
	j = i % (4*24);
	x = (j / 24) * 20 + 1;
	y = j % 24 + 1;
	gotoxy(x,y);
	textcolor(LIGHTRED);
	putmark(i);
	textcolor(Grau);
	push(x, y, i);
	for(j=0; *s && j<18; j++, s++)
		putch(*s);
	return (i+1) % (4*24) == 0;
}


/* Den Such-Index generieren */
void mk_s_index(int ind)
{	int x, y, j, i;
	char *s;

	i = report_array[ind];
	if (i<0)
		return;
	s = index[i].label;
	j = ind % (4*24);
	x = (j / 24) * 20 + 1;
	y = j % 24 + 1;
	gotoxy(x,y);
	textcolor(LIGHTRED);
	putmark(i);
	textcolor(Grau);
	push(x, y, i);
	for(j=0; *s && j<18; j++, s++)
		putch(*s);
}


/* Die entsprechende Markierung zur Seite #I ausgeben. */
void putmark(int to_page)
{
	assert(to_page<max_page);
	if (to_page<0)
		putch('?');
	else
	if (*(index[to_page].label) == '>')
		putch(STUFFMARK);
	else
		putch(LINKMARK);
}


/* Seitentext analysieren und ausgeben. */
int analyze(char *s)
{	char *ss;
	int x, mode, i, putok, max_x;

	x = 1;
	max_x = 0;
	putok = 1;
	mode = Grau;
	textcolor(mode);

	while(*s)
	{
		switch(*s)
		{
		case '\t':
			x = ((x+7) & 0xf8)+1;
			gotoxy(x, wherey());
			break;
		case '^':	/* Link */
			x++;
			ss = ++s;
			if (*s == '^')
			{
				if (putok)
					putch(*s);
				break;
			}
			while(*s && *s != '^')
			{
				x++;
				s++;
			}
			if (0 == *s)
				s[1]=0; /* FÅr die Schleife */
			*s=0;
			push(wherex(), wherey(), i=suche(ss));
			textcolor(LIGHTRED);
			putmark(i);
			textcolor(mode);
			break;
		case '¯': /* blauer Text */
			if (s[1] == '¯')
			{
				x++;
				s++;
				if (putok)
					putch(*s);
				break;
			}
			mode = (mode!=Grau)?Grau:LIGHTBLUE;
			textcolor(mode);
#if TOS
		   if (mode!=Grau)
				highvideo();
			else
				lowvideo();
#endif
			break;
		case '~': /* grÅner Text */
			if (s[1] == '~')
			{
				x++;
				s++;
				if (putok)
					putch(*s);
				break;
			}
			mode = (mode!=Grau)?Grau:LIGHTGREEN;
			textcolor(mode);
#if TOS
			if (mode!=Grau)
				highvideo();
			else
				lowvideo();
#endif
			break;
		default:
			if (putok)
				putch(*s);
			break;
		}
		x++;
		s++;
		max_x = max(max_x, wherex());
		if (wherex()==1 && max_x > 1)
		{
			gotoxy(1, wherey()-1);
			putok = 0;
		}
	}
	textcolor(Grau);
	return wherey()==25;
}


/* Seitenhistorie in Ringpuffer-Stack verwalten */
void s_push(int current_link, int idxp)
{
	s_pusharray[spp_start].current_link = current_link;
	s_pusharray[spp_start].on_page = idxp;
	spp_start = (spp_start+1)&(MAXSPP-1);
	if (spp_start == spp_end)
		spp_end = (spp_end+1)&(MAXSPP-1);
}

int s_pop(int *current_link, int *idxp)
{
	if (spp_start!=spp_end)
	{
		spp_start = (spp_start-1) & (MAXSPP-1);
		*current_link = s_pusharray[spp_start].current_link;
		*idxp= s_pusharray[spp_start].on_page;
		return 1;
	}
	return 0;
}

/* sicheres puts, schreibt nicht Åber den rechten Rand. */
void saveputs(const char *s)
{	int x;
	char text[80];

	x = wherex();
	if (strlen(s) < 81-x)
		puts(s);
	else
	{
		strncpy(text, s, 80-x);
		text[80-x] = '\0';
		puts(text);
	}
}

/* Statuszeile ausgeben */
void info(int pg)
{
	gotoxy(1, 25);
	highvideo();
	textbackground(BLUE);
	textcolor(YELLOW);
	if (page>=0)
	{
	   saveputs(" ");
		saveputs(index[page].label);
	}
	else
		saveputs(" <ESC> <F1>");
	if (pg<0)
	{
		if (max_link_on_page)
		{
			saveputs(" -- ");
			textcolor(WHITE);
			saveputs("(unbekannter Sprung)");
		}
	}
	else
	{
		saveputs(" -- ");
		if (*(index[pg].label)=='>')
		{
			textcolor(YELLOW);
			saveputs("Text von Seite ");
			textcolor(WHITE);
			saveputs(index[pg].label+1);
			textcolor(YELLOW);
			saveputs(" wird abgetippt.");
		}
		else
		{
			saveputs("> ");
			saveputs(index[pg].label);
		}
	}
	clreol();
	gotoxy(79,25);
	saveputs("q");
	textbackground(Schwarz);
	textcolor(Grau);
	lowvideo();
}


/* Funktionen zur Cursorbewegung. */
#define MAXUN 0xffffu
enum mode {DN, UP, RI, LE};

unsigned int dist(int a, int b, int c,int d, enum mode mode)
{	int fx=0, fy=2;
	switch (mode)
	{
	case DN:
		if (b>=d)
			return MAXUN;
		fx+=2;
		break;
	case UP:
		if (b<=d)
			return MAXUN;
		fx+=2;
		break;
	case RI:
		if (a>=c)
			return MAXUN;
		fy+=2;
		break;
	case LE:
		if (a<=c)
			return MAXUN;
		fy+=2;
		break;
	}
	return (((unsigned)((a-c)*(a-c)))<<fx)
		 + (((unsigned)((b-d)*(b-d)))<<fy);
}


/* minimalen Abstand eines Linkmarks von CURRENT_LINK
 * in Richtung MODE bestimmen.
 */
int minimal(enum mode mode, int current_link)
{	unsigned x=wherex(), y=wherey();
	unsigned min=MAXUN, d;
	int minindex=-1, i;

	for(i=0; i<max_link_on_page; i++)
	{
	   d = dist(x,y,pusharray[i].at_x, pusharray[i].at_y, mode);
		if (d<min)
		{
		  minindex = i;
			min = d;
		}
	}
	if (minindex==-1)
		return current_link;
	return minindex;
}


enum show_mode { NEW, FWD, BACK };
/* Die aktuelle Seite anzeigen */
void showpage(enum show_mode mode)
{	int i;
	unsigned long pos;
	static end_found;

	textbackground(Schwarz);

	if (NEW == mode)
	{
		pos = (0<=page && page<max_page) ? index[page].index : 0l;
		pagerm();
		pagepush(pos);
	}
	else if (FWD == mode)
	{
		if (end_found)
			return;
		pagepush(pos = pagepop());
	}
	else if (BACK == mode)
	{
		if (pagearray_index < 3)
			return;
		pagepop();
		pagepop();
		pagepush(pos = pagepop());
	}
	fseek(F, pos, SEEK_SET);
	end_found=0;
	pushrm();
	clrscr();
	for(i=0; i<24; i++)
	{
		if (NULL == fgets(buf, (int)sizeof(buf), F))
		{
			end_found = 1;
			break;
		}
		rmnl(buf);
		if (0 == strncmp(buf, LABELMARK, sizeof(LABELMARK)-1))
		{
			end_found = 1;
			break; /* nix mehr anzeigen */
		}
		analyze(buf);
		nl();
	}
	pagepush(ftell(F));
}


/* Hier ist das Hauptprogramm. */
void far pascal TsrMain(void)
{
static int current_link = 0, current_search = -1;
	int cont, c, i, this_is_index, pg, old_link=-1, indexmode = 0, old_index;
	enum show_mode mode;
	int search_mode;

	if (saveall())
		return;
	mode = NEW;
	old_index = 0;
	this_is_index = 0;
von_vorn:
	c = -2;
	/* Text anzeigen. */
	showpage(mode);
	mode = NEW;
	/* user interaction */
	cont = 1;
	while (cont)
	{
		switch(c)
		{
		case 's': /* Show last search index */
			if (current_search < 0)
				break;
			indexmode = 0;
			clrscr();
			pushrm();
			for(i=0; i<rep_arr_len && i<4*24; i++)
				mk_s_index(i);
			current_link=0;
			page = -2;
			c = -2;
			continue;
		case 'a': /* and index */
			search_mode = 0;
			goto such;
		case 'S': /* search index */
			search_mode = 1;
		such:
			gotoxy(1,25);
			textcolor(YELLOW);
			textbackground(BLUE);
			puts("Keyword(s): ");
			clreol();
			fgets(buf, sizeof(buf), stdin);
			rmnl(buf);
			pg = -1;
			current_search = i_suche(buf, search_mode);
			if (current_search >= 0)
				pg = report_array[current_search];
			if (pg>=0)
			{
				if (!indexmode)
					s_push(current_link, page);
				page = pg;
				current_link = 0;
				indexmode = 0;
			}
			goto von_vorn;
		case '+':
			if (current_search < 0)
				break;
			current_search++;
			if (current_search >= rep_arr_len)
					current_search = 0;
			if (!indexmode)
				s_push(current_link, page);
			page = report_array[current_search];
			current_link = 0;
			indexmode = 0;
			goto von_vorn;
		case '-':
			if (current_search < 0)
				break;
			current_search--;
			if (current_search < 0)
				current_search = rep_arr_len-1;
			if (!indexmode)
				s_push(current_link, page);
			page = report_array[current_search];
			current_link = 0;
			indexmode = 0;
			goto von_vorn;
		case -'I': /* pg up */
			if (indexmode && this_is_index >= 2*4*24)
			{
				c = 'i';
				this_is_index -= 2*4*24;
				continue;
			}
			mode = BACK;
			current_link = 0;
			goto von_vorn;
		case -'Q': /* pg dn */
		case ' ':
			if (indexmode)
			{
				c = 'i';
				continue;
			}
			mode = FWD;
			current_link = 0;
			goto von_vorn;
		case 3:
		case 'q':
		case 'Q':
			cont = 0;
			continue;
		case 'h':
			if (!indexmode)
				s_push(current_link, page);
			page=0;
			pushrm();
			current_link=0;
			goto von_vorn;
		case 'p':
		case 'b':
			if (indexmode && this_is_index >= 2*4*24)
			{
				c = 'i';
				this_is_index -= 2*4*24;
				continue;
			}
			for(pg=page-1; pg>=0; pg--)
			{	char ch;

				ch = *index[pg].label;
				if (ch != '.' && ch != '>')
				{
					page = pg;
					current_link=0;
					goto von_vorn;
				}
			}
			break;
		case 'n':
		case 'f':
			if (indexmode)
			{
				c = 'i';
				continue;
			}
			for(pg=page+1; pg<max_page; pg++)
			{	char ch;

				ch = *index[pg].label;
				if (ch != '.' && ch != '>')
				{
					page = pg;
					current_link=0;
					goto von_vorn;
				}
			}
			break;
		case -1:
			if (max_link_on_page)
			{
				textcolor(LIGHTRED);
				putmark(pusharray[old_link].to_page);
				textcolor(Grau);
			}
			/* fall through */
		case '.':
		case -2:
			if (0<=current_link && current_link<max_link_on_page)
			{
				info(pusharray[current_link].to_page);
				gotoxy(pusharray[current_link].at_x, pusharray[current_link].at_y);
				textcolor(LIGHTMAGENTA);
				putmark(pusharray[current_link].to_page);
				textcolor(Grau);
				gotoxy(pusharray[current_link].at_x, pusharray[current_link].at_y);
			}
			else
			{
				info(-1);
				current_link = -1;
				gotoxy(1,1);
			}
			break;
		case -'P':	/* csr dn */
			old_link = current_link;
			current_link = minimal(DN, current_link);
			c = -1;
			continue;
		case '\t':
			old_link = current_link;
			if (current_link<max_link_on_page-1)
				current_link++;
			c = -1;
			continue;
		case -'M':	/* csr right */
			old_link = current_link;
			current_link = minimal(RI, current_link);
			c = -1;
			continue;
		case -'H':	/* csr up */
			old_link = current_link;
			current_link = minimal(UP, current_link);
			c = -1; continue;
		case '\b':
			old_link = current_link;
			if (0<current_link)
				current_link--;
			c = -1;
			continue;
		case -'K':	/* csr left */
			old_link = current_link;
			current_link = minimal(LE, current_link);
			c = -1;
			continue;
		case 'I':
		case 'i':
		case -'<': /* F2 */
			if (!indexmode)
			{
				this_is_index = old_index;
				if (page >= 0)
					s_push(current_link, page);
			}
			indexmode = 1;
			if (this_is_index >= max_page)
				break;
			pushrm();
			current_link=0;
			clrscr();
			old_index = this_is_index;
			for(i=this_is_index; i<max_page; i++)
				if (mk_index(i))
					break;
			current_link=0;
			this_is_index=((i+4*24)/(4*24))*(4*24);
			page = -2;
			c = -2;
			continue;
		case 'g':
			gotoxy(1,25);
			textcolor(YELLOW);
			textbackground(BLUE);
			puts("Goto: ");
			clreol();
			fgets(buf, sizeof(buf), stdin);
			rmnl(buf);
			pg = suche(buf);
			if (pg>=0)
			{
				if (!indexmode)
					s_push(current_link, page);
				page = pg;
				current_link = 0;
				indexmode = 0;
			}
			goto von_vorn;
		case '\r':
			if (current_link>=0)
			{
				pg = pusharray[current_link].to_page;
				if (pg>=0 && *(index[pg].label) == '>')
				{  int length=0;

					fseek(F, index[pg].index, SEEK_SET);
					while(1)
					{
						if (NULL == fgets(buf, (int)sizeof(buf), F))
							break;
						if (0 == strncmp(buf, LABELMARK, sizeof(LABELMARK)-1))
							break; /* nix mehr */
						rmnl(buf);
						if (length /* as flag */)
							/* ein \r nach jeder Zeile ausser der letzten */
							if(TsStuffKeyboard(id_num, "\r", 1, 0xff01))
								break;
						length=strlen(buf);
						if (length)
							if(TsStuffKeyboard(id_num, buf, length, 0xff01))
								break;
						length = 1;
					}
					cont = 0;
					continue;
				}
				if (!indexmode)
					s_push(current_link, page);
				page = pg;
				current_link=0;
				indexmode = 0;
				goto von_vorn;
			}
			break;
		case 'u':
		case 27:
			indexmode = 0;
			if (s_pop(&current_link, &page))
				goto von_vorn;
			break;
		}
help_loop:
		if (0 ==(c = xgetch()))
			c = -xgetch();
		if (c == -';' /* F1 */)
		{
			help();
			goto help_loop;
		}
	}
	pushrm();
	restoreall();
}


#if TSR
void far pascal TsrCleanUp(unsigned Shutdown)
{
	extern void _restorezero(void);

	if(Shutdown)	  /* if we're shutting down 		 */
	{
	   fclose(F);
	   fclose(IF);
	/*
	 * Please note that it is *absolutely* vital to call _restorezero()
	 *	 at this point -- otherwise, the INT 0 vector
	 *	 is not restored, and a divide-by-zero exception will cause
	 *	 a crash, rather than a clean exit.  Note that this routine
	 *	 is compiler-dependent .... CR
	 */
	   _restorezero();
	}
}

/* Wie gross bin ich? */
unsigned SizeOfCode(void)
{
	extern unsigned _psp;			/* segment address of PSP		   */
	unsigned used;						/* variable to save paragraphs	   */
	struct SREGS sregs; 			/* segment register structure	   */

	segread(&sregs);				/* read the segment regs		   */
	used = (((((unsigned)sbrk(0))+16)>>4) + sregs.ds) - _psp;
	return(used);						/* return number of paragraphs	   */
}
#endif



/* TSR Installation */
#pragma warn -use
void hyper(int pg)
{	unsigned len;

	F = fopen(HTFILE, "r");
	IF = fopen(HTINDEX, "r");
	page = pg;
#if TSR
	if (TsCheckResident(id_string, &id_num) == 0xffff)
	{
		if(id_num & 0xff00) /* if released */
		{
			puts("Wir warten darauf, entfernt zu werden.");
			nl();
			puts("Hyper ist jetzt wieder aktiv. Strg+Alt+H zum Start.");
			nl();
			TsRestart(id_num & 0x00ff);
		}
		else
		{
			puts("Hyper wird entfernt.");
			nl();
			TsRelease(id_num);
		}
		exit(1);
	}
	/* Zeit zum Installieren. */
	len = SizeOfCode();
	if (verbose)
	{
		puts(itoa(len, buf, 10));
		puts(" Paragraphen = ");
		puts(ltoa(((long)len)<<4, buf, 10));
		puts(" Bytes sind resident.");
		nl();
	}
	puts("Benutze Strg+Alt+H um Hypertext zu starten.");
	nl();
	TsDoInit(TSRHOT_H, TSRPOPCTRL+TSRPOPALT, TSRUSEPOPUP+NOPOPGRAPH, len);
#else
	TsrMain();
	Unstuff();
#endif
}
#pragma warn +use

#if TSR
static Popupstack[1024]; /* 2k for popupstack */
static Backstack[8];	 /* 16 bytes for backgroundstack */
# define popupstack ((void far *)(Popupstack+1024))
# define backstack	((void far *)(Backstack+8))
#endif


void usage(const char *name)
{
	puts("Micro-Hypertext-Reader "VERSION" von Dipl. Math. Martin Lercher.");
	nl();
#if TOS
  puts("Mit Borland Turbo C V2.0 compiliert ("__DATE__" "__TIME__")");
#else
  puts("Mit Borland Turbo C V2.0 compiliert ("__DATE__" "__TIME__")");
#endif
	nl();
	nl();
	puts("usage: ");
	puts(name);
	puts(" [-i|-v|-f file] [keyword]"); nl();
	puts("  -i Just create index file"); nl();
	puts("  -v Verbose"); nl();
	puts("  -f Use file as hypertext. Default: hyper"); nl();
	puts("  keyword: Name of first Page to appear."); nl();
}

/* Suche deinen Hypertext ... */
int main(int argc, char **argv)
{	char *drv, *path, *file, *name;
	int pg, indexonly = 0;
	FILE *f;
#if TOS
	argv[0] = "c:\\tc20\\myfiles\\hyper.ttp";
#endif
#if TSR
	TsSetStack(popupstack, backstack);
#endif
	screen = xmalloc(4000);
	screen2 = xmalloc(4000);
	name = argv[0];
	drv=xmalloc(MAXDRIVE);
	path=xmalloc(MAXDIR);
	file=xmalloc(MAXFILE);
	while(argc>1)
	{
		if (0 == strcmp(argv[1], "-?") || 0 == strcmp(argv[1], "/?"))
		{
			usage(name);
			return 1;
		}
		if (0 == strcmp(argv[1], "-v"))
		{
			argv++;
			argc--;
			verbose = 1;
			continue;
		}
		if (0 == strcmp(argv[1], "-i"))
		{
			argv++;
			argc--;
			indexonly = 1;
			continue;
		}
		if (argc>2 && 0 == strcmp(argv[1], "-f"))
		{
			hyperfile = xstrdup(argv[2]);
			argv += 2;
			argc -= 2;
			continue;
		}
		break;
	}
	if (!hyperfile)
		hyperfile = xstrdup("hyper");

	fnsplit(hyperfile, drv, path, file, buf);
	fnmerge(buf, drv, path, file, ".idx");
	hyperindex = xstrdup(buf);

	xfree(drv);
	xfree(path);
	xfree(file);
	f = fopen(hyperfile, "r");
	if (f)
	{
		HTFILE = hyperfile;
		HTINDEX = hyperindex;
		fclose(f);
	}
	else
	{
		puts("Hypertext '");
		puts(hyperfile);
		puts("' nicht gefunden. Programm wird abgebrochen.");
		nl();
		return 2;
	}

	page = 0;
	max_link_on_page = 0;
	max_page = 0;
	spp_start = spp_end = 0;
	if (verbose)
	{
		puts("Micro-Hypertext-Reader "VERSION" von Dipl. Math. Martin Lercher.");
		nl();
		puts("Hypertext ist ");
		puts(HTFILE);
		puts(", Indexdatei ist ");
		puts(HTINDEX);
		puts(".");
		nl();
	}
	read_index();
	pg = 0;
	if (argc > 1)
		pg = suche(argv[1]);
	if (pg < 0)
		pg = 0;
	if (!indexonly)
		hyper(pg);
	return 0;
}
