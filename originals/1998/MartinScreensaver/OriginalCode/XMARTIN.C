#define PATCHLEVEL 0
#define TOS 0
/*
 *   xmartin - set root window to Martin hopalong pattern
 *
 *   xmartin [-f {martin1|martin2|ejk1|...|ejk6|rr1|cp1[,order[,xn,yn]]]}
 *           [-p npoints] [-P npoints] 
 *           [-a value[:[:]value]] [-b value[:[:]value]] [-c value[:[:]value]]
 *           [-d value[:[:]value]] [-x value[:[:]value]] [-y value[:[:]value]]
 *           [-nc npoints] [-dynam [npoints]] [-static]
 *           [-tile [XxY]] [-perturb [n[,v]]] [-coord {xy|yx|ra|ar}]
 *           [-zoom real] [-move d,p]
 *           [-recall] [-display display]
 *           [+rv] [-rv] [-bg [color[,intensity]]]
 *           [-v]
 *
 *   -f function : martin1|martin2|ejk1|...|ejk6|rr1|cp1
 *   -p npoints  : maximum in-range points   [25% of display/tile pixels]
 *   -P npoints  : maximum total points to calculate   [3 x -p value]
 *   -a -b -c -d -x -y  : real values for hopalong parameters   [random]
 *   -dynam [nd] : flush after nd points   [1024, 128 if -tile]
 *   -static     : calculate all points before display
 *   -tile [XxY] : tiling factors   [random if XxY omitted]
 *   -perturb [n,v] : perturb (x,y) every n points by v  [random if n,v omitted]
 *   -coord xx   : coordinate mode - xy | yx | ra | ar
 *   -zoom real  : zoom factor   [1.0, 4.0 if martin2]
 *   -move d,p   : moves pattern p pixels in direction d. d is a compass
 *                 heading in degrees or 'n', 'sw', 'nnw' etc
 *   -recall     : recalls f, abcdxy, zoom, move, tile & perturb values from 
 *                 last plot before processing other arguments
 *   -nrc        : turns off randomizing of color sequences
 *   -nc npoints : points calculated before color change   [(-P value)/ncolors]
 *   -mono       : forces white-on-black for grayscale or color displays
 *   -rv +rv     : reverse video on/off  [-rv (on)]
 *   -bg color[,intensity] : background color  [random]
 *   -v          : print version & patchlevel
 *
 *   Hopalong was attributed to Barry Martin of Aston University (Birmingham, 
 *   England) by A. K. Dewdney in the Septmber 86 Scientific American. 
 * 
 *   The cp1 sine sculptures were published by Clifford Pickover in the 
 *   January 91 Algorithm. The rr1 generalized exponent version of the 
 *   martin1 square root was developed by Renaldo Recuerdo of the Santa Cruz
 *   Operation.
 ^
 *   The ranf random number generator is based on work by D. H. Lehmer, Knuth,
 *   David Sachs(Fermilab) and Curt Canada(NCSA).
 *
 *   This software is provided "as is" and with no warranties of any kind.
 *
 *   Ed Kubaitis
 *   Computing Services Office
 *   University of Illinois, Urbana
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#if TOS
# include <ext.h>
# include <fastdraw.h>	/* my own one */
#else
# include <conio.h>
# include <graph.h>	/* MS-C 6.00 */
#endif


#define Min(x,y) ((x < y)?x:y)
#define Max(x,y) ((x > y)?x:y)
#define Fmod1(p) ((p) - (long)(p))

#define Msg(fmt) fprintf(stderr,fmt)
#define Msg1(fmt,a) fprintf(stderr,fmt,a)
#define Msg2(fmt,a,b) fprintf(stderr,fmt,a,b)
#define Msg3(fmt,a,b,c) fprintf(stderr,fmt,a,b,c)
#define Msg4(fmt,a,b,c,d) fprintf(stderr,fmt,a,b,c,d)

static long	mxp=0, np=0;	/* max in-range points, in-range points    */
static long	mxP=0, nP=0;	/* max total points,    total points       */
static long	nC=0, nc=0;		/* color-change interval (points)          */
static long	Pn=0, pn=0;		/* seed perturbation interval (points)     */
static double	Pv=0;			/* seed perturbation value                 */

