#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "getopt.h"

#define VERSION "V1.7a"

#define contains(a,b) (NULL != strstr(a,b))
#ifndef __SMALL__
# error "Small Model please"
#endif

static char buf[513], buf1[513];
static any, upper, inv, keepl, keepr, with, chop,
       single, onoff, reverse, dual, dualm, pos, eofmet;
static long line, lineon, lineoff;
static char *splitchars;

int process(FILE *f, const char *s)
{
	while(fgets(buf1, sizeof(buf1), f))
	{
		dualm = 1;
		strcpy(buf, buf1);
		if (chop)
		{	int l;
		
			l = strlen(buf);
			if (l && buf[l-1] == '\n')
				buf[l-1] = 0;
		}
		if (!reverse)
			line++;

		if (lineoff && line == lineoff+1)
		{
			if (lineon<=lineoff && !with)
				return 0; /* nicht weitermachen */
			onoff = 0;
		}
		if (line == lineon)
			onoff = 1;

		if (!reverse && onoff == 0)
		{
			if (with)
			{
				dualm = 0;
				return 1;
			}
			continue;
		}
		
		if (upper)
			strlwr(buf1);
		if (any)
			return 1;
		if (inv)
		{
			if (!contains(buf1, s))
				return 1;
		}
		else
		{
			if (contains(buf1, s))
				return 1;
		}
		dualm = 0;
		if (dual)
			return 1;

	}
	return 0;
}

int issplit(const char c)
{
	if (splitchars)
		return (NULL != strchr(splitchars, c));
	return isspace(c);
}

int pr(int i)
{   char *s = buf;
	int chrs;

	chrs = 0;
	if (!single)
	{
		while(issplit(*s))
		{
			if (!*s)
				return chrs;
			if (keepl && i==0)
			{
				putchar(*s);
				chrs++;
			}
			s++;
		}
	}
	while(i--)
	{
		while(!issplit(*s))
		{
			if (!*s)
				return chrs;
			s++;
		}
		while(issplit(*s))
		{
			if (!*s)
				return chrs;
			if (keepl && i==0)
			{
				putchar(*s);
				chrs++;
			}
			s++;
			if (single)
				break;
		}
	}
	while(!issplit(*s))
	{
		if (!*s)
			return chrs;
		putchar(*s);
		chrs++;
		s++;
	}
	if (keepr)
	{
		while(issplit(*s))
		{
			if (!*s)
				return chrs;
			putchar(*s);
			chrs++;
			s++;
			if (single)
				break;
		}
	}
	return chrs;
}


void do_the_split(const char *s)
{	char numbuf[14];
	int num;

	if (!s)
		return;

	for(;*s;s++)
	{
		if (*s == '$')
		{
			s++;
			if (!*s)
				break;
			if (isdigit(*s))
			{
				pos += pr(*s - '0');
				continue;
			}
			switch(*s)
			{
			case '(':
				s++;
				num = 0;
				while(*s && isdigit(*s))
				{
					num *= 10;
					num += *s - '0';
					s++;
				}
				for(;pos<num; pos++)
					putchar(' ');
				break;
			case '[':
				s++;
				num = 0;
				while(*s && isdigit(*s))
				{
					num *= 10;
					num += *s - '0';
					s++;
				}
				pos += pr(num);
				break;
			case 't':
				pos += 7;
				pos %= 8;
				putchar('\t');
				break;
			case 'n':
				pos = 0;
				putchar('\n');
				break;
			case '-':
				if (gets(buf1))
				{
					fputs(buf1, stdout);
					pos += strlen(buf1);
				}
				else
					eofmet = 1;
				break;
			case 'e':
				pos = 0;
				putchar('\033');
				break;
			case 'l':
				pos=0;
				fputs(buf, stdout);
				break;
			case '$':
				pos++;
				putchar('$');
				break;
			case 's':
				pos++;
				putchar(' ');
				break;
			case '#':
				ltoa(reverse ? line-1 : line, numbuf, 10);
				fputs(numbuf, stdout);
				pos += strlen(numbuf);
				break;
			default:
				putchar('$');
				putchar(*s);
				pos+=2;
				break;
			}
		}
		else
		{
			putchar(*s);
			pos++;
		}
	}
}

void split(char **argv)
{	char *s;

	pos = 0;
	if (!dual)
	{
		while(NULL != (s=*argv))
		{
			do_the_split(s);
			
			argv++;
			if (*argv)
			{
				pos++;
				putchar(' ');
			}
		}
	}
	else
	{
		if (dualm)
			do_the_split(argv[0]);
		else
			do_the_split(argv[1]);
	}
}

