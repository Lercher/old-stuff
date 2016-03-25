/*  ein Programm zum aktuellhalten von Dateien
 *  mL@suntari	Version 1.0 vom 25 Nov 1990.
 *  Martin Lercher
 *  Schl”álgartenweg 40
 *  D-8415 Nittenau
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloc.h>
#include <dos.h>

#include "getopt.h"

#define VERSION "MSDOS Version 1.6"
#define NAME "Refresh"

char SCCS_ID[]="@""(#)"__FILE__" "VERSION"  Compiled "__DATE__" "__TIME__".\n";

#ifndef __SMALL__
#  error "SMALL model required"
#endif


#define MAXLEN 160

static int verbose=1;		/* bei 0 ist refresh sehr schweigsam */
static int returncode=0;
static int type;		/* type a file */
static int newline=1;		/* print a newline after copy msg */
static int security=1;		/* *.$$$ anlegen? */
static int nocopy=0;		/* so tun als ob? */
static int create=0;		/* subdirs erzeugen? */
static char f1[MAXLEN], f2[MAXLEN];	/* basedir 1 and 2 */
static char rstr[MAXLEN]="*.*";	/* refresh string */

FILE *rf;	/* refresh file */

struct entry {
	struct entry *next;
	char *name;
};	/* Struktur fuer Directory-Eintrag */


void usage(void)
{
	fputs("usage: "NAME" [-cdnqrvQ] [-f{string} -F{refreshfile|-}] dir1 [dir2]\n", stderr);
	fputs("  -v Verbose\n  -c create subdirs\n  -d don't make tempfiles.\n", stderr);
	fputs("  -q no headerline\n  -n don't copy (just compare)\n  -r don't use newline.\n", stderr);
	fputs("  -Q totally quiet operation.\n", stderr);
	fputs("  Copy the latest versions of each file (determinded by date) listed\n", stderr);
	fputs("  in <refreshfile> ('-' means stdin) or specified by <string>\n", stderr);
	fputs("  from <dir1> to <dir2> (defaults to '.') and vice versa.\n", stderr);
	exit(16);
}


void eeol(FILE *f)
{
	fputs("\033[K", f);
}


void nl(void)
{
	if (newline)
		fputs("\n", stdout);
	else
	{
		eeol(stdout);
		fputs("\r", stdout);  /* eeol() and \r */
	}
}

void *xmalloc(size_t len)	/* sicheres allozieren */
{	void *p;
	
	p = malloc(len);
	if (!p)
	{
		fputs("No more core-memory, aborting.\n", stderr);
		exit(returncode|32);
	}
	return p;
}

/* haenge einen '\\' an s an, falls noch keiner da ist */
void appendslash(char *s)
{	int l;

	l = (int) strlen(s);
	if ( l && s[l-1] != '\\' && s[l-1]!=':')
	{
		s[l]='\\';
		s[l+1]='\0';
	}
}


char *cons(char *d, const char *s1, const char *s2)
{
	strcpy(d, s1);
	return strcat(d, s2);
}


/* entferne whitespace von anfang und ende von string.
 * Rueckgabewert ist die Anwesenheit eines ':'.
 */
int strip(char *string)
{	char *s=string;
	int colon=0;
	
	/* delete comment '% ... \n' */
	while(*s)
	{
		if (':' == *s)
			colon++;
		if ('%' == *s)
			*s=0;
		else
			s++;
	}
	/* delete trailing whitespace */
	while(isspace(*s) || 0 == *s)
	{
		*s=0;
		if (s==string)
			break;
		s--;
	}
	/* delete leading whitespace */
	for(s=string; isspace(*s); s++)
		;
	if (s!=string)
		strcpy(string, s);
	return colon;
}

/* oeffne ein file, auch wenn ein entsprechendes subdir nicht
 * vorhanden ist. (falls create!=0)
 */
