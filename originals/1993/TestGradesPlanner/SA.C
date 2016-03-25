// sa.c - Schulaufgabenplanprogramm
// Martin Lercher, Schloesslgartenweg 40, 8415 Nittenau
// Maerz 1993

// Konfigurationsabschnitt
#define DRUCKEN 1
#define WEITEAKTIV 1
// Ende Konfigurationsabschnitt

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <time.h>

#define printf cprintf

#define MAXROWS 80
#define MAXCOLS 15
#define NAMELEN 41
#define COLLEN 9
#define HALBE 171
#if DRUCKEN
# define Stdprn stdprn
#else
# define Stdprn stdout
#endif

struct row {
	char name[NAMELEN];
	int proz[MAXCOLS];
	int total;
};

struct column {
	char name[COLLEN];
	int maxpunkte;
};

struct column colums[MAXCOLS];
struct row rows[MAXROWS];
char title[120], namefile[80], filename[80], *error;

enum { HIN, RUECK };
enum mode { NAMES, PROZ, MAX, NOTEN, AUFGABEN, PUNKTE, NOTENPROZENT };

#define isNOTE() (edit==NOTEN||edit==NOTENPROZENT)

enum mode edit;
int curcol, currow, curnot, rowofs;
int gesamtpunkte, Schnitt;
int note[7], np[7], anznote[7];
int Maxweite;

#if WEITEAKTIV
int Druckerweite=120;
#endif

int convert, *number, maxsize;
char *string;
typedef void cvtfkt(int *i, char *s, enum cvt mode);
cvtfkt *convertf;

int dirty;

void rmnl(char *str)
{
	if (*str)
	{	char *s;

		s = str+strlen(str)-1;
		if (*s == '\n')
			*s = 0;
	}
}

void anpassen(void)
{	int i;

	if (edit==NOTEN)
	{
		if (gesamtpunkte)
			for(i=1; i<6; i++)
				np[i] = note[i]*50 / gesamtpunkte;
	}
	else
	{
		note[0] = 2*gesamtpunkte+1;
		for(i=1; i<6; i++)
			note[i] = (np[i]*gesamtpunkte+25)/50;
	}
}

void cvtid(int *i, char *s, enum cvt mode)
{
	switch(mode)
	{
	case HIN:
		itoa(*i, s, 10);
		break;
	case RUECK:
		*i = atoi(s);
		break;
	}
}

void cvtproz(int *i, char *s, enum cvt mode)
{	char *halb;
	int pkt;

	switch(mode)
	{
	case HIN:
		pkt = (colums[curcol].maxpunkte * *i + 25) / 50;
		sprintf(s, "%d%c", pkt/2, pkt%2 ? HALBE : ' ');
		break;
	case RUECK:
		if (colums[curcol].maxpunkte <= 0)
		{
			*i = 100;
			break;
		}
		halb = strchr(s, HALBE);
		pkt = 2*atoi(s) + (halb?1:0);
		*i = 50*pkt / colums[curcol].maxpunkte;
		break;
	}
}

void cvthalb(int *i, char *s, enum cvt mode)
{	char *halb;
	int pkt;

	switch(mode)
	{
	case HIN:
		pkt = *i;
		sprintf(s, "%d%c", pkt/2, pkt%2 ? HALBE : ' ');
		break;
	case RUECK:
		halb = strchr(s, HALBE);
		*i = 2*atoi(s) + (halb?1:0);
		break;
	}
}


void clear(void)
{	int row, col;

	edit = PUNKTE;
	curcol = 0;
	currow = 0;
	curnot = 1;
	rowofs = 0;

	for(col=0; col<MAXCOLS; col++)
	{
		*colums[col].name = 'A';
		itoa(col+1, colums[col].name+1, 10);
		colums[col].maxpunkte = 0;
	}
	for(row = 0; row<MAXROWS; row++)
	{
		*rows[row].name = 0;
		for(col=0; col<MAXCOLS; col++)
			rows[row].proz[col] = 0;
	}
	for(row=0; row<6; row++)
		note[row] = 0;
	np[0] = 100;
	np[1] = 85;
	np[2] = 70;
	np[3] = 55;
	np[4] = 40;
	np[5] = 20;
	np[6] = 0;
	note[6] = 0;
}