static double  A=0, B=0, C=0, D=0, x=0, y=0; /* hopalong parameters                */
static int     Color=0;		/* non-zero if color                       */
static int		Ncolor;
static int		Randomcolor=1;	/* non-zero for randomized color sequences */
static int     Coord=0;		/* coordinate mode 0:xy 1:yx 2:ra 3:ar     */
static int     Tx=1, Ty=1;		/* tile factors                            */
static double  Zf=0;			/* magnification factor                    */
static int     Recall=0;		/* non-zero if previous parms recalled     */
static int     CpOrder = 0;	/* Cp1 highest-order term (1->16)          */
static unsigned long CpXOrdinal;	/* Cp1 x permutation ordinal (<4**order)   */
static unsigned long CpYOrdinal;	/* Cp1 y permutation ordinal (<4**order)   */


static int cx, cy, ix, iy, mxX, mxY, col = 0;
static double pi, X0, Y0;

static int Function = -1;
#define Martin1 0
#define Martin2 1
#define Ejk1    2
#define Ejk2    3
#define Ejk3    4
#define Ejk4    5
#define Ejk5    6
#define Ejk6    7
#define Rr1     8
#define Cp1     9
#define Nfunc  10

static char fname[Nfunc][16] = { 
   "martin1", "martin2", 
   "ejk1", "ejk2", "ejk3", "ejk4", "ejk5", "ejk6",
   "rr1" , "cp1"
   };
static double fprob[Nfunc] = {
   .32,       .02,       
   .08,    .03,    .06,    .06,    .06,    .06, 
   .06,    .25
   };

static char compass[16][4] = {
   "n", "nne", "ne", "ene", "e", "ese", "se", "sse",
   "s", "ssw", "sw", "wsw", "w", "wnw", "nw", "nnw" };

static double cp_zoom[16] = {
   200, 200, 175, 175, 150, 150, 125, 125, 
   125, 100, 100,  80,  70,  60,  60,  60};
static char   cp_move[16][8] = {
   "310,220", "310,220", "310,220", "310,220", "310,220", "310,320",
   "310,320", "310,320", "310,320", "310,320", "310,320", "310,320",
   "310,320", "310,320", "310,320", "310,320"};

static double sine[4], xt, yt;
#define Ax 0
#define Ay 1
#define Bx 2
#define By 3
static int xi, yi, order, n;
static unsigned long xo, yo;

#define XY 0
#define YX 1
#define RA 2
#define AR 3
static char coords[4][3] = { "xy", "yx", "ra", "ar" };

static int     W, H, DP;   

static char    Savefile[128]; FILE *sf;
static char    *Move = NULL;
static char    **Argv = NULL; int Argc = 0;
static time_t  t0;

static void hopalong(void);
static void preset(int, char **);
static void finish(void);
static void pr_Argv(char *s);
static void pr_intArg(char *, long *);
static void pr_rangeArg(char *, double *);
static void usage(void);
static double ranf(void);

void cdecl main(int argc, char *argv[])
{
   preset(argc, argv);
   t0 = time(0);
   hopalong();
   finish();
   exit(0);
}

struct pair {
    double x1, y1;
};

/* Hopfunctions following */
typedef void hopfunc(struct pair *pos);

