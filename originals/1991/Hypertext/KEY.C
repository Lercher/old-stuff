#include <conio.h>

int main(void)
{	int c;

	do {
		c = getch();
		putch(c);
	} while (c != 'q');
	return 0;
}

