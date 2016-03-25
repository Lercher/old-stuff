#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "getopt.h"

struct prj {
	unsigned char prjnr;
	time_t time;
};
static time_t summe[256];
static int seen[256];

#define FILENAME  "project.log"
#define IFILENAME "project.inf"
static char *name, *iname, *template;
static int cpr = 0;

int last(void)
{   struct prj p;
	long l;
	int r;
	FILE *f;

	f = fopen(name, "rb");
	if (!f)
	{
		perror(name);
		exit(1);
	}
	l = sizeof(struct prj);
	if(fseek(f, -l, SEEK_END))
		r = 0;
	else if (1 != fread(&p, sizeof(struct prj), 1, f))
		r = 0;
	else r = (int) p.prjnr;
	fclose(f);
	return r;
}

void start(int nr, time_t t)
{   struct prj p;
	FILE *f;

	p.prjnr = (unsigned char) nr;
	p.time = t;

	f = fopen(name, "ab");
	if (!f)
	{
		perror(name);
		exit(2);
	}
	if (1 != fwrite(&p, sizeof(struct prj), 1, f))
	{
		perror(name);
		exit(3);
	}
	if (fclose(f))
    {
		perror(name);
		exit(4);
	}
}

void collect(time_t t)
{   struct prj p;
	time_t tim;
	FILE *f;
	int nr;

	f = fopen(name, "rb");
	if (!f)
	{
		perror(name);
		exit(3);
	}
	tim = 0;
	nr = 0;
	while(fread(&p, sizeof(struct prj), 1, f))
	{
		seen[nr]++;
		summe[nr] += p.time - tim;
		nr = (int)p.prjnr;
		tim = p.time;
	}
	seen[nr]++;
	summe[nr] += t - tim;
	cpr = nr;
	fclose(f);
}

void showline(int line, int crlf)
{	FILE *f;
	char buf[512];
	int l, i;

	f = fopen(iname, "r");
	if (!f)
		if (crlf)
			return;
		else
		{
			perror(iname);
			exit(7);
		}
	i = 1;
	while(fgets(buf, sizeof(buf)-1, f) && i<line)
		i++;
	if (i==line)
	{
		l = strlen(buf);
		if (l && buf[l-1] == '\n')
			buf[l-1] = 0;
		printf("%s", buf);
	}
	fclose(f);
	if (crlf)
		putchar('\n');
}

void printt(int nr)
{   char *s;

	if (nr == -1)
		nr = cpr;
	if (nr == 0 || !seen[nr])
		return;
	if (!template)
	{
		printf("%d %ld %d\n", nr, summe[nr], seen[nr]);
		return;
	}
	for(s=template; *s; s++)
	{
		if (*s == '$')
		{
			switch(s[1])
			{
			case 'P':
				printf("%d", nr);
				s++;
				break;
			case 'S':
				printf("%ld", summe[nr]%60);
				s++;
				break;
			case 'M':
				printf("%ld", (summe[nr]%3600)/60);
				s++;
				break;
			case 'H':
				printf("%ld", summe[nr]/3600);
				s++;
				break;
			case 's':
				printf("%ld", summe[nr]);
				s++;
				break;
			case 'N':
				printf("%d", seen[nr]);
				s++;
				break;
			case 'c':
				printf("%d", cpr);
				s++;
				break;
			case 'A':
				printf("%ld.%02d", summe[nr]/seen[nr], (int)((100*summe[nr]/seen[nr])%100));
				s++;
				break;
			case 'h':
				printf("%ld.%02d", summe[nr]/3600, (int)((100*summe[nr]/3600)%100));
				s++;
				break;
			case 't':
				showline(nr, 0);
				s++;
				break;
			case 'm':
				printf("%ld", summe[nr]/60);
				s++;
				break;
			case 'n':
				putchar('\n');
				s++;
				break;
			case 0:
				break;
			default:
				putchar(*++s);
				break;
			}
		}
		else
			putchar(*s);
	}
}

void doreport(int nr)
{   int i;

	if (nr)
		printt(nr);
	else
		for (i=1; i<256; i++)
			printt(i);
}

void dostdreport(void)
{   int i;
	time_t sum;

	if (cpr)
		printf("Currently working on projekt %d.\n", cpr);
	for(i=1; i<256; i++)
	{
		if (seen[i])
		{
			sum = summe[i];
			showline(i, 1);
			printf("\tTotal of %d: %lds = %ldh %ldmin %lds in %d part(s).\n",
				i, sum, sum/3600, (sum%3600)/60, sum%60, seen[i]);
		}
	}
}

int main(int argc, char **argv)
{   int nr, opt, report, stdreport;
	time_t t;

	report = -100;
	stdreport = 0;
	if (argc<2)
	{
		printf("This is Project V1.0   Compiled "__DATE__". Martin Lercher.\n");
		printf("usage: %s [-crv] [-f logfile] [-F infofile] [-t \"teplate\"] ", *argv);
		printf("[-R prjnr] [0|1-255]\n");
		printf("\tWork on project 1-255 described by lines in infofile.\n");
		printf("\t0 stop working on current project.\n");
		printf("\tlogfile is "FILENAME" by default.\n\tCan be overridden by EnvVar PROJECTLOG.\n");
		printf("\tinfofile is "IFILENAME" by default.\n\tCan be overridden by EnvVar PROJECTINF.\n");
		printf("\t-c report current project\n");
		printf("\t-r report all projects\n");
		printf("\t-R report project nr prjnr\n");
		printf("\t-v generate standard report (no templates used)\n");
		printf("\t-t use template as a template for reports:\n");
		printf("\t$P prjnr  $c current prj  $n newline  $t Text from infofile\n");
		printf("\t$H hours integral  $M minutes (0-60)  $S seconds (0-60)\n");
		printf("\t$h hours fraction  $m minutes total   $s seconds total\n");
		printf("\t$N Number of parts  $A Average time in seconds (fraction).\n");
		return 255;
	}
	name = getenv("PROJECTLOG");
	if (!name)
		name = FILENAME;
	iname = getenv("PROJECTINF");
	if (!iname)
		iname = IFILENAME;
	t = time(NULL);
	while(EOF != (opt=getopt(argc, argv, "cf:rt:vF:R:")))
	{
		switch(opt)
		{
		case 'c':
			report = -1;
			break;
		case 'r':
			report = 0;
			break;
		case 'R':
			report = atoi(optarg);
			break;
		case 'f':
			name = optarg;
			break;
		case 'F':
			iname = optarg;
			break;
		case 't':
			template = optarg;
			break;
		case 'v':
			stdreport = 1;
			report = 0;
			break;
		default:
			return 254;
		}
	}

	if (optind < argc)
	{
		nr = atoi(argv[optind]);
		start(nr, t);
	}

	if (report>=-1)
	{
		collect(t);
		if (stdreport)
			dostdreport();
		else
			doreport(report);
	}

	return 0;
}
