/* bitmap.c - generiere eine bitmapdatei aus *.b Dateien. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char name[20];

void error(const char *s)
{
	fprintf(stderr, "error %s.\nAborting...", s);
	exit(1);
}

int gs(FILE *f)
{	int ch;

	ch = fgetc(f);
	if (ch == EOF)
		error("EOF");
	return ch;
}

void convert(FILE *s, FILE *d)
{	int ww, hh;
	short w, h;
	short size, i;
	unsigned char *bytes;
	size_t len;
	
	if (1 != fscanf(s, "%*s%*s%d\n", &ww)) error("in line 1");
	if (1 != fscanf(s, "%*s%*s%d\n", &hh)) error("in line 2");
	w = (short) ww;
	h = (short) hh;
	size = ((w+7)>>3) * h;
	bytes = (unsigned char *)calloc(size, sizeof(unsigned char));
	if (bytes == NULL)
		error("out of memory");
	printf("File %s contains %dx%d image consuming %d bytes.\n", name, (int)w, (int)h, (int)size);
	while(gs(s) != '{')
		/**/;
	for(i=0; i<size; i++)
	{	unsigned buf;
	
		while(gs(s) != 'x')
			/**/;
		if (1 != fscanf(s, "%2x", &buf))
			error("in scanning bytes");
		bytes[i] = (unsigned char) buf;
	}
	len =  fwrite(name,  sizeof(char), sizeof(name), d);
	len += fwrite(&w,    sizeof(w), 1, d);
	len += fwrite(&h,    sizeof(h), 1, d);
	len += fwrite(bytes, sizeof(unsigned char), size, d);
	if (len != sizeof(name) + 2 + size)
	{
		perror("while writing file");
		exit(1);
	}
}


int main(int argc, char **argv)
{	FILE *f, *bin;
	int err = 0;
	char *s, *s1;
		
	printf("This is bitmap 1.0 - ");
	
	if (argc != 3)
	{
		printf("usage:  bitmap <bitmap.b> <binfile>\n");
		printf("  converts <bitmap.b> (X11) to binary data and append it to <binfile>.\n");
		printf("format: [20 byte filename][2 byte width][2 byte height][data] repeated.\n");
		printf("  [data] has length h*(w>>3) bytes.\n");
		return 1;
	}
	f = fopen(argv[1], "r");
	if (!f)
	{
		perror(argv[1]);
		err = 1;
	}
	bin = fopen(argv[2], "ab");
	if (!bin)
	{
		perror(argv[2]);
		err = 1;
	}
	if (err)
		return 1;
	s1 = strrchr(argv[1], ':');
	s1 = s1 ? s1+1 : argv[1];
	s  = strrchr(s1, '\\');
	strncpy(name, s ? s+1 : s1, sizeof(name));
	for(s = name; *s; s++)
		*s = tolower(*s);
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