int shownote(int pkte, int disp, int toprn)
{   int n;

	if (!gesamtpunkte)
		return 0;
	for(n=1; n<7; n++)
	{
		if (pkte>=note[n])
			break;
	}
	if (disp)
	{
		if (n<6 && pkte+1>=note[n-1])
			printf("+");
		printf("%d", n);
		if (n<6 && pkte==note[n])
			printf("-");
	}
	if (toprn)
	{
		if (n<6 && pkte+1>=note[n-1])
			fprintf(Stdprn, "+%d", n);
		else
		if (n<6 && pkte==note[n])
			fprintf(Stdprn, "%d-", n);
		else
			fprintf(Stdprn, "%d ", n);
	}
	return n;
}

void puthalb(int pkt)
{
	printf("%2d%c", pkt/2, pkt%2 ? HALBE : ' ');
}

#define XOFS 20
#define YOFS 5

void display(void)
{	int col, row, total, schnitt, div;

	Maxweite = 0;
	gesamtpunkte = 0;
	for(col=0; col<7; col++)
		anznote[col]=0;
	for(col=0; col<MAXCOLS; col++)
	{
		if (colums[col].maxpunkte <=0)
			continue;
		gotoxy(XOFS+col*4, YOFS-2);
		printf("%2d", colums[col].maxpunkte);
		gotoxy(XOFS+col*4-2, YOFS-1);
		printf("%4s", colums[col].name);
		gesamtpunkte += colums[col].maxpunkte;
	}
	gotoxy(XOFS-4, YOFS-2);
	printf("%2d", gesamtpunkte);
	anpassen();

	gotoxy(81-strlen(title), 2);
	printf("%s", title);

	schnitt = 0;
	div = 0;
	for(row=0; row<MAXROWS; row++)
	{	int i, disp, hnote;

		if ( *rows[row].name == 0 )
			continue;
		Maxweite = max(Maxweite, strlen(rows[row].name));
		disp = !(row<rowofs || row>rowofs + 25-YOFS-1);
		if (disp)
		{
			gotoxy(1, YOFS+row-rowofs);
			for(i=0; i<11; i++)
			{ 	int ch;
				ch = rows[row].name[i];
				if (!ch)
					break;
				printf("%c", ch);
			}
		}
		total = 0;
		for(col=0; col<MAXCOLS; col++)
		{   int proz, pkt;

			if (colums[col].maxpunkte <= 0)
				continue;
			proz = rows[row].proz[col];
			pkt = (colums[col].maxpunkte * proz + 25) / 50;
			if (disp)
			{
				gotoxy(XOFS+col*4, YOFS+row-rowofs);
				puthalb(pkt);
			}
			total += pkt;
		}
		if (disp)
		{
			gotoxy(XOFS-4, YOFS+row-rowofs);
			puthalb(total);
			gotoxy(XOFS-7, YOFS+row-rowofs);
		}
		rows[row].total = total;
		schnitt += (hnote=shownote(total, disp, 0));
		anznote[hnote]++;
		div++;
	}

	if (div && gesamtpunkte)
	{   int vk, nk;

		schnitt *= 100;
		schnitt += div/2;
		schnitt /= div;
		vk = schnitt/100;
		nk = schnitt%100;
		Schnitt = schnitt;
		gotoxy(1, YOFS-2);
		printf("Schnitt: %d,%02d", vk, nk);
	}
	if (gesamtpunkte)
	{
		for(col=0; col<6; col++)
		{
			gotoxy(1+12*col, 25);
			puthalb(note[col]-1);
			printf("<%2d>", anznote[col+1]);
			puthalb(note[col+1]);
		}
	}
	gotoxy(77,25);
	printf("F8!");
}

#define halb(i)	(i)/2, (i)%2?HALBE:' '