int mkfile(char *s)
{	int w;
	char *ss, *os=NULL;

	if (0<(w=open(s, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IFBLK|S_IFREG|S_IWRITE)) || !create)
		return w;
	/* splitte s der reihe nach an '\\' */
	for(ss=s; NULL != (ss=strchr(ss+1, '\\')); )
	{
		if (ss[-1] == ':') /* a:\foo -> a: verhindern */
			continue;
		*ss = 0;		/* splitten */
		if (0<=mkdir(s))
			os = ss;	/* letztes erfolgreiches anlegen merken */
		*ss = '\\';		/* joinen */
	} 
	if (verbose && os)
	{	/* informiere den Benutzer */
		*os = 0;	/* split */
		fputs("Directory ", stdout);
		fputs(s, stdout);
		fputs(" created.", stdout);
		nl();
		*os = '\\';	/* join */
	}
	returncode |= os ? 2:0; 	/* hat Verzeichnis angelegt */
	return (os ? open(s, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IFBLK|S_IWRITE|S_IFREG) : -1);
}


/* kopieren von src nach dest, Zeit time setzen */
void copy(char *dest, char *src, struct ftime *time)
{	void *buf;
	char ren[MAXLEN];
	char *dot, *slash;
	int err=0;
	
	strcpy(ren, dest);
	if (security)
	{
		slash = strrchr(ren, '\\');	/* letzter '\\' in ren */
		if (!slash)
			slash = ren;
		dot = strrchr(slash, '.');	/* von dort aus nach '.' suchen */
		if (!dot)
			strcat(slash, ".$$$");
		else
			strcpy(dot+1,"$$$");	/* ".$$$" anhaengen */
		if(0==strcmp(ren, dest))	/* ren existiert schon */
		{
			fputs("can't handle file ", stderr);
			fputs(src, stderr);
			fputs("\n", stderr);
			return;
		}
	} /* security */
	if (verbose)
	{	/* benutzer ueber die Kopierabsicht informieren */
		if (nocopy)
			fputs("would ", stdout);
		fputs("copy ", stdout);
		fputs(src, stdout);
		fputs(" to ", stdout);
		fputs(dest, stdout);
		nl();
	}

	if (!nocopy)	/* nur so tun als ob? */
	{	int wr, re;
		size_t r, len;

		/* Speicher so viel wie moeglich anfordern */
		buf = xmalloc(len = coreleft()-1024);

		re = open(src, O_BINARY|O_RDONLY , S_IFREG|S_IFBLK|S_IREAD);
		if(re<0)
		{
			perror(src);
			free(buf);
			return;
		}
		wr = mkfile(ren);	/* oeffnen und evtl. subdir erzeugen */
		if(wr<0)
		{
			perror(ren);
			err++;
			goto error1;
		}
		while(0 != (r=read(re, buf, len)))
		{	/* solange noch was da ist */
			if(r != write(wr, buf, r))
			{
				perror(ren);
				if (errno == ENOSPC)	/* floppy voll? */
					exit (returncode|64);	/* hat keinen Sinn mehr */
				err++;
				goto error;	/* sonst weitermachen */
			}
			if (type)
			{       size_t s;

				for(s=0; s<r; s++)
					fputc(((char *)buf)[s], stdout);
			}
		}
error:		
		if (!security)
		{
			setftime(wr, time);
			returncode |= 1;
		}
		if (close(wr)<0)
		{
			perror(ren);
			err++;
		}
error1:
		close(re);
		free(buf);	/* Kopierspeicher freigeben */
		if(err)
			returncode |= 4;
		else
		if (security)
		{
			unlink(dest);	/* dest ueberfluessig */
			if(errno == EACCES)
				perror(dest);	/* ist readonly */
			else
			{
				if(rename(ren, dest))
					perror(ren);
				else
				{
					wr = open(dest, O_BINARY|O_WRONLY, S_IFBLK|S_IWRITE|S_IFREG);
					if(setftime(wr, time)<0)	/* Zeit stempeln */
						perror(dest);
					close(wr);
					returncode |= 1;	/* habe kopiert */
				}
			}
		}
	} /* if (!nocopy) */
	if (type)
		fputs("\r", stdout);
}


unsigned long mymktime(struct ftime *t)
{       unsigned long tt;

	tt = ((((t->ft_year*16ul
	   + t->ft_month)*32ul
	   + t->ft_day)*32ul
	   + t->ft_hour)*64ul
	   + t->ft_min)*32ul
	   + t->ft_tsec;
	return tt;
}


