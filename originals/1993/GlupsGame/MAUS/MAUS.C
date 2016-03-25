#include <stdio.h>
#include <conio.h>
#include <graphics.h>
#include "mouse.h"

int gdriver=DETECT,gmode;
int x,y,f;

main()
{ initgraph(&gdriver,&gmode,"");
  ms_do(INIT); ms_do(SHOW);
  while(1)
  { f=ms_poll(&x,&y,kbhit);
    if(kbhit()) break;
    if(f&1) { ms_do(HIDE); putpixel(x,y,YELLOW); ms_do(SHOW); }
    if(f&2) { ms_do(HIDE); putpixel(x,y,CYAN);   ms_do(SHOW); }
  }
  closegraph(); return;
}