static void f_Martin1(struct pair *pos)
{
  pos->x1 = y  - ( (x<0) ? sqrt(fabs(B*x-C)) : -sqrt(fabs(B*x-C)) );
  pos->y1 = A - x;
}
static void f_Martin2(struct pair *pos)
{
  pos->x1 = y  - sin(x);
  pos->y1 = A - x;
}
static void f_Ejk1(struct pair *pos)
{
  pos->x1 = y  - ( (x>0) ? (B*x-C) : -(B*x-C) );
  pos->y1 = A - x;
}
static void f_Ejk2(struct pair *pos)
{
 pos->x1 = y  - ( (x<0) ? log(fabs(B*x-C)) : -log(fabs(B*x-C)) );
 pos->y1 = A - x;
}
static void f_Ejk3(struct pair *pos)
{
 pos->x1 = y  - ( (x>0) ? sin(B*x) - C : -(sin(B*x) - C) );
 pos->y1 = A - x;
}
static void f_Ejk4(struct pair *pos)
{
 pos->x1 = y  - ( (x>0) ? sin(B*x) - C : -sqrt(fabs(B*x-C)) );
 pos->y1 =  A - x;
}
static void f_Ejk5(struct pair *pos)
{
 pos->x1 = y  - ( (x>0) ? sin(B*x) - C : -(B*x-C) );
 pos->y1 =  A - x;
}
static void f_Ejk6(struct pair *pos)
{
 pos->x1 = y	- asin(Fmod1(B*x));
 pos->y1 =  A - x;
}
static void f_Rr1(struct pair *pos)
{
 pos->x1 = y  - ( (x<0) ? -pow(fabs(B*x-C), D) : pow(fabs(B*x-C), D) );
 pos->y1 = A - x;
}
static void f_Cp1(struct pair *pos)
{
 double x1, y1;
 long xo, yo;
 int order, cpo, n;

 xo = CpXOrdinal;
 yo = CpYOrdinal;
 cpo = CpOrder;
 x1 = 0; y1 = 0;

 sine[Ax] = sin(A*x); sine[Ay] = sin(A*y);
 sine[Bx] = sin(B*x); sine[By] = sin(B*y);

 for (order=1; order <= cpo; order++) {
    int xi, yi;
    double xt, yt;

    xt = 1; yt = 1;
    xi = (int)xo & 3; xo >>= 2;
    yi = (int)yo & 3; yo >>= 2;
    for (n=0; n<order; n++) {
       xt *= sine[xi];
       yt *= sine[yi];
       }
    x1 += xt; y1 += yt;
    }
    pos->x1 = x1;
    pos->y1 = y1;
}

hopfunc *functions[10] = {
    f_Martin1,
    f_Martin2,
    f_Ejk1,
    f_Ejk2,
    f_Ejk3,
    f_Ejk4,
    f_Ejk5,
    f_Ejk6,
    f_Rr1,
    f_Cp1
};

static void hopalong(void) {
   double x1,y1;
   struct pair pos;
   const hopfunc *f = functions[Function];

   while (++nP < mxP) {

	if (kbhit())
		return;
      (*f)(&pos);
      x = x1 = pos.x1; y = y1 = pos.y1;

      if (Pn && ++pn > Pn) {
	 x += (x>0) ? -Pv : Pv;
	 y += (y>0) ? -Pv : Pv;
	 pn = 0;
	 }

      if (Color && ++nc > nC) {
	 col = (col+1)%Ncolor;
	 nc = 0;
	 /* XSetForeground(dpy, gc, colors[col].c_color); */
#if !TOS
	 _setcolor(col);
#endif
      }
      if (Coord == XY) {
	 iy = (int) (cy + Zf*y1);
	 ix = (int) (cx + Zf*x1);
	 }
      else if (Coord == YX) {
	 iy = (int) (cy + Zf*x1);
	 ix = (int) (cx + Zf*y1);
	 }
      else if (Coord == RA) {
	 iy = (int) (cy + Zf*x1*sin(y1));
	 ix = (int) (cx + Zf*x1*cos(y1));
	 }
      else if (Coord == AR) {
	 iy = (int) (cy + Zf*y1*sin(x1));
	 ix = (int) (cx + Zf*y1*cos(x1));
	 }

      if (iy < 0 || iy > mxY) continue;
      if (ix < 0 || ix > mxX) continue;

/*    XPutPixel(Xi, ix, iy, colors[color].c_color);	*/    /* very slow */
#if TOS
		fd_plot(ix, iy);
#else
      _setpixel(ix, iy);
#endif

      if (++np >= mxp) break;
      }
   }