void drucken(void)
{	int i, anzcol, rand;

	clrscr();
	display();
	clrscr();
	printf("Drucken.\n\r");
	fprintf(Stdprn, "%s\n\r", title);
	for(i=0; i<strlen(title); i++)
		fprintf(Stdprn, "-");
	fprintf(Stdprn, "\r\n\nNotendurchschnitt: %d,%02d\n\r", Schnitt/100, Schnitt%100);
	fprintf(Stdprn, "Gesamtpunkte:      %d\n\r", gesamtpunkte);
	fprintf(Stdprn, "Notenschlssel:\n\r    ");
	for(i=0; i<6; i++)
	{
		fprintf(Stdprn, "%2d x Note %d:%3d%c-%2d%c    ", anznote[i+1], i+1,
			halb(note[i]-1), halb(note[i+1]));
		if (i==2)
			fprintf(Stdprn, "\n\r    ");
	}

	anzcol=0;
	for(i=0; i<MAXCOLS; i++)
	{
		if (colums[i].maxpunkte>0)
			anzcol += 4;
	}
#if WEITEAKTIV
	rand = min(8, max(0, Druckerweite-(anzcol+Maxweite+11)));
	if (anzcol+Maxweite+10>Druckerweite)
	{
		rand = 8;
		fprintf(Stdprn, "\r\n\n%*s  ", rand, "");
		for(i=0; i<MAXCOLS; i++)
		{
			if (colums[i].maxpunkte>0)
				fprintf(Stdprn, "%4s", colums[i].name);
		}
		fprintf(Stdprn, "\r\n%*s  ", rand, "");
		for(i=0; i<MAXCOLS; i++)
		{
			if (colums[i].maxpunkte>0)
				fprintf(Stdprn, "%4d", colums[i].maxpunkte);
		}
		fprintf(Stdprn, "\r\n\n");
		for(i=0; i<MAXROWS; i++)
		{   int col;

			if (*(rows[i].name) == 0)
				continue;
			fprintf(Stdprn, "%*s%-*s  Punkte:%3d%c   Note: ",
				rand, "", Maxweite, rows[i].name, halb(rows[i].total));
			shownote(rows[i].total, 0, 1);
			fprintf(Stdprn, "\n\r%*s   ", rand, "");
			for(col=0; col<MAXCOLS; col++)
			{	int proz, pkt;

				if (colums[col].maxpunkte<=0)
					continue;
				proz = rows[i].proz[col];
				pkt = (colums[col].maxpunkte * proz + 25) / 50;
				fprintf(Stdprn, "%3d%c", halb(pkt));
			}
			fprintf(Stdprn, "\n\r");
		}
	}
	else    // einzeilig
#else
	rand = 8;
#endif
	{
		fprintf(Stdprn, "\r\n\n%*s%-*s%s", rand, "", Maxweite+1, "Name", " N  Sum ");
		for(i=0; i<MAXCOLS; i++)
		{
			if (colums[i].maxpunkte>0)
				fprintf(Stdprn, "%4s", colums[i].name);
		}
		fprintf(Stdprn, "\r\n%*s", rand+Maxweite+2+7, "");
		for(i=0; i<MAXCOLS; i++)
		{
			if (colums[i].maxpunkte>0)
				fprintf(Stdprn, "%4d", colums[i].maxpunkte);
		}
		fprintf(Stdprn, "\r\n\n");
		for(i=0; i<MAXROWS; i++)
		{   int col;

			if (*(rows[i].name) == 0)
				continue;
			fprintf(Stdprn, "%*s%-*s ", rand, "", Maxweite+1, rows[i].name);
			shownote(rows[i].total, 0, 1);
			fprintf(Stdprn, " %3d%c ", halb(rows[i].total));
			for(col=0; col<MAXCOLS; col++)
			{	int proz, pkt;

				if (colums[col].maxpunkte<=0)
					continue;
				proz = rows[i].proz[col];
				pkt = (colums[col].maxpunkte * proz + 25) / 50;
				fprintf(Stdprn, "%3d%c", halb(pkt));
			}
			fprintf(Stdprn, "\n\r");
		}

	}
#if DRUCKEN
	fprintf(Stdprn, "%c", 'L'-'@'); // Formfeed
#else
	getch();
#endif
}


