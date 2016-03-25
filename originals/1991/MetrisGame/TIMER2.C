#include <conio.h>
#include "times.c"

static clock_t x[100];

main()
{       unsigned i;

	for (i=0; i<100 ; i++)
	{
		x[i] = clock();
		cprintf("%9lx ", x[i]);
	}
	return 0;
}