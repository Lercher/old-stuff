#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum hoptop { EIN, AUS };

struct prj {
	int prjnr;
	time_t time;
	enum hoptop status;
};

#define FILENAME "project.log"
static FILE *f;
static char *name;

int last(void)
{   struct prj p;

	if(fseek(f, -sizeof(struct prj),SEEK_END))
		return 0;
	if (1 != fread(&p, sizeof(struct prj), 1, f))
		return 0;
	if (p.status == EIN)
		return p.prjnr;
	return 0;
}

void stop(int nr, time_t t)
{   struct prj p;

	p.prjnr = nr;
	p.time = t;
	p.status = AUS;

	if (1 != fwrite(&p, sizeof(struct prj), 1, f))
	{
		perror(name);
		exit(3);
	}
}

void start(int nr, time_t t)
{   struct prj p;

	p.prjnr = nr;
	p.time = t;
	p.status = EIN;

	if (1 != fwrite(&p, sizeof(struct prj), 1, f))
	{
		perror(name);
		exit(3);
	}
}

void report(int nr, time_t t)
{   struct prj p;
	enum hoptop ls;
	time_t summe, tim;

	summe = 0;
	rewind(f);
	while(fread(&p, sizeof(struct prj), 1, f))
	{
		if (nr == p.prjnr)
		{
			ls = p.status;
			if (ls == EIN)
				tim = p.time;
			if (ls == AUS)
				summe += p.time - tim;
		}
	}
	if (ls == EIN)
	{
		printf("currently working on projekt %d ", nr);
		summe += t - tim;
	}
	printf("Total: %lds = %ldh %ldmin %lds\n",
		summe, summe/3600, (summe%3600)/60, summe%60);
}

int main(int argc, char **argv)
{   int nr, oldnr;
	time_t t;

	if (argc<2)
	{
		printf("usage: %s <NR>\n\tWork on project <NR>\n", *argv);
		printf("\t0 stop working.\n\tNegative <NR>s generate Reports.\n");
		return 255;
	}
	name = getenv("PROJECTLOG");
	if (!name)
		name = FILENAME;

	t = time(NULL);

	if (argc>2)
		t = atol(argv[2]); /* just for test */

	f = fopen(name, "rb");
	if (!f)
	{
		f = fopen(name, "wb");
		if (!fclose(f))
		{
			perror(name);
			return 4;
		}
	}
	else
		fclose(f);

	f = fopen(name, "a+b");
	if (!f)
	{
		perror(name);
		return(1);
	}

	nr = atoi(argv[1]);
	if (nr>0)
	{
		oldnr = last();
		if (oldnr)
			stop(oldnr, t);
		if (nr)
			start(nr, t);

		if (!fclose(f))
		{
			perror(name);
			return 2;
		}
	}
	else
		report(nr, t);

	return 0;
}