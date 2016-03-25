#include <stdio.h>
#include <stdlib.h>


enum dir { LEFT, RIGHT };

static dx, dy; /* innitial direction */
static FILE *f;


void turn(enum dir dir)
{  int hilf;

	if (dir == LEFT)
	{
		fputc('L', stderr);
		hilf = dx;
		dx = -dy;
		dy =  hilf;
		fprintf(f, "AR%d,%d,90;", dx, dy);
	}
	else
	{
		fputc('R', stderr);
		hilf = dx;
		dx =  dy;
		dy = -hilf;
		fprintf(f, "AR%d,%d,-90;", dx, dy);
	}
}


void dragon(int n);
void nogard(int n);

void dragon(int n)
{
	if(n)
	{
		dragon(n-1);
		turn(LEFT);
		nogard(n-1);
	}
}


void nogard(int n)
{
	if(n)
	{
		dragon(n-1);
		turn(RIGHT);
		nogard(n-1);
	}
}

void init(int n)
{
	fprintf(f,
		"IN;SP1;PW0.05;\n"
		"PU100,300;"
		"LB%d Drachen\003\n"
		"PU1000,2200;PD;\n",
		n);
	dx = 0; dy = -40;
}


void quit(void)
{
	fprintf(f, "\nPE;IN;" );
	fflush(f);
}


int main(int argc, char *argv[])
{	int n;

	f = stdout;
	fprintf(stderr, "This program prints the Dragoncurve in HP-GL to stdout.\n");
	fprintf(stderr, "author: Dipl. Math. Martin Lercher. Compiled "__DATE__".\n");
	if (argc<2)
	{
		fprintf(stderr, "usage: %s #n, where #n means the recursion depth<14.\n",
			argv[0]);
		return 1;
	}
	n = atoi(argv[1]);
	if (n<=0 || n>13)
	{
		fprintf(stderr, "%s: illegal recursion deep.\n",
			argv[0]);
		return 2;
	}
	init(n);
	dragon(n);
	quit();
	return 0;
}
