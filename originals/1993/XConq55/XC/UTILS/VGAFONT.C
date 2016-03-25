/* generiere einen VGA font */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <graph.h>

short w=8, h=8;

void convert(FILE *d)
{	unsigned short i, byte, bit;
	int x, y;

	fwrite(&w,  sizeof(w), 1, d);
	fwrite(&h,  sizeof(h), 1, d);

	assert(w%8 == 0);
	
	for(i=0; i<255; i++)
	{
		fwrite(&i, sizeof(i), 1, d);
		fwrite(&w, sizeof(w), 1, d);
		fwrite(&h, sizeof(h), 1, d);

		printf("\033[1;1H \b%c", i);


		for(y = 0; y<h; y++)
		{
			bit = 1;
			for(x = 0; x<w; x++)
			{
				if (bit == 1)
					byte = 0;
				if (g_get(x,y))
					byte |= bit;
				bit <<= 1;
				if (bit == 0x100)
				{	unsigned char c;
					c = byte;
					
					fwrite(&c, sizeof(c), 1, d);
					bit = 1;
				}
			}
		}
	}
}

int main(int argc, char **argv)
{	FILE *f, *bin;
	int err = 0;
	char *s, *s1;
		
	printf("This is vgafont 1.1 - ");
	
	if (argc != 5)
	{
		printf("usage:  vgafont <VESAmode(dez)> <w> <h> <binfile>\n");
		printf("  converts screenfont to binary data in <binfile>.\n");
		return 1;
	}
	w = atoi(argv[2]);
	h = atoi(argv[3]);
	bin = fopen(argv[4], "wb");
	if (!bin)
	{
		perror(argv[4]);
		return 1;
	}
	if (!g_mode(atoi(argv[1])))
	{
		printf("Cannot open Graphics mode!\n");
		unlink(argv[4]);
		return 1;
	}
	convert(bin);
	g_mode(GTEXT);
	if (fclose(bin))
	{
		perror(argv[4]);
		err = 1;
	}
	return err;
}

