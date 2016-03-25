#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

FILE *file;
int day,month,year,wd0,wd1,disp,line;
static struct tm *tblock;
static time_t timer;
static int deltaday;

static char *preif = NULL;
static int precmd = 0;
int done = 0, done2 = 0;

void yyerror(char *s)
{
	fprintf(stderr, "\n%s in line %d, Aborting ...", s, line);
	exit(10);
}



int main(int argc, char **argv)
{	extern void yyparse(void);

	if ((argc != 2) || (0 == strcmp(argv[1], "/?")))
	{
		printf("This is "__FILE__" Version 1.0, Compiled "__DATE__".\n");
		printf("Author: Martin Lercher (martin@alf2.ngate.uni-regensburg.de)\n");
		printf("Usage: %s <filename>\n", argv[0]);
		printf("  According to the commands in <filename> and the current date,\n");
		printf("  either a string is printed, or a shell command is executed via\n");
		printf("  the system() call.  Two flags are maintained which indicate,\n");
		printf("  that an action was performed. #1 of these flags is resettable by\n");
		printf("  the keyword 'clear'. If any action was performed (flag #2) 1 is returned.\n");
		printf("Syntax of the contents of <filename>:\n");
		printf("  Each line contains either the keyword 'clear', which clears flag #1, or\n");
		printf("  <prefix> : [!] \"<text>\"    prints|executes(!) <text> if <prefix> succeeds,\n");
		printf("     where <text> is any Sequence of chars not containing '\"', and\n");
		printf("  <prefix> is {<wd> | <wd>-<wd>}   <wd> is {mo|di|mi|do|fr|sa|so} \n");
		printf("     succedes if current weekday is <wd> or within <wd>-<wd>\n");
		printf("  <prefix> is <day>.<month>.<year>   Each can be replaced by '*' as a wildcard.\n");
		printf("     succedes if current date is <day>.<month>.<year> of the date-line\n");
		printf("  <prefix> is {+|-}<integer> Let D be <day>.<mon>.<year> of the last date-line.\n");
		printf("     succedes if current date is within D{+|-}1 and D{+|-}<integer>. \n");
		printf("     <text> may contain '$' which is substituted by the actual difference.\n");
		printf("  <prefix> is {now|if|xif|else|xelse|section}\n");
		printf("     These keywords do not modify the flags, but test them. \n");
		printf("     if   - succeeds if flag #1 is set.   xif   - succeeds if flag #2 is set.\n");
		printf("     else - succeeds if flag #1 is reset. xelse - succeeds if flag #2 is reset.\n");
		printf("     now  - succeeds.  section - succeeds when(if) the next <prefix> succedes.\n");
		printf("Press <enter> to see an example file ... ");
		(void) getch();
		printf("\rAn example file:                               \n");
		printf("%s", "  % This is a comment, and is ignored until \\n\n");
		printf("  section : \"---------- Dates ----------\"\n");
		printf("  1.1.*   : \"New years day\"\n");
		printf("  1.*.93  : \"The first day of any month in 1993.\"\n");
		printf("  +1      : \"The day after\"\n");
		printf("  -3      : \"$ day(s) before\"\n");
		printf("  clear\n");
		printf("  section : \"---------- Weekends ----------\"\n");
		printf("  Fr      : \"Weekend comes ...\"\n");
		printf("  Mo-Fr   : \"I'm working.\"\n");
		printf("  Sa-So   : \"Weekend.\"\n");
		printf("  if      : \"User ... Weekend ... look to the screen!\"\n");
		printf("%s", "  % Here comes a shell command:\n");
		printf("  else    : !\"dir *.c /p /s\"\n");
		printf("  xif     : !\"wait4key.bat\"\n");
		printf("Notes:\n");
		printf("  A file that does not end with '\\n' causes a 'parse error'.\n");
		printf("  'parse error's abort the processing of <filename> immediately.\n");
		printf("  <prefix> commands are case insensitive.\n");
		printf("  Literal '\"'s or '$'s can be included in a string by writing \\\", \\$.\n");
		printf("  A double backslash '\\\\' is reduced to a literal '\\'.\n");
		printf("  An unterminated string causes in general a 'parse error'.\n");
		printf("  Blanks and Tabs outside strings and numbers are ignored.");
		return 0;
	}

	file = fopen(argv[1], "r");
	if (!file)
	{
		perror(argv[1]);
		return 1;
	}
	line = 1;
	yyparse();
	return done2;
}

void doit(char *command, int cmd)
{
        textcolor(BLACK+BLINK);
        textbackground(WHITE);
	if (cmd)
		system(command);
	else
	{
		for(;*command;command++)
		{
			if (*command == '\\' && command[1] == '$')
				command++;
			else if (*command == '$')
			{
				printf("%d", deltaday);
				continue;
			}
			printf("%c", *command);
		}
		printf("\n", command);
	}
}

void doit2(char *command, int cmd)
{
	if (preif)
	{
		doit(preif, precmd);
		preif = NULL;
	}
	done = done2 = 1;
	doit(command, cmd);
}

void special(char *s, int cmd, int what)
{
	switch(what)
	{
	case 1: /* now */
		doit(s, cmd);
		break;
	case 2: /* else */
		if (!done)
			doit(s, cmd);
		break;
	case 3: /* if */
		if (done)
			doit(s, cmd);
		break;
	case 4: /* section */
		precmd = cmd;
		if (preif)
			free(preif);
		preif = strdup(s);
		if (!preif)
			yyerror("out of memory");
		break;
	case 5: /* xif */
		if (done2)
			doit(s, cmd);
		break;
	case 6: /* xelse */
		if (!done2)
			doit(s, cmd);
		break;
	}
}


void execute(char *command, int cmd)
{
	int wd;
	int i;
	time_t delta;

	timer = time(NULL);
	tblock = localtime(&timer);
	deltaday = 0;

	wd = tblock->tm_wday;
	if ( (wd0 == wd1) && (wd0 == wd) )
	{
		doit2(command, cmd);
		return;
	}

	if ((wd0<wd1) &&  (wd0<=wd) && (wd<=wd1))
	{
		doit2(command, cmd);
		return;
	}

	if ((wd0>wd1) && ((wd0<=wd) || (wd<=wd1)))
	{
		doit2(command, cmd);
		return;
	}
	if (wd0>=0)
		return;

	if (year >= 1900)
		year -= 1900;

	if (  disp == 0
	 &&	((day == -1) || (day == tblock->tm_mday))
	 && ((month == -1) || (month-1 == tblock->tm_mon))
	 && ((year == -1) || (year == tblock->tm_year))
	  )
	{
		doit2(command, cmd);
		return;
	}

	if (disp>0)
	{
		delta = 3600l * 24l;
	}
	else
	{
		delta = -3600l * 24l;
		disp = -disp;
	}
	if (disp>40)
		fprintf(stderr, "%3d: Displacement %d groá ", line, disp);
	for(i=disp; i>0; i--)
	{
		timer -= delta;
		tblock = localtime(&timer);

		if (
			((day == -1) || (day == tblock->tm_mday))
		 && ((month == -1) || (month-1 == tblock->tm_mon))
		 && ((year == -1) || (year == tblock->tm_year))
		  )
		{
			if (disp>40)
				fprintf(stderr, "\n");
			deltaday = disp - i + 1;
			doit2(command, cmd);
			return;
		}
		if (disp>40 && ((i % 16) == 0))
			fprintf(stderr, ".");
	}
	if (disp>40)
		fprintf(stderr, "\n");
}