/* Zeiten vergleichen */
void compare(char *a, char *b)
{	int h,k;
	unsigned long ta,tb;
static	struct ftime t1, t2;

	h = open(a, O_RDONLY, S_IFREG|S_IREAD);
	k = open(b, O_RDONLY, S_IFREG|S_IREAD);
	getftime(h, &t1);
	getftime(k, &t2);
	close(h);
	close(k);

	if (h<0 && k<0)
	{	/* beide files nicht da */
		if (verbose)
		{
			fputs("files ", stdout);
			fputs(a, stderr);
			fputs(" and ", stdout);
			fputs(b, stdout);
			fputs(" do not exist.\n", stdout);
		}
		return;
	}
	if ( h<0 )
	{	/* a nicht vorhanden */
		if (verbose>1)
		{
			fputs("file ", stdout);
			fputs(a, stdout);
			fputs(" does not exist.\n", stdout);
		}
		copy(a, b, &t2);
		return;
	}
	if ( k<0 )
	{	/* b nicht vorhanden */
		if (verbose>1)
		{
			fputs("file ", stdout);
			fputs(b, stdout);
			fputs(" does not exist.\n", stdout);
		}
		copy(b, a, &t1);
		return;
	}
	/* wer ist aelter? */
	if ((ta=mymktime(&t1)) == (tb=mymktime(&t2)))
		return;
	if (ta < tb)
		copy(a, b, &t2);	/* a aelter */
	else
		copy(b, a, &t1);	/* b aelter */
}

/* wildcards vorhanden, directory speichern und  mit 
 * compare() bearbeiten.
 */
void make(const char *d, const char *s, char *filename)
{		int r;
		struct ffblk *f;
		struct entry *p, *start;	

static	char dest[MAXLEN];
static	char src[MAXLEN];
static	char work[MAXLEN];
static	char work2[MAXLEN];
static  struct ffblk ffff;

	strcpy(dest, d);
	strcpy(src, s);
	appendslash(dest);
	appendslash(src);
	if (*filename == 0)
		strcpy(filename, "*.*");

	start = (struct entry *) xmalloc(sizeof(struct entry));
	p = start;
	p->next = NULL;
	
	f = &ffff;
	r = findfirst(cons(work, src, filename), f, FA_RDONLY|FA_HIDDEN);
	while(0 == r)	
	{
		p->name = xmalloc(strlen(f->ff_name)+1);
		strcpy(p->name, f->ff_name);
		strlwr(p->name);
		p->next = xmalloc(sizeof(struct entry));
		p = p->next;
		p->next = NULL;
		r = (findnext(f));
	}
	p = start;
	while(p->next)
	{	struct entry *pp;

		cons(work, dest, p->name);
		cons(work2, src, p->name);
		free(p->name);
		compare(work, work2);
		pp = p;
		p = p->next;
		free(pp);
	}
}

/* Ist ein wlidcard in s enthalten oder ein '\\' am Schluss? */
int scanspecial(const char *s)
{	const char *ss = s;

	for( ; *s ; s++)
	{
		if (*s=='*' || *s=='?')
			return 1;	/* ja */
	}
	if (s!=ss && s[-1] == '\\')
		return 2;
	return 0;	/* nein */
}

void error(char *s, char *t)
{
	fputs("\r", stderr);
	eeol(stderr);
	fputs(s, stderr);
	fputs(t, stderr);
	fputs("\n", stderr);
}


