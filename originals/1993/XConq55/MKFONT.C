/* mkfont.c - generiere eine fontdatei aus *.bdf Dateien. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static char name[20], buf[1024];

void error(const char *s)
{
	fprintf(stderr, "\n%s. Aborting...", s);
	exit(1);
}

void scan(FILE *f, const char *tofind)
{
	assert(tofind && *tofind);
	do {
		if (!fgets(buf, (int)sizeof(buf), f))
			error("Premature EOF");
	} while(strncmp(buf, tofind, strlen(tofind)-1));
}

void convert(FILE *s, FILE *d)
{	int ch;
	short int w, h, ww;
	short int i, j;
	unsigned byte, wx, hx;
	unsigned char *bytes;
	
	scan(s, "FONT ");
	printf("%s", buf);
	scan(s, "FONTBOUNDINGBOX");
	sscanf(buf, "%*s%d%d", &wx, &hx);
	w = (short) wx;
	h = (short) hx;
	fwrite(&w,  sizeof(w), 1, d);
	fwrite(&h,  sizeof(h), 1, d);
	
	do {	
		scan(s, "ENCODING");  sscanf(buf, "%*s%d", &ch);
		scan(s, "BBX");       sscanf(buf, "%*s%d%d", &wx, &hx);
		w = (short) wx;
		h = (short) hx;
		printf("\r%c [%3d] width %2d  height %2d", isprint(ch) ? (char)ch : '.', ch, (int)w, (int)h);
		scan(s, "BITMAP");
		ww = (w+7)>>3;
	
		bytes = (unsigned char *) calloc(ww,sizeof(unsigned char));
		if (!bytes)
			error("out of memory");

		fwrite(&ch, sizeof(ch), 1, d);
		fwrite(&w,  sizeof(w), 1, d);
		fwrite(&h,  sizeof(h), 1, d);

		for(i=0; i<h; i++)
		{
			for(j=0; j<ww; j++)
			{
				if (1 != fscanf(s, "%2x", &byte))
					error("Error scanning BITMAP");
				bytes[j] = (unsigned char) byte;
			}
			if (ww != fwrite(bytes, sizeof(char), ww, d))
			{
				perror("\nin writing");
				exit(1);	
			}
		}
		free(bytes);
		
		scan(s, "ENDCHAR");
		if (!fgets(buf, (int)sizeof(buf), s))
			error("no ENDFONT");
	} while(strncmp(buf, "ENDFONT", sizeof("ENDFONT"-1)));
	printf("\n");
}


int main(int argc, char **argv)
{	FILE *f, *bin;
	int err = 0;
	char *s, *s1;
		
	printf("This is mkfont 1.0 - ");
	
	if (argc != 3)
	{
		printf("usage:  mkfont <font.bdf> <binfile>\n");
		printf("  converts <font.bdf> (X11) to binary data in <binfile>.\n");
		return 1;
	}
	f = fopen(argv[1], "r");
	if (!f)
	{
		perror(argv[1]);
		err = 1;
	}
	bin = fopen(argv[2], "wb");
	if (!bin)
	{
		perror(argv[2]);
		err = 1;
	}
	if (err)
		return 1;
	s1 = strrchr(argv[1], ':');
	s1 = s1 ? s1 + 1 : argv[1];
	s  = strrchr(s1, '\\');
	strncpy(name, s ? s : s1, sizeof(name));
	convert(f, bin);
	if (fclose(bin))
	{
		perror(argv[2]);
		err = 1;
	}
	if (fclose(f))
	{
		perror(argv[1]);
		err = 1;
	}	
	return err;
}