int main(int argc, char **argv)
{	int opt, result = 0, newline, br, error;
	char *such, *infile, *argument;
	int scanfromstdin, debug;
	FILE *thefile;
	
	eofmet = 0;
	chop = 0;
	with = 0;
	dual = 0;
	line = lineon = lineoff = 0;
	onoff = 0;
	debug = 0;
	reverse = 0;
	any = 0;
	upper = 0;
	newline = 0;
	error = 0;
	br = 0;
	inv = 0;
	keepr = keepl = 0;
	single = 0;
	splitchars = NULL;
	scanfromstdin = 0;
	infile = such = NULL;
	while(EOF != (opt=getopt(argc, argv, "12CDKL:P:SIacf:ikl:no:p:rs:uw")))
	{
		/* has argument */
		if (strchr("LPflops",opt) && optarg[0]=='-' && optarg[1]==0)
		{
			/* read argument fron stdin */
			argument = buf;
			gets(argument);
			if (debug)
				fprintf(stderr, "-%c %s\n", opt, argument);
		}
		else
			argument = optarg;
			
		switch(opt)
		{
		case 'C':
			chop = 1;
			break;
		case 'w':
			with = 1;
			break;
		case '2':
			dual = 1;
			break;
		case 'l':
			lineon = atol(argument);
			break;
		case 'L':
			lineoff = atol(argument);
			break;
		case 'p':
			lineon = lineoff = atol(argument);
			break;
		case 'P':
			lineoff = atol(argument) - 1;
			lineon = lineoff + 2;
			break;
		case 'D':
			debug = !debug;
			break;
		case 'r':
			line = 1;
			reverse = 1;
			break;
		case 'I':
			scanfromstdin = 1;
			gets(buf);
			if (such)
				free(such);
			such = strdup(buf);
			break;
		case 'S':
			single = 1;
			break;
		case 'k':
			keepr = 1;
			break;
		case 'K':
			keepl = 1;
			break;
		case 's':
			if (splitchars)
				free(splitchars);
			splitchars = strdup(argument);
			break;
		case 'n':
			newline = 1;
			break;
		case 'u':
			upper = 1;
			break;
		case 'c':
			upper = 0;
			break;
		case '1':
			br = 1;
			break;
		case 'i':
			inv = 1;
			break;
		case 'f':
			if (infile)
				free(infile);
			infile = strdup(argument);
			break;
		case 'o':
			if (!freopen(argument, "w", stdout))
			{
				perror(argument);
				return 100;
			}
			break;
		case 'a':
			any = 1;
			break;
		case '?':
			error = 1;
			break;
		}
	}
	if (argc<2 || error)
	{
		puts("This is "__FILE__" "VERSION" (c) 1993,1994 by Martin Lercher");
		fprintf(stdout, "usage: %s [-2[w]] [-1acDikKnrSu] [-{l|L|p|P}<int>]\n", argv[0]);
		puts("    [-s<chrs>] [-f<infile>] [-o<outfile>] {-a|-I|<string>} [\"Text1\" [\"Text2\"]]\n");
		puts("  scan in stdin for lines containing <string>, if found");
		puts("  print the <Text1> substituting $0-$9 by the n-th token of the line,");
		puts("  if not and if -2 is present print the <Text2> in the same manner,");
		puts("  $$ means $, $l whole line. $n means Newline, $s space, $t tab,");
		puts("  $e ESC, $# current line number.");
		puts("  $[<int>] means the <int>th token of the line. (0 is first token).");
		puts("  $(<int>) means tab to position <int> (0 is first column).");
		puts("  $- means print the next line from stdin (uninterpreted). (-f recommended)");
		puts("Press <Enter> to see the switches.");
		gets(buf);
		puts("Switches:");
		puts("  -1 (1st line only) stop after first match");
		puts("  -2 (2 Texts) print <Text2> if not matched.");
		puts("  -a (any) match any line");
		puts("  -C (Chop) remove trailing newline.");
		puts("  -c casesensitive matching (default)");
		puts("  -D (debug toggle) print option arguments that are read from stdin to stderr");
		puts("  -f<file> process <file> instead of stdin for input");
		puts("  -I take <string> from stdin (use of -f recommended)");
		puts("  -i inverted matching of <string>");
		puts("  -k/-K keep right/left seperators of column");
		puts("  -l#/-L## select lines #..## if #<=## else select 1..## and #..eof");
		puts("  -n (newline) print newline after each match");
		puts("  -o<file> put output to <file>");
		puts("  -p#/-P# pick/omit line number #");
		puts("  -r (reverse) apply line numbers to matched lines instead of input lines.");
		puts("  -S single char seperates tokens.");
		puts("  -s<chrs> use list <chrs> of chars to split input line. (default whitespace)");
		puts("  -u (upper) caseinsensitive matching");
		puts("  -w (with lines) (-2 only) print Text2 on excluded lines.");
		puts("An argument - (eg. -L-) is taken from stdin.");
		puts("Returns 1 on success, 0 on failure (no match), 100 on error");
		return 100;
	}

	if (with && !dual)
	{
		fprintf(stderr, "-w can only be combined with -2.\n");
		return 100;
	}
	if (reverse && dual)
	{
		fprintf(stderr, "-r cannot be combined with -2.\n");
		return 100;
	}
	
	onoff = !(lineon && (lineoff==0 || lineon<=lineoff));
	
	if (!any)
	{
		if (!scanfromstdin)
			such = argv[optind++];
		if (!such)
		{
			fprintf(stderr, "No scan string specified. Provide it or -a or -I.\n");
			return 100;
		}
		if (upper)
			strlwr(such);
	}
	if (infile)
	{
		if (!(thefile = fopen(infile, "r")))
		{
			perror(infile);
			return 100;
		}
	}
	else
		thefile = stdin;
		
	while(process(thefile, such))
	{
		if (reverse)
		{
			line++;
			if (onoff)
			{
				split(&argv[optind]);
				if (newline)
					putchar('\n');
			}
		}
		else
		{
			split(&argv[optind]);
			if (newline)
				putchar('\n');
		}
		result = 1;
		if (br)
			break;
	}
	if (eofmet)
		fputs("scan: Premature EOF met on stdin (used by $-).", stderr);
		
	return result;
}
