#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static error = 0;

void usage(void)
{
        printf("usage:\tsum { + | - | file [...]}\n");
        printf("\tfile - means: read filenames from stdin.\n");
        printf("\tfile + means: read output of previous 'sum -' run\n");
        printf("\tand compare the results. Print filename on mismatch. Return 1.\n");
        printf("examples:\n");
        printf("\tsum sum.exe sum.c\n");
        printf("\tdir *.exe /a-d/s/l/b | sum -\n");
        exit(255);
}

void sum(const char *fn, unsigned long test)
{       FILE *f;
        unsigned long l;
        unsigned int i;

        f = fopen(fn, "rb");
        if (!f)
        {
                perror(fn);
                return;
        }
        l = 1;
        while(!feof(f))
        {
                i = getc(f);
                if (l & 0x40000000ul) /* 31 bit periode */
                {        
                        l <<= 1;
                        l++;
                }
                else
                        l <<= 1;
                l ^= i;
        }
        fclose(f);
        if (test==0)
                printf("%s %08lX\n", fn, l);
        else if (test != l)
        {        
                printf("%s\n", fn);
                error = 1;
        }
}

int main(int argc, char **argv)
{       unsigned int j;
        unsigned long l;
        char *sc;
        char buf[256];

        if (argc < 2)
                usage();
        for(j = 1; j < argc; j++)
        {
                sc = NULL;
                if (0 == strcmp(argv[j], "-"))
                        sc = "%s\n";
                else if (0 == strcmp(argv[j], "+"))
                        sc = "%s%lx\n";
                if (sc) 
                {      
                        while(!feof(stdin))
                        {        
                                l = 0;
                                fscanf(stdin, sc, buf, &l);
                                sum(buf, l);
                        }
                }
                else
                        sum(argv[j], 0);
        }
        return error;
}
