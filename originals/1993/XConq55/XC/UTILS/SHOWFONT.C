#include <stdio.h>
#include <stdlib.h>
#include <linea.h>

#define N 16

void show(short ch, short w, short h, unsigned char *bytes)
{	int ww, x, y, i, j;
	
	ww = (w+7)>>3;
	
	y = ch / N;
	x = ch % N;
	
	x *= 640/N;
	y *= 400/N;
	
	for(i=0; i<h; i++)
		for(j=0; j<w; j++)
			put_pixel(j+x, i+y, 
			  (
			   bytes[i*ww+(j>>3)] 
			   &
			   (1<<(7-(j&7)))
			  ) 
			  ==
			  ( 1<<(7-(j&7)))
			); 
}

void doit(FILE *f)
{	short ch, w, h, ww;
	
	fseek(f, 2*sizeof(short), SEEK_SET); /* skip fw, fh */
	while(!feof(f))
	{	unsigned char *bytes;
		
		fread(&ch, sizeof(short), 1, f);
		fread(&w,  sizeof(short), 1, f);
		fread(&h,  sizeof(short), 1, f);
		ww = (w+7)>>3;
		bytes = malloc(h*ww);
		fread(bytes, h, ww, f);
		show(ch, w, h, bytes);
		free(bytes);
	}
}

int main(int argc, char **argv)
{	FILE *f;

	printf("\033E\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	linea_init();
	if (argc>1)
	{
		f = fopen(argv[1], "rb");
		if (!f)
			perror(argv[1]);
		else
		{
			doit(f);
			fclose(f);
		}
	}
	return 0;
}
