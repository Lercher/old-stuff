#include <dos.h>
#include "maus/mouse.h"

int ms_getpos(int* x,int*y)
{  struct REGPACK reg;
  reg.r_ax=3; intr(0x33,&reg);
  *x=reg.r_cx; *y=reg.r_dx; return(reg.r_bx);
}

void ms_do(enum mousestatus s)
{ struct REGPACK reg;
  reg.r_ax=(int)s; intr(0x33,&reg);
}

int ms_poll(int* x,int* y,int(*abbruch)(void))
{ int f;
  static altf=-1,altx,alty;
  do f=ms_getpos(x,y);
  while((altx==*x)&&(alty==*y)&&(altf==f)&&(!(*abbruch)()));
  altf=f; altx=*x; alty=*y; return(f);
}