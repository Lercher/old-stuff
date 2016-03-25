#include "version.h"
char SCCS_ID[]="@(#)"__FILE__" V"VERSION" \tCompiled "__DATE__" "__TIME__"\n";

/* eine kleine Shell fÅr MSDOS Benutzer.
 * File: test.c
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
#include <dir.h>
#include <dos.h>

int cc;
char **vv;

char *next(void)
{
	if (cc-->0)
    	return *vv++;
    return "";
}

int eval(void)
{	int a,b;
	char *ca, *cb;
    char *s, buf[7];
    struct ffblk ff;

    s = next();
    if (*s=='-' && s[1])
    {
    	switch(s[1])
        {
        case 'v':
        	a = eval();
        	puts(itoa(a, buf, 10));
            return a;
        case 'V':
        	puts(SCCS_ID+4);
        	return 0;
        case '!':
        	a = eval();
            return !a;
        case 'A':
        case '&':
        	a = eval();
            b = eval();
        	return a && b;
        case 'O':
        case '|':
        	a = eval();
            b = eval();
        	return a || b;
        case 'e':
        	ca = next();
            cb = next();
            return !strcmp(ca, cb);
        case '=':
        	a = eval();
            b = eval();
            return a==b;
        case 'L':
        case '<':
        	a = eval();
            b = eval();
            return a<b;
        case 'G':
        case '>':
        	a = eval();
            b = eval();
            return a>b;
        case 'l':
        	ca = next();
            cb = next();
            return strcmp(ca, cb)<0;
        case 'g':
        	ca = next();
            cb = next();
            return strcmp(ca, cb)>0;
        case '+':
        	a = eval();
            b = eval();
            return a+b;
        case '-':
        	a = eval();
            b = eval();
            return a-b;
        case '*':
        	a = eval();
            b = eval();
            return a*b;
        case '/':
        	a = eval();
            b = eval();
            return b ? a/b : 0;
        case '%':
        	a = eval();
            b = eval();
            return b ? a%b : 0;
        case 'd':
        	ca = next();
            a = findfirst(ca, &ff, FA_DIREC);
            if (!a)
            	return FA_DIREC == (ff.ff_attrib & FA_DIREC);
            return 0;
        case 'f':
        	ca = next();
            return !findfirst(ca, &ff, 0);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        	/* negative Zahl */
        	break;
        default:
        	puts("unknown operator.");
            return 0;
        }
    }
    return atoi(s);
}


int main(int argc, char **argv)
{

	cc = argc-1;
    vv = argv+1;

	return eval();
}