static void finish(void) {

	t0 = time(0) - t0;
	putchar('\007');
	getch();
	
#if !TOS
	_setvideomode(_DEFAULTMODE);
#endif
   Msg4("xmartin: %ld points   %ld(%d%%) in-range   %ld seconds\n",
	  nP, np, (int)((100*np)/nP), t0);

   if (!Recall) {
      sf = fopen(Savefile, "w");
      if (!sf) { perror("xmartin: xmartin.dat"); exit(1); }
      fprintf(sf, "-f\n%s", fname[Function]);
      if (Function == Cp1) fprintf(sf, ",%d,%lu,%lu", CpOrder, CpXOrdinal, CpYOrdinal);
      fprintf(sf, "\n-zoom\n%.17e\n", Zf);
      if (A) fprintf(sf, "-a\n%.17e\n", A);
      if (B) fprintf(sf, "-b\n%.17e\n", B);
      if (C) fprintf(sf, "-c\n%.17e\n", C);
      if (D) fprintf(sf, "-d\n%.17e\n", D);
      if (x) fprintf(sf, "-x\n%.17e\n", X0);
      if (y) fprintf(sf, "-y\n%.17e\n", Y0);
      if (Pn) fprintf(sf, "-perturb\n%ld,%.17e\n", Pn, Pv);
      if (Tx != 1 || Ty != 1) fprintf(sf, "-tile\n%dx%d\n", Tx, Ty);
      if (Move) fprintf(sf, "-move\n%s\n", Move);
      if (Coord) fprintf(sf, "-coord\n%s\n", coords[Coord]);
      fclose(sf);
      }
   }


#define Rmod 536870911l
#define Rmul 3.99
#define Ranfset(l) (ranfseed=((long)((labs(l)%Rmod)*Rmul))%Rmod)
long    ranfseed=268435456l;

static double ranf(void) {
   ranfseed = (long) (ranfseed*Rmul)%Rmod;
   return(ranfseed/(double)Rmod);
   }