static char buf[NAMELEN];

void showint(int *what, cvtfkt *cvt)
{
	convertf = cvt;
	convert = 1;
	number = what;
	string = buf;
	maxsize = 6;
	(*cvt)(what, buf, HIN);
	printf("%-40s", string);
}

void show(char *what, int siz)
{
	maxsize = siz;
	convert = 0;
	string = what;
	printf("%-40s", string);
}

void cursor(void)
{
	if (isNOTE())
		gotoxy(-2+12*curnot, 25);
	else
		gotoxy(XOFS+curcol*4+2, YOFS+currow-rowofs);
}


void showcell(void)
{
	gotoxy(1,1);
	switch(edit)
	{
		case NOTENPROZENT:
			showint(&np[curnot], cvtid);
			break;
		case NOTEN:
			showint(&note[curnot], cvthalb);
			break;
		case PUNKTE:
			showint(&rows[currow].proz[curcol], cvtproz);
			break;
		case PROZ:
			showint(&rows[currow].proz[curcol], cvtid);
			break;
		case NAMES:
			show(rows[currow].name, NAMELEN);
			break;
		case AUFGABEN:
			show(colums[curcol].name, COLLEN);
			break;
		case MAX:
			showint(&colums[curcol].maxpunkte, cvtid);
			break;
	}

	gotoxy(69, 1);
	switch(edit)
	{
	case NAMES:
		printf("Namen   (F1)");
		break;
	case PROZ:
		printf("Prozent (F2)");
		break;
	case MAX:
		printf("MaxPkte (F3)");
		break;
	case NOTEN:
		printf("Noten   (F4)");
		break;
	case AUFGABEN:
		printf("Aufgaben(F5)");
		break;
	case PUNKTE:
		printf("Punkte  (F6)");
		break;
	case NOTENPROZENT:
		printf("NoteProz(F7)");
		break;
	}
	cursor();

}

void pending(void)
{   clock_t t;

	cursor();
	if (kbhit())
		return;

	showcell();
	t = clock() + 20;
	while(clock()<t)
		if (kbhit())
			return;

	clrscr();
	display();
	showcell();
	while(!kbhit())
		/**/;
}

int editcell(int ch)
{   int l;
	clock_t t;

	gotoxy(1,1);
	if (convert)
		*string=0;
	printf("%-40s", string);

	l = (int) strlen(string);
	for(;;)
	{
		redo:
		gotoxy(1+l, 1);

		if (ch == 27)
			return 0;
		if (ch <0 || ch == '\r')
		{
			if (convert)
				(*convertf)(number, string, RUECK);
			return ch;
		}
		if (convert && (ch == ',' || ch == '.' || ch == 'h'))
			ch = HALBE;
		if (ch == '\b' && l>0)
		{
			string[--l]=0;
			gotoxy(l+1, 1);
			printf(" ");
			gotoxy(l+1, 1);
		}
		if (ch && ch != '\b' && l<maxsize-1)
		{
			string[l++] = ch;
			string[l] = 0;
			printf("%c", ch);
		}
		t = clock()+30;
		while(t>clock())
		{
			if (kbhit())
			{
				ch = getch();
				if (ch == 0)
					ch = -getch();
				goto redo;
			}
		}
		ch = -67; // display cmd F9
	}
}

void loadnames(const char *fn)
{	FILE *f;

	f = fopen(fn, "r");
	if (f)
	{	int i;
		for(i=0; i<MAXROWS; i++)
		{
			fgets(rows[i].name, NAMELEN, f);
			rmnl(rows[i].name);
		}
		fclose(f);
		error = "Namen geladen.";
		dirty = 0;
	}
	else
		error = "Kann Datei nicht finden.";
}

