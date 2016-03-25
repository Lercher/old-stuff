/* ersatz.c - Ersetze strings in Dateien.c
%%install=L:\ersatz
%%save=L:\ersatz
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 500
#define BUF 255

char buf1[BUF], buf2[BUF];
char *from[MAX], *to[MAX];
int flen[MAX], tlen[MAX];
static subs, debug=0;
FILE *in, *out;

char *xstrdup(char *s)
{
        char *d;
	int l;

	l = strlen(s);
	if (l>0 && s[l-1] == '\n')
		s[l-1] = 0;
	d = strdup(s);
	if (!d)
	{
		fprintf(stderr, "Speichermangel. Programm abgebrochen.\n");
		exit(2);
	}
	return d;
}

void sub(int i)
{
        char *s, *d, *q;
	int l;
	
	s = buf1;
	d = buf2;
	for(;;)
	{
		q = strstr(s, from[i]);
		if (!q)
			break;
		l = (int) (q-s);
		strncpy(d, s, l);
		d += l;
		strncpy(d, to[i], tlen[i]);
		d += tlen[i];
		s = q + flen[i];
	}
	strcpy(d, s);
	strcpy(buf1, buf2);
}


void ersetze(void)
{
        static long f_pos = 0;
        int i, len;

        while(fgets(buf1, sizeof(buf1)-1, in))
	{
                len = ftell(in) - f_pos;
                f_pos += len;
                for(i=0; i<len; i++)      /* handle binary NULLs */
                {
                        if (buf1[i] == '\0')
                                buf1[i] = '\777';  /* als -1, 255 */
                }
                buf1[sizeof(buf1)-1] = '\0';     /* sicher ist sicher */
                for(i=0; i<subs; i++)        /* Jede Ersetzung */
                        sub(i);              /* durchfuehren   */
                for(i=0; buf1[i]; i++)
                {
                        if (buf1[i] == '\777')
                                buf1[i] = '\0'; /* Nuller zurueck */
                        fputc(buf1[i], out);    /* auch die Nullen ausgeben */
                }
	}
}

int main(int argc, char **argv)
{
	int i, j;
	FILE *f;

        if (argc != 4  &&  argc != 5)
	{
                printf("This is "__FILE__" V1.1\n(c) 1995 by M. Lercher/Singhammer Datentechnik GmbH. Compiled "__DATE__"\n");
                printf("usage: %s [-d] configfile infile outfile\n", *argv);
                printf("\tThis program converts strings to other strings.\n");
		printf("\t-d Debug. Show Conversation Table in file configfile.\n");
		printf("\tconfigfile consists of pairs of lines, the first of witch\n");
		printf("\tcontains the string to be substituted by the second one.\n");
                printf("constraints:\n");
                printf("\tLinelength less than %d characters.\n", BUF);
                printf("\tConversation table less then %d entries.\n", MAX);
                printf("\tText must not contain DEL (ASCII 255) characters. It may\n");
                printf("\tcontain NUL (ASCII 0) characters.\n");
		return 10;
	}
	if (strcmp(argv[1], "-d")==0)
	{
		argv++;
		debug = 1;
	}
	f = fopen(argv[1], "r");
	if (!f)
	{
		perror(argv[1]);
		return 1;
	}
	i = 0;
	j = 0;
	while(fgets(buf1, sizeof(buf1)-1, f) &&	fgets(buf2, sizeof(buf2)-1, f))
	{
		from[i] = xstrdup(buf1);
		to[i] = xstrdup(buf2);
		flen[i] = strlen(buf1);
		tlen[i] = strlen(buf2);
		j++;
		if (flen[i])
			i++;
		else
			fprintf(stderr, "Warnung: Leerstring in Zeile %d ignoriert.\n", 2*j+1);
		if(debug)
			fprintf(stderr, "'%s' -> '%s'\n", buf1, buf2);
		if (i==MAX)
		{
			fprintf(stderr, "Fehler: zu viele Ersetzungspaare. Programm abgebrochen.\n");
			return 3;
		}
	}
	subs = i;

        in = fopen(argv[2], "rb");
        if (!in)
        {
                perror(argv[2]);
                return 2;
        }
        out = fopen(argv[3], "wb");
        if (!out)
        {
                perror(argv[3]);
                return 3;
        }
        ersetze();
	return 0;
}
