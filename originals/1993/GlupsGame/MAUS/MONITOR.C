#include <stdio.h>
#include <io.h>
#include <conio.h>

#define MAX 40000ul

unsigned char a[MAX+1];
char name[40];

void schreibe(int i)
{
  int j,k;
  k=16*i;
  printf("%04x  ",k);
  for (j=k;j<k+16;j++)
    { printf("%02x",a[j]);
      if (j%2==1) printf(" ");
    }
  printf("    ");
  for (j=k;j<k+16;j++)
    { if (isalnum(a[j])) printf("%c",a[j]);
      else printf(".");
    }
}


main()
{ FILE *daten;
  int c,extended;
  unsigned int u;
  int i,j,k;
  int zeile;

  clrscr();
  printf("Dateiname eingeben: ");
  scanf("%s",name);
  if ((daten=fopen(name,"rb"))==NULL)
    { fprintf(stderr,"Datei nicht gefunden."); return; }
  u=0;
  while (!feof(daten)&&u<MAX+1)
    { a[u]=fgetc(daten); u++; }
  fclose(daten);
  clrscr();
  for (i=0; i<25; i++) { schreibe(i); if (i<24) printf("\n"); }
  gotoxy(7,1); zeile=0;
  while((c=getch())!='x')
    { if (!c)
      { i=wherex(); j=wherey();
	switch(getch())
	{  case 72: if(zeile==0) break;
		    zeile--;
		    if(j>1) gotoxy(i,j-1);
		    else { insline();
			   gotoxy(1,1);
			   schreibe(zeile);
			   gotoxy(i,1);
			 }
		    break;
	   case 80: if(zeile>= MAX/16) break;
		    zeile++;
		    if (j<25) gotoxy(i,j+1);
		    else { gotoxy(1,1);
			   delline();
			   gotoxy(1,25);
			   schreibe(zeile);
			   gotoxy(i,25);
			 }
		    break;
	   case 75: if (i==7) break;
		    if (i%5==2) gotoxy(i-2,j);
			   else gotoxy(i-1,j);
		    break;
	   case 77: if (i==45) break;
		    if (i%5) gotoxy(i+1,j);
			else gotoxy(i+2,j);
		    break;

	}
      }
      else
      { if (!isxdigit(c)) break;
	i=wherex(); j=wherey();
	k=j-7-(j-7)/5;
	a[zeile*16+k]=c;
	if (isalnum(c))
	  { gotoxy(i,47+k); printf("%c",c); gotoxy(i,j); }

      }
    }

  return;
}