/* die hauptprozedur */
void refresh(const char *a, const char *b)
{	char string[MAXLEN];
	char p1[MAXLEN*2];
	char p2[MAXLEN*2];
	int goon = 1;
	
	while(goon)
	{
		type = 0;
		if (rf)	/* aus datei lesen? */
		{
			goon = (int) fgets(string, MAXLEN, rf);
			if (!goon)
				break;
		}
		else
		{	/* von rstr lesen */
			goon = 0;
			strcpy(string, rstr);
		}
once_more:
		if (strip(string))
		{
			if (*string != ':')
			{
				error("No colons allowed here: ", string);
				continue;
			}
			switch(string[1])
			{
			case 't':
				type = 1;
				if (string[2] != ' ')
				{
					error("Invalid Syntax for type, skipping ", string);
					continue;
				}
				strcpy(string, string+2);
				goto once_more;
			case 0:
				error("Missing argument to ':', skipping ", string);
				continue;
			default:
				error("Command error, skipping ", string);
				continue;
			}

		}
		if (*string == '\\')
		{
			error("No absolute paths allowed: ", string);
			continue;
		}
		if (*string)
		{	int i;
		
			if (!scanspecial(string))
			{	/* simple file */
				compare(cons(p1, a, string), cons(p2, b, string));
				continue;
			}
			for (i=(int) strlen(string)-1;
				 i+1 && string[i]!='\\';
				 i--) { }
			if (i+1)	/* a '\\' in string */
			{
				string[i]=0;	/* split at '\\' */
				cons(p1, a, string);
				cons(p2, b, string);
				make(p1, p2, string+i+1);
				make(p2, p1, string+i+1);
			}
			else	/* no '\\' in string */
			{
				make(a, b, string);
				make(b, a, string);
			}
		}
	}
}


main(int argc, char *argv[])
{	int opt, err=0, explicitfile=0, explicitstring=0;
	int printheader = 1;
	char ref[MAXLEN] = "Refresh";	/* refresh file */

	opterr = 0;	/* eigene Fehlerbehandlung */
	while((opt=getopt(argc, argv, "vqQncdrF:f:"))!=EOF)
	{
		switch(opt)
		{
		case 'd':
			security=0;
			break;
		case 'c':	/* create subdir */
			create++;
			break;
		case 'n':	/* don't copy */
			nocopy++;
			break;
		case 'q':
			printheader = 0;
			break;
		case 'Q':	/* no Verbose */
			verbose=0;
			break;
		case 'v':	/* Verbose */
			verbose=2;
			break;
		case 'f':	/* refreshstring */
			strcpy(rstr, optarg);
			explicitstring++;
			break;
		case 'F':	/* refreshfile */
			strcpy(ref, optarg);
			explicitfile++;
			break;
		case 'r':	/* only return for copy msg */
			newline = 0;
			break;
		case '?':	/* any error */
			err++;
			break;
		}
	}
	if (err)
	{
		fputs("Option parse error.\n", stderr);
		usage();
	}
	if (optind < argc)
	{	/* mind. ein Argument */
		strcpy(f1, argv[optind++]);
		if (optind < argc)
		{	/* mind. zwei Argumente */
			strcpy(f2, argv[optind++]);
			if (optind < argc)	/* drei oder mehr? */
				fputs("additional parameters ignored!\n", stderr);
		}
		else
			*f2 = '\0';	/* aktuelles dir fuer f2 */
	}
	else	/* kein 1. Parameter */
		usage();
	rf = NULL;
	if (!explicitstring)	/* -f hat Vorrang vor -F */
	{
		if (0==strcmp("-", ref))
			rf = stdin;				/* use stdin as refresh file */
		else
			rf = fopen(ref, "r");	/* open refresh file */
		if(rf==NULL && explicitfile)
		{
			perror(ref);
			return(16);
		}
	}
	appendslash(f1);
	appendslash(f2);
	if(verbose)
	{
		if (printheader)
			fputs(SCCS_ID+4, stderr);
		if (verbose>1)
		{
			fputs("Author: Martin Lercher, Schl”álgartenweg 40, D-8415 Nittenau, Tel. 09436/8475\n", stdout);
			fputs("As usual: ABSOLUTELY NO WARRENTY FOR ANYTHING ...\n", stdout);
			fputs("There is also an (almost compatible) AtariST Version of Refresh too.\n", stdout);
		}
/*
 *		fputs("Memory available: ", stderr);
 *		fputs(ultoa(((unsigned long)coreleft()), ref, 10), stderr);
 *		fputs(" Bytes.\n", stderr);
 */
	}
	if (0 == strcmp(f1, f2))
	{
		fputs("Nothing to do.\n", stderr);
		return 0;
	}
	refresh(f1, f2);	/* Do the work */
	if (rf && rf != stdin)
		fclose(rf);
	if (verbose && !nocopy && !returncode && newline)
		fputs("Project was allready up to date.\n", stdout);
	return returncode;
}