static void preset(int argc, char *argv[]) {
   char dir[4], **argv0 = argv, buf[80];

   int	i, k, mono = 0, argc0 = argc, dist = 0, Grayscale = 0;
   int  tile = 0, ordinals = 0, coord = 0;
   double degr = 0;

   Ranfset(time(0));
   for (i=0; i<32; i++) ranf(); /* jump to a more chaotic point in sequence */
   pi = 4 * atan(1.0);
   strcpy(Savefile, "xmartin.dat");
   fflush(stderr);

   /* prescan arguments for "-recall" */

   while (++argv, --argc > 0) { 
      if (!strcmp(*argv, "-recall")) {
	 sf = fopen(Savefile, "r");
	 if (!sf) { perror("xmartin: xmartin.dat"); exit(1); }
	 while(fgets(buf, 80, sf)) {
	    buf[strlen(buf)-1] = '\0';
	    pr_Argv(buf);
	    }
	 Recall++; break;
	 }
      }
   while (++argv0, --argc0 > 0) { pr_Argv(*argv0); }

#define ArgEq(a) (!strcmp(Argv[i],a))
#define Arg (++i < Argc)
#define NoArg ((i+2)>Argc || Argv[i+1][0]=='-' || Argv[i+1][0]=='+')

   /*  parse arguments */

   for (i=0; i<Argc; i++) {

      if      (ArgEq("-mono"))   { mono++; }
      else if (ArgEq("-nrc"))    { Randomcolor = 0; }
      else if (ArgEq("-recall")) {;}
      else if (ArgEq("-v"))      { Msg1("xmartin: version 2.%d\n", PATCHLEVEL);}
      else if (ArgEq("-a") && Arg)  { pr_rangeArg(Argv[i], &A); }
      else if (ArgEq("-b") && Arg)  { pr_rangeArg(Argv[i], &B); }
      else if (ArgEq("-c") && Arg)  { pr_rangeArg(Argv[i], &C); }
      else if (ArgEq("-d") && Arg)  { pr_rangeArg(Argv[i], &D); }
      else if (ArgEq("-x") && Arg)  { pr_rangeArg(Argv[i], &x); }
      else if (ArgEq("-y") && Arg)  { pr_rangeArg(Argv[i], &y); }
      else if (ArgEq("-p") && Arg)  { pr_intArg(Argv[i], &mxp); }
      else if (ArgEq("-P") && Arg)  { pr_intArg(Argv[i], &mxP); }
      else if (ArgEq("-nc") && Arg) { pr_intArg(Argv[i], &nC); }
      else if (ArgEq("-coord") && Arg) { 
	 coord++;
	 if      (ArgEq("xy"))  Coord = 0;  
	 else if (ArgEq("yx"))  Coord = 1;  
	 else if (ArgEq("ra"))  Coord = 2;  
	 else if (ArgEq("ar"))  Coord = 3;  
	 else usage();
	 }
      else if (ArgEq("-f") && Arg) {
	 Function = -1;
	 while(++Function < Nfunc){ 
	    int nc = (int)strlen(fname[Function]);
	    if (!strncmp(fname[Function], Argv[i], nc)) break;
	    }
	 if (Function == Nfunc) usage();
	 if (strlen(Argv[i]) > strlen(fname[Function])) {
	    if (Function != Cp1)
	    	usage();
        if (sscanf(Argv[i], "cp1,%d,%lu,%lu", 
		&CpOrder, &CpXOrdinal, &CpYOrdinal)==3)   { ordinals++; }
	    else if (sscanf(Argv[i], "cp1,%d", &CpOrder))               {;}
	    else usage();
	    if (CpOrder<=0) usage();
	    }
	 }
      else if (ArgEq("-move") && Arg) {
	 if (sscanf(Argv[i], "%lf,%d", &degr, &dist)==2)  {;}
	 else if (sscanf(Argv[i], "%3[nsew],%d", dir, &dist)==2) {
	    for(k=0; k<16; k++) {
	       if (!strcmp(dir, compass[k])) { degr = k * 22.5; break; }
	       }
	    if (k == 16) usage();
	    }
	 else usage();
	 Move = Argv[i];  /* save for output at end to ~/.xmartin */
	 if (degr < 0 || dist < 0) usage();
	 degr = 450. - degr;
	 while (degr > 360) degr -= 360;
	 }
      else if (ArgEq("-perturb")) {
	 Pn = (long) pow(10., 1+ranf()*3); Pv = pow(10.,ranf()*3);
	 if   (NoArg)                                        {;} 
	 else if ( sscanf(Argv[++i], "%ld,%lf", &Pn, &Pv)==2) {;}
	 else pr_intArg(Argv[i], &Pn);
	 }
      else if (ArgEq("-tile")) {
	 if (NoArg) tile++;
	 else if (sscanf(Argv[++i], "%dx%d", &Tx, &Ty) != 2) usage(); 
	 }
      else if (ArgEq("-zoom") && Arg) {
	 if (!sscanf(Argv[i], "%lf", &Zf) || Zf <= 0) usage();
	 }
      else usage();

      }
   /* open display and extract needed values */

#if TOS
	if (fd_cnf())
	{
		Msg("xmartin: Mono Screen 640x400 required.\n");
		exit(1);
	}
	W = 640;
	H = 400;
	DP = 1;	
#else
   W = 640;
   H = 480;
   DP = 4;
#endif
   Ncolor = 1<<(DP-1);

   if (DP > 1) {
      Color++; 
      }

   /* color processing */

   /* provide default hopalong parameters if needed */ 

   if (Function < 0) {
      double p=0, r = ranf();
      for(Function=0; Function <Nfunc; Function++) {
	 p += fprob[Function];
	 if (r < p) break;
	 }
      }

   if (tile) {
      int mxt = (Function == Cp1) ? 4 : 8;
      while ((Tx+Ty) == 2 ) {
	 Tx = (int)(ranf()*mxt + 1);
	 Ty = (int)(ranf()*mxt + 1);
	 }
      }

   W /= Tx; H /= Ty;
   if (W==0||H==0) { Msg("xmartin: tile too small for display\n"); exit(1); }

   if (mxp > 0 && mxp <= 100) mxp = (long) ((mxp*W*H)/100.);
   if (mxP > 0 && mxP <= 100) mxP = (long) ((mxP*W*H)/100.);

   if (!mxp && !mxP) {
      mxp = (long) ((Function == Cp1) ? 200000l : 0.25 * ((double)W * (double)H));
      mxP = (Function >= Cp1) ? mxp : 3 * mxp;
      }
   else if (!mxp) mxp = mxP;
   else if (!mxP) mxP = (Function >= Cp1) ? mxp : 3 * mxp;

   if (!nC) nC = mxP/Ncolor;

   cx = W/2; cy = H/2; mxX = W-1; mxY = H-1;
   if (dist) { 
      cx += dist * cos(degr * (pi/180)); 
      cy -= dist * sin(degr * (pi/180));
      }

   if (Function == Martin1) {
      if (!A) pr_rangeArg("40::1540",  &A);
      if (!B) pr_rangeArg("3::20",     &B);
      if (!C) pr_rangeArg("100::3100", &C);
      if (!Zf) Zf = 1.0;
      }
   else if (Function == Martin2) {
      if (!A) pr_rangeArg("3.0715927::3.2115927",  &A);
      if (!Zf) Zf = 4.0;
      }
   else if (Function == Ejk1) {
      if (!A) pr_rangeArg("0::500", &A);
      if (!B) pr_rangeArg("0::.40", &B);
      if (!C) pr_rangeArg("10::110", &C);
      if (!Zf) Zf = 1.0;
      }
   else if (Function == Ejk2) {
      if (!A) pr_rangeArg("0::500", &A);
      if (!B) { B = pow(10.,6+ranf()*24); if (ranf()<0.5) B = -B; }
      if (!C) { C = pow(10.,  ranf()*9);  if (ranf()<0.5) C = -C; }
      if (!Zf) Zf = 1.0;
      }
   else if (Function == Ejk3) {
      if (!A) pr_rangeArg("0::500", &A);
      if (!B) pr_rangeArg(".05::.40", &B);
      if (!C) pr_rangeArg("30::110", &C);
      if (!Zf) Zf = 1.0;
      }
   else if (Function == Ejk4) {
      if (!A) pr_rangeArg("0::1000", &A);
      if (!B) pr_rangeArg("1::10", &B);
      if (!C) pr_rangeArg("30:70", &C);
      if (!Zf) Zf = 0.7;
      }
   else if (Function == Ejk5) {
      if (!A) pr_rangeArg("0::600", &A);
      if (!B) pr_rangeArg(".1::.4", &B);
      if (!C) pr_rangeArg("20::110", &C);
      if (!Zf) Zf = 0.7;
      }
   else if (Function == Ejk6) {
      if (!A) pr_rangeArg("550:650", &A);
      if (!B) pr_rangeArg(".5::1.5", &B);
      if (!Zf) Zf = 1.2;
      if (!Move) {
	 Move = "320,500";
	 sscanf(Move, "%lf,%d", &degr, &dist);
	 cx += dist * cos((450.-degr)*(pi/180.));
	 cy -= dist * sin((450.-degr)*(pi/180.));
         }
      }
   else if (Function == Rr1) {
      if (!A) pr_rangeArg("0::100", &A);
      if (!B) pr_rangeArg("0::20", &B);
      if (!C) pr_rangeArg("0::200", &C);
      if (!D) pr_rangeArg("0:.9", &D);
      if (!Zf) { 
	 if      (D < .2) Zf = 10;
	 else if (D < .4) Zf = 5;
	 else if (D < .5) Zf = 3;
	 else if (D < .7) Zf = 1;
	 else             Zf = .5;
	 }
      }
   else if (Function == Cp1) {
      unsigned long mxord;
      if (CpOrder) {
      	if (CpOrder > 16) {
      		Msg("xmartin: maximum order 16 for cp1 sine series\n");
	    	exit(1);
	    }
	 mxord = 1; 
	 for (k=0; k<CpOrder; k++) mxord *= 4; 
	 mxord -= 1;
	 if (CpXOrdinal > mxord || CpYOrdinal > mxord) {
	    Msg2("xmartin: %ld maximum ordinal for order %d cp1 sine series.\n",
		  mxord, CpOrder);
	    exit(1);
	    }
	 else if (!ordinals) {
	    CpXOrdinal = (unsigned long) (ranf() * mxord + 0.5);
	    CpYOrdinal = (unsigned long) (ranf() * mxord + 0.5);
	    }
	 }
      else {
	 CpOrder = (int)(3 + ranf()*7);
         mxord = 1; 
	 for (k=0; k<CpOrder; k++) mxord *= 4; 
	 CpXOrdinal = (unsigned long) (ranf() * mxord + 0.5);
	 CpYOrdinal = (unsigned long) (ranf() * mxord + 0.5);
	 }
      
      if (!B) { B = (5 + (int)(ranf()*6)) + ranf(); if (ranf()<0.5) B = -B; }
      if (!A) { A = ((int)(1 + ranf()*4) * B);      if (ranf()<0.5) A = -A;}
      if (!x) { x = 20; }
      if (!y) { y = 30; }
      if (!Zf) { Zf = cp_zoom[CpOrder-1]; }
      if (!Grayscale && nC == mxP/Ncolor) nC = mxP;
      if (!coord) { Coord = (int)(ranf()*4); coord++; }
      if (!Move && (Coord == XY || Coord == YX) ) {
	 Move = cp_move[CpOrder-1];
	 sscanf(Move, "%lf,%d", &degr, &dist);
	 cx += dist * cos((450.-degr)*(pi/180.));
	 cy -= dist * sin((450.-degr)*(pi/180.));
         }
      }

   X0 = x; Y0 = y;

   /* allocate resources needed for pattern */

   /* display pattern parameters */

   Msg("xmartin:------------------------------------------------------------");
   Msg("\n");
   Msg2("xmartin: %dx%d ", W, H);
   Msg("\n");

   Msg1("xmartin: -f %s", fname[Function]);
   if (Function == Cp1) Msg3(",%d,%lu,%lu", CpOrder, CpXOrdinal, CpYOrdinal);
   Msg1(" -a %g ", A);
   if (B) Msg1("-b %g ", B);
   if (C) Msg1("-c %g ", C);
   if (D) Msg1("-d %g ", D);
   if (x) Msg1("-x %g ", x);
   if (y) Msg1("-y %g ", y);
   Msg("\n");

   if (coord || Pn || Move || (Zf + Tx + Ty) != 3) {
      Msg("xmartin: ");
      if (Pn) Msg2("-perturb %ld,%.1f ", Pn, Pv);
      if (coord) Msg1("-coord %s ", coords[Coord]);
      if (Move) Msg1("-move %s ", Move);
      if (Zf != 1) Msg1("-zoom %.2f ", Zf);
      if (Tx!=1 || Ty!=1) Msg2("-tile %dx%d ", Tx, Ty);
      Msg("\n");
      }
	putchar('\007');
	getch();

#if !TOS
   if (!_setvideomode(_VRES16COLOR))
   {
      Msg("xmartin: VGA required.\n");
      exit(1);
   }
#endif
}