void saveall(const char *fn)
{   FILE *f;

	if (*namefile == 0)
	{
		error = "Keine Namensdatei angegeben.";
		return;
	}
	f = fopen(fn, "w");
	if (f)
	{   int i, j;

		fprintf(f, "%s\n", title);
		fprintf(f, "%s\n", namefile);
		fprintf(f, "Notenprozent: %d %d %d %d %d\n",
			np[1], np[2], np[3], np[4], np[5]);
		for(i=0; i<MAXCOLS; i++)
			fprintf(f, "%s\n%d\n", colums[i].name, colums[i].maxpunkte);
		for(i=0; i<MAXROWS; i++)
			for(j=0; j<MAXCOLS; j++)
			{
				fprintf(f, "%d\n", rows[i].proz[j]);
			}
		if (fclose(f))
		{
			error = "Fehler beim Schlieáen der Datendatei.";
			return;
		}
		if (!dirty)
		{
			error = "Speichern OK";
			return;
		}
		f = fopen(namefile, "w");
		if (f)
		{
			for(i=0; i<MAXROWS; i++)
				fprintf(f, "%s\n", rows[i].name);
			if (fclose(f))
			{
				error = "Fehler beim Schlieáen der Namensdatei.";
				return;
			}
		}
		else
		{
			error = "Kann Namensdatei nicht ”ffnen.";
			return;
		}
	}
	else
	{
		error = "Kann Datendatei nicht ”ffnen.";
		return;
	}
	error = "Speichern OK, Namensdatei berichtigt und OK.";
	dirty = 0;
}


void loadall(const char *fn)
{   FILE *f;
	char dummy[40];

	f = fopen(fn, "r");
	if (f)
	{   int i, j;

		clear();
		fgets(title, sizeof(title), f); rmnl(title);
		fgets(namefile, sizeof(namefile), f); rmnl(namefile);
		fscanf(f, "%s %d %d %d %d %d\n",
			dummy, np+1, np+2, np+3, np+4, np+5);
		for(i=0; i<MAXCOLS; i++)
		{
			fgets(dummy, sizeof(dummy), f); rmnl(dummy);
			strncpy(colums[i].name, dummy, COLLEN);
			colums[i].name[COLLEN-1]=0;
			fscanf(f, "%d\n", &(colums[i].maxpunkte));
		}
		for(i=0; i<MAXROWS; i++)
			for(j=0; j<MAXCOLS; j++)
			{
				fscanf(f, "%d\n", &(rows[i].proz[j]));
			}
		fclose(f);
		loadnames(namefile);
	}
	else
	{
		error = "Kann Datendatei nicht ”ffnen.";
		return;
	}
	error = "Laden OK.";
}

void defscr(void)
{
	clrscr();
	gotoxy(1,3);
	printf("  t  - Titel:       %s\r\n", title);
	printf("  d  - Dateiname:   %s\r\n", filename);
	printf("  n  - Namen-Datei: %s\r\n", namefile);
	printf("  N  - nur Namen laden.\r\n");
	printf("  L  - alles Laden\r\n");
	printf("  S  - alles Speichern\r\n");
	printf("  F1 - Alles L”schen\r\n");
	printf("  D  - Drucken\r\n");
#if WEITEAKTIV
	printf("  U  - Umbruch nach %d Druckspalten.\r\n", Druckerweite);
#endif
	printf("\n  Weiterarbeiten mit F8 oder ESC,\r\n");
	printf("  Programm beenden mit F10.\r\n");
	textcolor(1);
	if (error)
		printf("\n  %s\r\n", error);
	textcolor(4);
	printf("\n\n  Im Arbeitsblatt:\n\r");
	printf("  F1-Namen,     F2-Prozente,  F3-MaxPunkte,     F4-Notenpunkte\n\r");
	printf("  F5-AufgNamen, F6-Punkte,    F7-NotenProzente, F8-Optionen\n\r");
	printf("  F9-Redraw,    + -Increment, - -Decrement\n\r");
	textcolor(0);
	gotoxy(1,25);
}



