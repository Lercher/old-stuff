enum { X=55, Y=63, W=X+4 };
int  ISIZE;
int  LOOPPAL;
typedef void *IMAGES[10];

void load(char *filename, IMAGES image)
{	FILE *f;
	int i;

	ISIZE = imagesize(0,0,X,Y);

	f = fopen(filename, "rb");
	if (f)
	{
		for(i=0; i<10; i++)
		{
			image[i] = malloc(ISIZE);
			fread(image[i], ISIZE, 1, f);
		}
		fclose(f);
	}
	else
	{
		for(i=0; i<10; i++)
			image[i] = NULL;
	}
}

void mypalette(void)
{   int i;
	struct palettetype pal;

	getpalette(&pal);
	setrgbpalette(pal.colors[0], 63, 63, 63); //hintergrund weiss
	setrgbpalette(pal.colors[1],  0,  0,  0); //Schwarz
	for(i=2; i<13; i++)
		setrgbpalette(pal.colors[i], 24+3*i, 22+3*i, 22+3*i);
	setrgbpalette(pal.colors[13], 63, 0, 0); //rot
	setrgbpalette(pal.colors[14],  0,63, 0); //gruen
	setrgbpalette(pal.colors[15], 20,63,20); //gruen->blau->rot
	LOOPPAL = pal.colors[15];
}