static void pr_Argv(char *s) {
   Argv = (Argc) ? (char **) realloc ((char *)Argv, (Argc+1) * sizeof(char *))
                 : (char **) malloc  (sizeof(char *));
   Argv[Argc++] = (s) ? strcpy((char *) malloc (strlen(s)+1), s) : NULL;
   }



static void pr_intArg(char *arg, long *i) {
   if(!sscanf(arg, "%ld", i) || *i <= 0) usage();
   }


static void pr_rangeArg(char *arg, double *x) {
   double x2=0; int xf = 0; 

   if      ( sscanf(arg, "%lf::%lf", x, &x2) == 2) { xf++; }
   else if ( sscanf(arg, "%lf:%lf",  x, &x2) == 2) { ; }
   else if (!sscanf(arg, "%lf",      x))           { usage(); }
   if (x2) {
      *x = Min(*x,x2) + ranf()*(Max(*x,x2) - Min(*x,x2));
      if (xf && ranf()<0.5) *x = -(*x);
      }
   }


static void usage(void) {
   int f = -1; 
   Msg("usage: xmartin\n");
   Msg1("[-f {%s", fname[++f]);
   while (++f < Nfunc) Msg1("|%s", fname[f]); 
   Msg("}]\n");
   Msg("[-f cp1[,order[,xn,yn]]]");
   Msg("\n");
   Msg("[-a real[[:]:real]] "); 
   Msg("[-b real[[:]:real]] "); 
   Msg("[-c real[[:]:real]] "); 
   Msg("\n");
   Msg("[-d real[[:]:real]] "); 
   Msg("[-x real[[:]:real]] "); 
   Msg("[-y real[[:]:real]] "); 
   Msg("\n");
   Msg("[-p npoints] "); 
   Msg("[-P npoints] "); 
   Msg("[-static] "); 
   Msg("\n");
   Msg("[-bg [color[,intensity]]] ");
   Msg("[-nc npoints] "); 
   Msg("[-mono] "); 
   Msg("[-nrc]"); 
   Msg("\n");
   Msg("[-tile [XxY]] "); 
   Msg("[-perturb [n[,v]]] "); 
   Msg("[-coord {xy|yx|ra|ar}]");
   Msg("\n");
   Msg("[-zoom real] "); 
   Msg("[-move d,p] "); 
   Msg("\n");
   Msg("[-recall] "); 
   Msg("[-v]"); 
   Msg("\n"); 
   exit(1); 
   }