int definitions(void)
{	int ch, quit;

	error = NULL;
	quit = 0;
	while(!quit)
	{
		defscr();
		ch = getch();
		if (!ch)
			ch = -getch();
		gotoxy(1,1);
		switch(ch)
		{
		case 'D':
			drucken();
			error = "Drucken beendet";
			break;
		case 't':
			show(title, sizeof(title));
			editcell(0);
			error = NULL;
			break;
		case 'N':
			if (*namefile)
				loadnames(namefile);
			else
				error = "Kein Name angegeben.";
			break;
		case 'n':
			show(namefile, sizeof(namefile));
			editcell(0);
			error = NULL;
			break;
		case 'd':
			show(filename, sizeof(filename));
			editcell(0);
			error = NULL;
			break;
#if WEITEAKTIV
		case 'U':
			showint(&Druckerweite, cvtid);
			editcell(0);
			error = NULL;
			break;
#endif
		case -59:
			clear();
			break;
		case 'S':
			if (*filename)
				saveall(filename);
			else
				error = "Kein Name angegeben";
			break;
		case 'L':
			if (*filename)
				loadall(filename);
			else
				error = "Kein Name angegeben";
			break;
		case  27: // ESC
		case -66: // F8
			quit = 1;
			clrscr();
			break;
		case -68: // F10
			gotoxy(1,1);
			printf("%-40s", "Nochmal F10 zur Best„tigung!");
			ch = getch();
			if (!ch)
				ch = -getch();
			quit = (ch == -68);
			break;
		}
	}
	return ch==-68;
}


void commands(void)
{   int ch, quit, faktor;
	// This is the main loop

	display();
	quit = 0;
	while(!quit)
	{
		pending();
		ch = getch();
		if (!ch)
			ch = -getch();

redo:
//		gotoxy(75,25);
//		printf("%4d", ch);
		faktor=1;
		switch(ch)
		{
		case 0:
			break;
		case 27:  // ESC
		case -66: // F8
			quit = definitions();
			if (!quit)
				display();
			break;
		case -65: // F7
			edit = NOTENPROZENT;
			break;
		case -64: // F6
			edit = PUNKTE;
			break;
		case -63: // F5
			edit = AUFGABEN;
			break;
		case -62: // F4
			edit = NOTEN;
			break;
		case -61: // F3
			edit = MAX;
			break;
		case -60: // F2
			edit = PROZ;
			break;
		case -59: // F1
			edit = NAMES;
			break;
		case -73:
			faktor = 25-YOFS-1;
		case -72:
			currow = max(0, currow-faktor);
			if (currow<rowofs)
				rowofs = currow;
			break;
		case -81:
			faktor = 25-YOFS-1;
		case '\r':
		case -80:
			currow = min(MAXROWS-1, currow+faktor);
			if (currow-rowofs>25-YOFS-1)
				rowofs = currow-(25-YOFS-1);
			if (ch == '\r')
				display();
			break;
		case -75:
			if (isNOTE())
				curnot = max(1, curnot-1);
			else
				curcol = max(0, curcol-1);
			break;
		case -77:
			if (isNOTE())
				curnot = min(5, curnot+1);
			else
			curcol = min(MAXCOLS-1, curcol+1);
			break;
		case -67: // F9
			display();
			break;
		case '+':
			if (convert)
				(*number)++;
			break;
		case '-':
			if (convert)
				(*number)--;
			break;
		default:
			if (ch<=0)
				break;
			ch = editcell(ch);
			if (edit == NAMES)
				dirty = 1;
			goto redo;
		}
	}
}

int main(int argc, char **argv)
{
	textcolor(0);
	textbackground(15);
	*filename = 0;
	*namefile = 0;
	*title = 0;
	dirty = 0;
	clear();
	if (argc>1)
	{
		strcpy(filename, argv[1]);
		loadall(filename);
	}
	else
	if (!definitions())
	{
		clrscr();
		commands();
	}
	gotoxy(1,25);
	return 0;
}