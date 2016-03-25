#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
        int hh, mm, h, m, tt, t, s;
        struct tm *now;
        time_t tnow;

        if (argc != 3)
        {
                fprintf(stderr, "usage: %s HH MM\n\tStop execution at HH:MM-HH:MM+4 (incl). 0<=HH<=23.\n", argv[0]);
                fprintf(stderr, "\t(c) 1996 Singhammer Datentechnik, M. Lercher\n");
                fprintf(stderr, "\tCompiled "__DATE__" "__TIME__"\n");
                return 99;
        }
        hh = atoi(argv[1]);
        mm = atoi(argv[2]);
        tt = hh*60+mm;

        while(1)
        {
                tnow = time(NULL);
                now = localtime(&tnow);
                h = now->tm_hour;
                m = now->tm_min;
                s = now->tm_sec;
                t = h*60+m;
                printf("%02d:%02d\r", h, m);
                if (t >= tt && t < tt+5)
                        break;
                sleep(61-s);
        }
        printf("\n");
        return 0;
}

