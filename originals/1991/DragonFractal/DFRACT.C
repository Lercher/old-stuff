#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum dir { LEFT, RIGHT , PEN };

static xx, yy, dx, dy; /* innitial startpoint and direction */
static line;
static char *nonterminal[26]={NULL};
static FILE *f;
static char start;


void turn(enum dir dir, int what)
{  int hilf;

	if (dir == LEFT)
	{
		fputc('L', stderr);
		hilf = dx;
		dx = -dy;
		dy =  hilf;
		fprintf(f, "AR%d,%d,90;", dx, dy);
	}
	else if (dir == RIGHT)
	{
		fputc('R', stderr);
		hilf = dx;
		dx =  dy;
		dy = -hilf;
		fprintf(f, "AR%d,%d,-90;", dx, dy);
	}
	else
	{
		fprintf(f, "SP%d;", what);
	}
}


void parse(int n, char symbol)
{	char *ableit;

	if (symbol == 'L')
	{
		turn(LEFT, 0);
		return;
	}
	if (symbol == 'R')
	{
		turn(RIGHT, 0);
		return;
	}
	if ('0'<=symbol && symbol<='9')
	{
		turn(PEN, symbol-'0');
		return;
	}
	if ('a'>symbol || symbol>'z')
	{
		fprintf(stderr, "illegal symbol 0x%02x encountered.\n", (unsigned)symbol);
		return;
	}
	if(n)
	{
		ableit = nonterminal[symbol-'a'];
		if (ableit == NULL)
		{
			fprintf(stderr, "undefined nonterminal '%c' ignored.\n", symbol);
			return;
		}
		while(*ableit)
			parse(n-1, *ableit++);
	}
}


void init(int n, char *s)
{
	fprintf(f,
		"IN;SP1;PW0.05;\n"
		"PU100,300;"
		"LB%d dragons from file %s\003\n"
		"PU%d,%d;PD;\n",
		n, s, xx, yy);
}


void quit(void)
{
	fprintf(f, "\nPE;IN;" );
	fflush(f);
}


void error(char *s)
{
	fprintf(stderr, s);
	fprintf(stderr, " in line%3d ignored.\n", line);
}


int rmnl(char *s)
{  int l;

	l = strlen(s);
	if (l>0 && s[l-1] == '\n')
	{
		s[l-1] = 0;
		return l-1;
	}
	return l;
}

char buf[100];

void scan_the(FILE *def)
{  int l;
	char c;

	line = 0;
	while(fgets(buf, sizeof(buf), def))
	{
		line++;
		l = rmnl(buf);
		if (l==0 || *buf == '#')
			continue;
		if (l>2 && buf[1] == '>')
		{
			c = *buf;
			if ('a'<=c && c<='z')
			{
				nonterminal[c-'a']=strdup(buf+2);
			}
			else
			{
				error("illegal nonterminal char");
			}
			continue;
		}
		if (l>2 && buf[1]=='=')
		{
			c = *buf;
			switch(c)
			{
			case 'S':
				c = buf[2];
				if (c == 'L' || c == 'R' || 'a'<=c && c<='z')
					start = c;
				else
					error("illegal startsymbol S");
				continue;
			case 'X':
				xx = atoi(buf+2);
				continue;
			case 'Y':
				yy = atoi(buf+2);
				continue;
			case 'x':
				dx = atoi(buf+2);
				continue;
			case 'y':
				dy = atoi(buf+2);
				continue;
			default:
				error("illegal set command");
				continue;
			}
		}
		error("unknown directive");
	}
}


int main(int argc, char *argv[])
{	int n;
	FILE *def;

	f = stdout;
	fprintf(stderr, "This program prints the Dragoncurve-like fractals in HP-GL to stdout.\n");
	fprintf(stderr, "author: Dipl. Math. Martin Lercher. Compiled "__DATE__".\n");
	if (argc<2)
	{
		fprintf(stderr, "usage: %s <defn-file> [#n], where\n", argv[0]);
		fprintf(stderr, "\t#n means the recursion depth<14.\n");
		fprintf(stderr, "\t<defn-file> means the file, that contains the definitions\n");
		fprintf(stderr, "\t            for derivation of the curve.\n");
		return 1;
	}
	def = fopen(argv[1], "r");
	if (!def)
	{
		fprintf(stderr, "%s: file not found\n",
			argv[0]);
		return 2;
	}
	if (argc>2)
		n = atoi(argv[2]);
	else
		n = 3;
	if (n<=0 || n>13)
	{
		fprintf(stderr, "%s: illegal recursion deep.\n",
			argv[0]);
		return 3;
	}
	dx = 0; dy = -40;
	xx = 1000; yy= 2200;
	start = 'a';
	scan_the(def);
	init(n, argv[1]);
	parse(n, start);
	quit();
	return 0;
}
