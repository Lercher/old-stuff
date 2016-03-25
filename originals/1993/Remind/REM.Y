%{
#include <stdio.h>
#include <stdlib.h>

char buf[2048];
void *alloca(size_t bytes);
int yylex(void);

extern FILE *file;
extern int day,month,year,wd0,wd1,disp,line,done;
extern void yyerror(char *s);
extern void execute(char *s, int cmd);
extern void special(char *s, int cmd, int what);
%}

%union {
int val;
char *str;
}

%token <val> DISPLACEMENT
%token <str> COMMAND
%type  <val> weekday
%type  <val> num
%type  <val> cmd
%type  <val> special

%%
input : /* nix */
  | input commandline
;

commandline :  '\n'
  | 'c' 'l' 'e' 'a' 'r' '\n'      { done = 0; }
  | prefix  ':' cmd COMMAND '\n'  { execute($4, $3); }
  | special ':' cmd COMMAND '\n'  { special($4, $3, $1); }
;

cmd : /* nix */ { $$ = 0; }
  | '!' { $$ = 1; }
;

special :
    'n' 'o' 'w'                 { $$ = 1; }
  | 'e' 'l' 's' 'e'             { $$ = 2; }
  | 'i' 'f'                     { $$ = 3; }
  | 's' 'e' 'c' 't' 'i' 'o' 'n' { $$ = 4; }
  | 'x' 'i' 'f'                 { $$ = 5; }
  | 'x' 'e' 'l' 's' 'e'         { $$ = 6; }
;

prefix :
    '+' DISPLACEMENT { disp =  $2; }
  | '-' DISPLACEMENT { disp = -$2; }
  | num '.' num '.' num { disp=0; wd0 = wd1 = -1; day = $1; month = $3; year = $5; }
  | weekday '-' weekday { wd0 = $1; wd1 = $3 }
  | weekday { wd0 = wd1 = $1 }
;

num : '*' { $$ = -1; }
  | DISPLACEMENT { $$ = $1; }
;

weekday :
    's' 'o' { $$ = 0; }
  | 'm' 'o' { $$ = 1; }
  | 'd' 'i' { $$ = 2; }
  | 'm' 'i' { $$ = 3; }
  | 'd' 'o' { $$ = 4; }
  | 'f' 'r' { $$ = 5; }
  | 's' 'a' { $$ = 6; }
;
%%

void *alloca(size_t bytes)
{
	fprintf(stderr, "Panic: No alloca(%p) call!\nAborting ...\n", bytes);
	exit(255);
	return NULL;
}

int yylex(void)
{	int c;

redo:
	c = getc(file);
	if (c == ' ' || c == '\t')
		goto redo;
	if (c == '%')
	{
		fgets(buf, sizeof(buf), file);
		line++;
		goto redo;
	}
	if (c == '\n')
		line++;
	if (c == '"')
	{	char *d = buf;
		size_t l = sizeof(buf)-2;

		while((c=getc(file)) != '"')
		{
			if (c == EOF)
			{
				yyerror("Error: EOF in String found");
			}
			if (c == '\\')
			{
				c = getc(file);
				if (c != '"' && c!= '\\')
				{
					ungetc(c, file);
					c = '\\';
				}
			}
			if (l)
			{
				*d++ = c;
				l--;
			}
			if (c == '\n')
				line++;
		}
		*d =0;
		yylval.str=buf;
		if (!l)
			fprintf(stderr, "%d: Warning, long \"...\". Rest ignored.\n", line);
		return COMMAND;
	}
	if (isdigit(c))
	{
		ungetc(c, file);
		fscanf(file, "%d", &yylval.val);
		return DISPLACEMENT;
	}
	if (c == EOF)
		return 0;
	if (isalpha(c))
		return tolower(c);
	return c;
}

