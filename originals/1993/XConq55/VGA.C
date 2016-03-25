/*  vga.c - for xconq 5.5 and emx gcc
 *  replaces x11.c or curses.c 
 * 
 *  (c) 1993 by Martin Lercher
 *  martin@alf2.ngate.uni-regensburg.de
 * 
 *  This is FREE SOFTWARE. See GNU Licence for Details.
 *
 */

#define WINLOG 0
#define SETPALBUG 0   /* 0 ab Vesalib 1.4 */


#include <stdio.h>
#include <stdlib.h>
#include "vesa/graph.h"
#include <signal.h>
#include <conio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/time.h>
#include <string.h>
#include <dos.h>

#include "config.h"
#include "misc.h"
#include "period.h"
#include "global.h"
#include "side.h"
#include "map.h"

#if SETPALBUG
# define g_setpal(a,b,c,d)  /* baeaeaeaeae */
#endif

#define MAXWINDOWS 12
#define MAXRGB 1024 /* Min 750 */

#define INFOLINES 5
#define BH 32

int hw = 20;
int hh = 22;
int hch = 17;
int margin = 2;
int bd = 1;

static wr_mode = 1;

typedef long Window;  /* a window handle */
struct aWindow {
	int x, y, w, h;
}; 

char *routine_executing = "Program Start";

int helpwinlines = 1; /* size of welp window */

#define sd() ((Screeno *) side->display)
#define sw(win, ext) sd()->wins[win].ext

typedef struct a_Pixmap {
	short w, h;
	unsigned char *bytes; /* linksbuendig */	
} Pixmap;

typedef struct a_Font {
	short w, h;
	Pixmap pixels[256];
} Font;

typedef struct a_screen {
    short help_on;
    short escape;
    short xsize, ysize, colors;
    Font *textfont;	/* font for text display */
    Font *iconfont;	/* utility font with assorted icons */
    Font *unitfont;	/* font for unit characters */
    Font *helpfont;      /* font for help window */
    Pixmap *unitpics[MAXUTYPES];	/* used instead of font sometimes */
    Pixmap bombpics[4]; /* the mushrooms */
    struct aWindow wins[MAXWINDOWS];
    Window lastwindow;
} Screeno;


typedef struct a_rgb {
	unsigned char r, g, b, pixel;
	char *name;
} RGB;

static RGB rgb[MAXRGB];
static lastrgb;


static struct bombPixmap
{ unsigned char bytes[4*32]; /* 32x32 */ }
bombpics[4] = {  /* mushroom clouds */ 
{
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0xc0,  0x01, 0x00,  0x00, 0xf0,  0x07, 0x00, 
   0x00, 0xf0,  0x07, 0x00,  0x00, 0xf8,  0x0f, 0x00, 
   0x00, 0xf8,  0x0f, 0x00,  0x00, 0xfc,  0x0f, 0x00, 
   0x00, 0xfe,  0x1f, 0x00,  0x00, 0xfe,  0x3f, 0x00, 
   0x00, 0xfe,  0x3f, 0x00,  0x00, 0xfc,  0x1f, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00},
{
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0xf8,  0x01, 0x00, 
   0x00, 0xf8,  0x07, 0x00,  0x00, 0xfc,  0x0f, 0x00, 
   0x00, 0xfc,  0x1f, 0x00,  0x00, 0xfe,  0x1f, 0x00, 
   0x00, 0xfe,  0x3f, 0x00,  0x00, 0xfc,  0x1f, 0x00, 
   0x00, 0xfc,  0x0f, 0x00,  0x00, 0xf0,  0x07, 0x00, 
   0x00, 0xf0,  0x03, 0x00,  0x00, 0xe0,  0x03, 0x00, 
   0x00, 0xf0,  0x07, 0x00,  0x00, 0xf0,  0x07, 0x00, 
   0x00, 0xf0,  0x0f, 0x00,  0x00, 0xf0,  0x0f, 0x00, 
   0x00, 0xf8,  0x0f, 0x00,  0x00, 0xf8,  0x1f, 0x00, 
   0x00, 0xf8,  0x1f, 0x00,  0x00, 0xfc,  0x1f, 0x00, 
   0x00, 0xff,  0x7f, 0x00,  0xc0, 0xff,  0xff, 0x03, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00},
{
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0xfe, 0xe2,  0xa3, 0x3f, 
   0x00, 0xfc,  0x1f, 0x00,  0x3c, 0xff,  0x7f, 0x7c, 
   0x80, 0xff,  0xff, 0x00,  0xc0, 0xff,  0xff, 0x01, 
   0xc0, 0xff,  0xff, 0x01,  0xe0, 0xff,  0xff, 0x03, 
   0xe0, 0xff,  0xff, 0x03,  0xe0, 0xff,  0xff, 0x03, 
   0xe0, 0xff,  0xff, 0x01,  0xe0, 0xff,  0xff, 0x49, 
   0x82, 0xff,  0xff, 0x34,  0x14, 0xfe,  0x3f, 0x42, 
   0xe2, 0xff,  0x9f, 0x34,  0x40, 0xfe,  0x3f, 0x41, 
   0xbe, 0xfd,  0xdf, 0x1e,  0x00, 0xf8,  0x1f, 0x01, 
   0xfe, 0xfd,  0xdf, 0x7f,  0x00, 0xfc,  0x1f, 0x00, 
   0x00, 0xfc,  0x1f, 0x00,  0x00, 0xfc,  0x3f, 0x00, 
   0x00, 0xfe,  0x1f, 0x00,  0x00, 0xfe,  0x3f, 0x00, 
   0x00, 0xff,  0x3f, 0x00,  0xc0, 0xff,  0xff, 0x00, 
   0xfc, 0xff,  0xff, 0x3f,  0xfe, 0xff,  0xff, 0x7f, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00},
{
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0xc0,  0x07, 0x00, 
   0x00, 0x78,  0x10, 0x00,  0x00, 0x0f,  0x46, 0x00, 
   0x80, 0x61,  0x81, 0x00,  0xc0, 0x1c,  0x00, 0x01, 
   0x40, 0x02,  0x00, 0x00,  0x20, 0x01,  0x00, 0x00, 
   0x20, 0x01,  0x00, 0x00,  0x20, 0x00,  0x00, 0x00, 
   0x20, 0x00,  0x00, 0x00,  0x40, 0x00,  0x00, 0x00, 
   0x80, 0x00,  0x00, 0x00,  0x00, 0x02,  0x00, 0x00, 
   0x00, 0x02,  0x00, 0x00,  0x00, 0x04,  0x00, 0x00, 
   0x00, 0x04,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x04,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x04,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x04,  0x00, 0x00,  0x00, 0x02,  0x00, 0x00, 
   0x00, 0x01,  0x00, 0x00,  0xc0, 0x00,  0x00, 0x00, 
   0x2c, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00}
};


static void mouse_init(void)
{	union REGS in, out;

	in.x.ax = 0;
	_int86(0x33, &in, &out);
	in.x.ax = 7;
	in.x.cx = 0;
	in.x.dx = g_xsize-1;
	_int86(0x33, &in, &out);
	in.x.ax = 8;
	in.x.cx = 0;
	in.x.dx = g_ysize-1;
	_int86(0x33, &in, &out);	
}

static int mouse_pos(int *x, int *y)
{	union REGS in, out;

	in.x.ax = 3;
	_int86(0x33, &in, &out);
	*x=out.x.cx; *y=out.x.dx;
	return out.x.bx;
}

static void put_pixmap(Side *side, Window win, int x, int y, int color, int bgcolor, Pixmap *pixmap)
{	int w, h, ww, i, j, n, len;
	unsigned bit;
	unsigned char *p, *q;
static unsigned char buf[50*50];
	
	if (!pixmap)
		return;
	x += sw(win, x);
	y += sw(win, y);
	
	if (!pixmap->bytes)
		return;
	w = pixmap->w;
	h = pixmap->h;
	ww = (w+7)>>3;
	len = w*h;
	assert(len < 50*50);
#if 1
	if (wr_mode)
	{
		p = pixmap->bytes;
		for(j=0; j<h; j++)
		{
			bit = 1;
			for(i=0; i<w; i++)
			{	unsigned t;

			if (bit == 0x100)
				bit = 1;
			if (bit == 1)
				t = *p++;
			if (t&bit)
				g_set(x+i, y+j, color);
			bit <<= 1;
			}
		}
	}
	else
	{
		p = pixmap->bytes;
		for(j=0; j<h; j++)
		{
			bit = 1;
			for(i=0; i<w; i++)
			{	unsigned t;

			if (bit == 0x100)
				bit = 1;
			if (bit == 1)
				t = *p++;
			if (t&bit)
				g_set(x+i, y+j, color);
			else
				g_set(x+i, y+j, bgcolor);
			bit <<= 1;
			}
		}
	}
#else
  if (wr_mode)
  {
	g_getimage(x, y, x+w-1, y+h-1, buf);
	p = pixmap->bytes;
	q = buf;
	for(n=0; n<len; n++)
	{	unsigned t;

		if (bit == 0x100 || (n%w) == 0)
			bit = 1;
		if (bit == 1)
			t = *p++;
		if (t&bit)
			*q = (unsigned char)color;
		q++;
		bit <<= 1;
	}
  }
  else
  {
	p = pixmap->bytes;
	q = buf;
	for(n=0; n<len; n++)
	{	unsigned t;

		if (bit == 0x100 || (n%w) == 0)
			bit = 1;
		if (bit == 1)
			t = *p++;
		if (t&bit)
			*q = (unsigned char)color;
		else
			*q = (unsigned char)bgcolor;
		q++;
		bit <<= 1;
	}
  }
	g_putimage(x, y, x+w-1, y+h-1, buf);
#endif
}

static void put_text(Side *side, Window win, int x, int y,
	int c, int bc, Font *font, char *buf, int len)
{	int xx;
	
	xx = x;
	while(len--)
	{
		if (*buf != '\n')
		{
			put_pixmap(side, win, x, y + font->h - font->pixels[*buf].h,
				c, bc, &(font->pixels[*buf]));
#if 1
			x += font->w;
#else
			x += font->pixels[*buf].w;
#endif
			buf++;
		}
		else
		{
			x = xx;
			y += font->h;
			buf++;
		}
	}
}


static void clipwin(Side *side, Window win)
{	int x,y,w,h;

	x = sw(win, x);
	y = sw(win, y);
	w = sw(win, w);
	h = sw(win, h);
	g_clip(x, y+1, x+w-1, y+h-1);
}

long request_color(side, name, class, fallback)
Side *side;
char *name, *class, *fallback;
{
static int co=0;
int i;

#if SETPALBUG
	co = 1;
#endif

	for(i=0; i<lastrgb; i++)
	{
		if (0 == stricmp(fallback, rgb[i].name))
		{	unsigned char buf[3];

			if (rgb[i].pixel)
				return (unsigned long)rgb[i].pixel;
			if (co>=0x100)
				break;
			buf[0] = rgb[i].r >> 2;
			buf[1] = rgb[i].g >> 2;
			buf[2] = rgb[i].b >> 2;
			rgb[i].pixel = co;
			g_setpal(buf, co, 1, 1);
			return co++;
		}
	}
	return white_color();
}	

add_default_player()
{
	char *myname;

	myname = getenv("NAME");
	if (myname == NULL)
		myname = "No NAME";
	add_player(TRUE, myname);
}


static void program_crash(int sig, int code, void *scp, char *addr)
{
  int i;
  static bool already_been_here = FALSE;

  printf("Fatal error encountered. Signal %d code %d\n", sig, code);
  printf("Routine executing %s\n", routine_executing);
  printf("Procedure stack (incomplete): \n");
  for (i = 0; i <= procedure_stack_ptr; i++)
    printf(" %s\n", procedure_executing[i]);
  if (already_been_here) exit(0);
  else {
    already_been_here = TRUE;
    exit(1);
  }
}


init_sighandlers()
{
	signal(SIGFPE, program_crash);
	signal(SIGINT, program_crash);
	signal(0, program_crash);
	signal(1, program_crash);
	signal(SIGHUP, program_crash);
    signal(SIGBUS, program_crash);
    signal(SIGSEGV, program_crash);
    signal(SIGFPE, program_crash);
    signal(SIGILL, program_crash);
    signal(SIGSYS, program_crash);
    signal(SIGINT, program_crash);
    signal(SIGQUIT, program_crash);
    signal(SIGTERM, program_crash);
}

static void load_rgb(void)
{	FILE *f;
	char buf[50];
	int r, g, b;

	make_pathname(xconqlib, "rgb", NULL, spbuf);
	f = fopen(spbuf, "r");
	if (!f)
	{
		perror(spbuf);
		exit(1);
	}
    lastrgb = 0;
    while(!feof(f))
    {
	fscanf(f, "%d %d %d %[^\n]", &r, &g, &b, buf);
	assert(lastrgb < MAXRGB);
	rgb[lastrgb].name = strdup(buf);
	assert(rgb[lastrgb].name);
	rgb[lastrgb].r = r;
	rgb[lastrgb].g = g;
	rgb[lastrgb].b = b;
	rgb[lastrgb].pixel = 0; /* noch nicht alloziert */
	lastrgb++;
    }
    fclose(f);
    if (Debug) printf("RGB Database loaded ...\n");
}

static void std_color(Side *side)
{
	request_color(side, NULL, NULL, "gray40");
	request_color(side, NULL, NULL, "black");
	request_color(side, NULL, NULL, "white");
}


open_display(side)
Side *side;
{	int mode;
static here=0;

	if (here)
	{
		fprintf(stderr, "No second display!\n");
		exit(1);
	}
	here = 1;
	
	side->display = (long) calloc(sizeof(struct a_screen), 1);
	assert(side->display);

	mode = G_MODE_VGA_L; /* 300x200x256 */
	if (getenv("VESAMODE"))
		mode = atoi(getenv("VESAMODE"));
	/* g_mode(0x212); / * 360x480x256 */
	if (!g_mode(mode)) 
	{
		printf("Cannot open graphics mode. This is fatal.\n");
		exit(1); /* this is fatal */
	}
	load_rgb();
	std_color(side);
	mouse_init();
	sd()->xsize = g_xsize;
	sd()->ysize = g_ysize;
	sd()->colors = g_colors;
	return 1;
}	


active_display(side)
Side *side;
{
	return (side && side->host && !side->lost && side->display);
}	


display_width(side)
Side *side;
{
	return sd()->xsize; 
}	


display_height(side)
Side *side;
{
	return sd()->ysize;
}	


world_display(side)
Side *side;
{
	return TRUE;
}	


static void get_font_size(Font *font, short *fw, short *fh)
{
	*fw = font->w;
	*fh = font->h;
}


static Font *load_font(Side *side, char *fn)
{	FILE *f;
	Font *font;
	
	if (Debug) printf("Loading %s ... ", fn);
	font = (Font *) calloc(sizeof(Font), 1);
	assert(font);
	make_pathname(xconqlib, fn, NULL, spbuf);
	f = fopen(spbuf, "rb");
	if (!f)
	{
		perror(spbuf);
		exit(1);
	}
	
	fread(&font->w, sizeof(font->w), 1, f);
	fread(&font->h, sizeof(font->h), 1, f);
	font->h++;
	
	if (Debug) printf("[%dx%d] ", (int) font->w, (int) font->h);
	while(!feof(f))
	{	short ch, w, h, ww;
		int r;
	
		fread(&ch, sizeof(ch), 1, f);
		fread(&w, sizeof(w), 1, f);
		fread(&h, sizeof(h), 1, f);
		assert(ch<0x100);
		font->pixels[ch].w = w;
		font->pixels[ch].h = h;
		ww = (w+7) >> 3;
		font->pixels[ch].bytes = malloc((int)h * ww);
		assert(font->pixels[ch].bytes);
		fread(font->pixels[ch].bytes, h, ww, f);
		if (Debug) printf("%c", isprint(ch) ? ch : '.');
	}
	fclose(f);
	if (Debug) printf("Done.\n");
	return font;
}


static Pixmap *load_pixmap(FILE *f, Side *side, char *name)
{	char buf[20], buf1[20], *s;
	short w, h, ww;
	
	strncpy(buf1, name, 8);
	buf1[8]=0;
	for(s = buf1; *s; s++)
		*s = tolower(*s);
	strcat(buf1, ".b");
	if (Debug) printf("Loading pixmap %s ... ", buf1);
	rewind(f);
	while(!feof(f))
	{	
		if (!fread(buf, sizeof(buf), 1, f))
			break;
		fread(&w, sizeof(w), 1, f);
		fread(&h, sizeof(h), 1, f);
		ww = h * (w >> 3);
		if (0 == strncmp(buf, buf1, sizeof(buf)))
		{	Pixmap *p;
			unsigned char *bytes;
			
			p = calloc(sizeof(Pixmap), 1);
			assert(p);
			bytes = malloc(ww);
			assert(bytes);
			fread(bytes, (int)ww, 1, f);
			p->w = w;
			p->h = h;
			p->bytes = bytes;
			if (Debug) printf("Done.\n");
			return p;
		}
		fseek(f, ww, SEEK_CUR);
	}
	fprintf(stderr, "Bitmap %s not found\n", name);
	return NULL;
}


init_misc(side)
Side *side;
{	FILE *f;
	int u, i;

    side->margin = margin;
    side->bd = bd;
    side->hw = hw;
    side->hh = hh;
    side->hch = hch;

	sd()->helpfont = load_font(side, "HelpFont");
	sd()->textfont = load_font(side, "TextFont");
	sd()->iconfont = load_font(side, "IconFont");
	if (period.fontname != NULL && strlen(period.fontname) > 0)
		sd()->unitfont = load_font(side, period.fontname);
	else
	      	sd()->unitfont = load_font(side, "Standard");
	      	
	get_font_size(sd()->helpfont, &(side->hfw), &(side->hfh));
	get_font_size(sd()->textfont, &(side->fw),  &(side->fh) );
	get_font_size(sd()->iconfont, &(side->hw),  &(side->hh) );
	get_font_size(sd()->unitfont, &(side->uw),  &(side->uh) );

	make_pathname(xconqlib, "pixmaps", NULL, spbuf);
	f = fopen(spbuf, "rb");
	if (!f)
	{
		perror(spbuf);
		exit(1);
	}
	
    for_all_unit_types(u) {
	if (utypes[u].bitmapname && (strlen(utypes[u].bitmapname) > 0)) {
	    sd()->unitpics[u] = load_pixmap(f, side, utypes[u].bitmapname);
	} else {
	    utypes[u].bitmapname = NULL;
	}
    }
    fclose(f);
    for(i=0; i<4; i++)
    {
	sd()->bombpics[i].w = 32;
	sd()->bombpics[i].h = 32;
	sd()->bombpics[i].bytes = bombpics[i].bytes;
    }
    if (Debug) printf("Bitmaps stored ...\n");


	side->bd = side->margin = 0;

	side->owncolor   = request_color(side, NULL, NULL, "black");
	side->altcolor   = request_color(side, NULL, NULL, "blue");
	side->diffcolor  = request_color(side, NULL, NULL, "maroon");
	side->bdcolor    = request_color(side, NULL, NULL, "blue");
	side->graycolor  = request_color(side, NULL, NULL, "light gray");
	side->enemycolor = request_color(side, NULL, NULL, "red");
	side->neutcolor  = request_color(side, NULL, NULL, "light gray");
	side->goodcolor  = request_color(side, NULL, NULL, "green");
	side->badcolor   = request_color(side, NULL, NULL, "red");

	side->showmode = 3; /* BothIcons */
	
}	


get_color_resources(side)
     Side	*side;
{

}	


create_window(side, x, y, w, h)
Side *side;
int x, y, w, h;
{
	if (sd()->lastwindow >= MAXWINDOWS)
		return 0;
	sw(sd()->lastwindow, x) = x;
	sw(sd()->lastwindow, y) = y;
	sw(sd()->lastwindow, w) = w;
	sw(sd()->lastwindow, h) = h;
	return sd()->lastwindow++;
}	


create_main_window(side)
Side *side;
{
	create_window(side, 0, 0, g_xsize, g_ysize);
}	



create_help_window(side)
Side *side;
{
	helpwinlines =
		max(55, (24 + period.numrtypes + period.numttypes +period.numutypes));
	side->help = 
		create_window(side, 0,0, 80*side->hfw+1, helpwinlines * side->hfh+1);
}

set_retain(side, window)
Side *side;
long window;
{
	/* ignore this */
}	


static void map_window(Side *side, Window win)
{
	clipwin(side, 0);
	if (win<sd()->lastwindow)
	{	int x,y,w,h;
	
		x = sw(win, x);
		y = sw(win, y);
		w = sw(win, w);
		h = sw(win, h);
		g_box(x-1,y,x+w,y+h, side->fgcolor, G_OUTLINE);
	}
}


fixup_windows(side)
Side *side;
{
	map_window(side, side->main);
	map_window(side, side->msg);
	map_window(side, side->info);
	map_window(side, side->prompt);
	map_window(side, side->map);
	map_window(side, side->sides);
	map_window(side, side->timemode);
	map_window(side, side->clock);
	map_window(side, side->state);
	map_window(side, side->world);
	
#if WINLOG
	{
		int i;
		FILE *f;

		f=fopen("winlog", "a");
		for(i = 0; i<sd()->lastwindow; i++)
		{
			fprintf(f, "Window %2d is  x=%3d   y=%3d   w=%3d   h=%3d\n",
				i, sw(i, x), sw(i, y), sw(i, w), sw(i, h));
		}
		fprintf(f, "-------------------------------------------\n");
		fclose(f);
	}
#endif
}	


enable_input(side)
Side *side;
{

}	


reset_misc(side)
Side *side;
{
	clear_window(side, 0);
}	


change_window(side, win, x, y, w, h)
Side *side;
Window win;
int x, y, w, h;
{
	clear_window(side, win);
	sw(win, x) = x;
	sw(win, y) = y;
	sw(win, w) = w;
	sw(win, h) = h;
	/* OK, redraw ??? */
#if WINLOG
	{
		int i = win;
		FILE *f;

		f=fopen("winlog", "a");
		fprintf(f, "*** Change Window %d is  x=%3d   y=%3d   w=%3d   h=%3d\n",
				i, sw(i, x), sw(i, y), sw(i, w), sw(i, h));
		fclose(f);
	}
#endif
}	


display_colors(side)
Side *side;
{
	return sd()->colors;
}	


static int grey_color(Side *side)
{
#if SETPALBUG
	return G_WHITE;
#else
	return 0;
#endif
}	


white_color(side)
Side *side;
{
#if SETPALBUG
	return G_WHITE|G_INTENSITY;
#else
	return 2;
#endif
}	


black_color(side)
Side *side;
{
#if SETPALBUG
	return G_BLACK;
#else
	return 1;
#endif
}	



#define PLACE 0
#define REMOVE 1
#define mouse_w 11
#define mouse_h 11
#define mouse_x 5
#define mouse_y 5

static unsigned char mouse_p[mouse_w][mouse_h] = {
 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0
};

static void mouse_do(int mode, int x, int y)
{	int a, b;

static unsigned char save[mouse_w * mouse_h];

	x -= mouse_x;
	y -= mouse_y;
	
	if (mode == PLACE)
	{
		g_getimage(x, y, x+mouse_w-1, y+mouse_h-1, save);
		for(a=0; a<mouse_h; a++)
		{
			for(b=0; b<mouse_w; b++)
			{
				if (mouse_p[a][b])
					g_set(x+a, y+b, mouse_p[a][b]);
			}
		}
	}
	else
	{
		g_putimage(x, y, x+mouse_w-1, y+mouse_h-1, save);
	}
}

static Window get_window(Side *side, int x, int y, int *xx, int *yy)
{	Window win;

	/* Omit the help window */
	for(win = sd()->lastwindow - 2; win; win--)
	{
		if (x>=sw(win, x) && x<=sw(win, x)+sw(win, w)-1
		&&  y>=sw(win, y) && y<=sw(win, y)+sw(win, h)-1)
			break;
	}
	*xx = x - sw(win, x);
	*yy = y - sw(win, y);
	return win;
}

static void event(Side *side)
{	char ch;
	int mx, my, mf;
	int x, y, f;
	int n;
	
	sd()->escape = FALSE;
	clipwin(side, 0);
	/* Polling, nur linke Maustaste */
	mf = mouse_pos(&mx, &my) & 1;
	mouse_do(PLACE, mx, my);
	side->reqtype = GARBAGE;
	for(n=0; n<4000; n++) /* let time a chance */
	{
		ch = _read_kbd(0,0,0);
		if (ch != -1)
		{
			side->reqtype = KEYBOARD;
			side->reqch = ch;
			break;
		}
		f = mouse_pos(&x, &y) & 1;
		if (x != mx || y != my)
		{
			mouse_do(REMOVE, mx, my);
			mx = x;
			my = y;
			mouse_do(PLACE, mx, my);
		}
		if (f != mf)
		{	int rawx, rawy;
		
			mf = f;
			if (f == 1)
			{	Window window;
		/* BUTTONPRESS */
			
				window = get_window(side, mx, my, &rawx, &rawy);
				if (window == side->map) {
					side->reqtype = MAPPOS;
					side->reqch = f;
					/* the modifier is used by x_command */
					deform(side, rawx, rawy, &(side->reqx), &(side->reqy));
					if (Debug) printf("Host %s returns map %d %d\n",
				      		side->host, side->reqx, side->reqy);
				      	break;
				} else if (window == side->world) {
					side->reqtype = MAPPOS;
					w_deform(side, rawx, rawy, &(side->reqx), &(side->reqy));
					if (Debug) printf("Host %s returns world %d %d\n",
						side->host, side->reqx, side->reqy);
					break;
				} else if (window == side->state) {
					side->reqtype = UNITTYPE;
					side->requtype = rawy / max(side->hh, side->fh);
					if (Debug) printf("Host %s returns unit type %d\n",
						side->host, side->requtype);
					break;
				} else if (window == side->msg) {
					if (rawy < side->nh * side->fh / 2)
						side->reqtype = SCROLLUP;
					else side->reqtype = SCROLLDOWN;
					break;
				}
		/* BUTTONPRESS */
			}
		}
	}
	mouse_do(REMOVE, mx, my);
}

get_input()
{
	Side *side, *curside;

	while(1)
	{

	for_all_sides(side) {
	  int delta_time = (time(0) - side->lasttime);

	  side->lasttime = time(0);
	  if (side->more_units) {
	    side->timeleft -= delta_time;
	    side->timetaken += delta_time;
	    update_clock(side);
	  }
	}

		for_all_sides(curside)
    			if (curside->reqactive)
    				side = curside;
	        if (active_display(side))
		{
	        	if (sd()->help_on > 1)
	        	{
	        		sd()->help_on = (sd()->help_on == 2);
				side->reqtype = KEYBOARD;
				side->reqch = '\014';  /* ^L */
	        	}
	        	else
	        	{
	        		event(side);
			}
			/* Program will not work without following due to compiler bug. */
			if (sd()->help_on)
				x_help(side);
			else
				return TRUE;
		}
	}
}	


/* Allow a player to exit robot mode, even if he is the only player. */
exit_robot_check()
{
  Side *side;
  
  for_all_sides(side)
  {
    if (active_display(side))
    {
        side->redisplay_ok = TRUE;
 	if (!humanside(side) && sd()->escape)
	{
	  side->humanp = TRUE;
	  numhumans++;
	  init_sighandlers();
	  return;
	}
    } 
  }
}



freeze_wait(side)
Side *side;
{
	return getch();
}	


flush_input(side)
Side *side;
{	char ch;

	while((ch = _read_kbd(0,0,0)) != -1)
		if (ch == '\033')
			sd()->escape = TRUE;
	side->reqchange = TRUE;
}	


flush_output(side) 
Side *side; 
{
	side->reqchange = TRUE;
}	


clear_window(side, win)
Side *side;
Window win;
{	int x,y,w,h;

	map_window(side, win);

	x = sw(win, x);
	y = sw(win, y);
	w = sw(win, w);
	h = sw(win, h);
	g_box(x,y+1,x+w-1,y+h-1, side->bgcolor, G_FILL);

#if WINLOG
	{
		int i = win;
		FILE *f;

		f=fopen("winlog", "a");
		fprintf(f, "Clear Window %d is  x=%3d   y=%3d   w=%3d   h=%3d\n",
				i, sw(i, x), sw(i, y), sw(i, w), sw(i, h));
		fclose(f);
	}
#endif
}	

/* Draw a single horizontal constant-color bar on the world map.  If part */
/* would not be drawn because of the map's obliqueness, cut it in two and */
/* wrap one of the pieces around. */

typedef unsigned int unint;

draw_bar(side, x, y, len, color)
Side *side;
int x, y, len, color;
{
    int sx1, sx2, sy, sww;

    w_xform(side, x, y, &sx1, &sy);
    w_xform(side, x + len, y, &sx2, &sy);
    sww = side->mm * world.width;
    clipwin(side, side->world);
    if (sx1 < sww && sx2 >= sww) {
    	g_box(sx1+sw(side->world, x)  , sy           +sw(side->world, y), 
    	      sww+sw(side->world, x)-1, sy+ side->mm +sw(side->world, y)-1,
	      color, G_FILL);
    	g_box(    sw(side->world, x)  , sy           +sw(side->world, y), 
    	  sx2-sww+sw(side->world, x)-1, sy+ side->mm +sw(side->world, y)-1,
          color, G_FILL);
    } else {
		sx1 %= sww;
		sx2 %= sww;
		g_box(sx1+sw(side->world, x)  , sy         +sw(side->world, y), 
    		      sx2+sw(side->world, x)-1, sy+side->mm+sw(side->world, y)-1,
		      color, G_FILL);
    }
}	


static void invert_v(int x, int y, int y2)
{
	while(y<=y2)
	{
		g_set(x, y, ~g_get(x, y));
		y++;
	}
}


static void invert_h(int y, int x, int x2)
{
	while(x<=x2)
	{
		g_set(x, y, ~g_get(x, y));
		x++;
	}
}

/* Invert the outline box on the world map.  This is a little tricky, */
/* because we want the lines to run through the middle of the world's */
/* hexes, and because the line drawn should not overlap (or the overlaps */
/* will be doubly inverted and look strange). */

invert_box(side, vcx, vcy)
Side *side;
int vcx, vcy;
{
    int x1, y1, x2, y2, sx1, sy1, sx2, sy2, mm2 = side->mm/2;

    x1 = vcx - side->vw2 + side->vh2/2;  y1 = vcy - side->vh2;
    x2 = vcx + side->vw2 - side->vh2/2;  y2 = vcy + side->vh2;
    w_xform(side, x1, y1, &sx1, &sy1);
    w_xform(side, x2, y2, &sx2, &sy2);
    sx1 += mm2;  sy1 -= mm2;  sx2 += mm2;  sy2 += mm2;

    clipwin(side, side->world);
    sx1 += sw(side->world, x);
    sx2 += sw(side->world, x);
    sy1 += sw(side->world, y);
    sy2 += sw(side->world, y);
	invert_h(sy1, sx1, sx2);
	invert_h(sy2, sx1, sx2);
	invert_v(sx1, sy2+1, sy1-1);
	invert_v(sx2, sy2+1, sy1-1);
}


draw_terrain_row(side, sx, sy, buf, len, color)
Side *side;
int sx, sy, len, color;
char *buf;
{
	clipwin(side, side->map);
	put_text(side, side->map, sx, sy, color, side->bgcolor, sd()->iconfont, buf, len);
}	

/* Napping is like sleeping, but maybe shorter.  Arg is milliseconds. */

static void nap(t)
long t;
{
#if defined(UNIX) || defined (GCCDOS)
#ifdef SELECT2
    struct timeval naptime;

    naptime.tv_usec = (t % 1000) * 1000;
    naptime.tv_sec = t / 1000;
    select(0, NULL, NULL, NULL, &naptime);
#else
    if (t >= 1000) sleep(t/1000);
#endif
#endif /* UNIX */
}

static void inv_line(int x, int y, int x1, int y1)
{
	if (y<y1)
		for (; x<x1; x++)
		{
			g_set(x, y, ~g_get(x, y));
			y++;
		}
	else
		for (; x<x1; x++)
		{
			g_set(x, y, ~g_get(x, y));
			y--;
		}
}

/* Flash a pair of lines up, slow enough to draw the eye, but not so slow */
/* as to get in the way. */

flash_position(side, sx, sy, tm)
Side *side;
int sx, sy, tm;
{    int sx1, sy1, sx2, sy2;
	
#define XDrawLine(a,win,b,x0,y0,x1,y1) inv_line(x0+sw(win, x),y0+sw(win, y),x1+sw(win, x),y1+sw(win, y))
    if (tm > 0) {
	clipwin(side, side->map);
	sx1 = sx - 50 + side->hw/2;  sy1 = sy + 50 + side->hch/2;
	sx2 = sx + 50 + side->hw/2;  sy2 = sy - 50 + side->hch/2;
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy1, sx2, sy2);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy2, sx2, sy1);
	flush_output(side);
	nap(tm);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy1, sx2, sy2);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy2, sx2, sy1);
    }
#undef XDrawLine
}	

draw_cursor_icon(side, sx, sy)
Side *side;
int sx, sy;
{
	clipwin(side, side->map);
#if 0
	draw_text(side->map, sx, sy, side->bgcolor, sd()->iconfont, "[", 1);
	draw_text(side->map, sx, sy, side->fgcolor, sd()->iconfont, "]", 1);
#else
	g_box(5+sx+sw(side->map, x), 5+sy+sw(side->map, y),
		sx+15+sw(side->map, x), sy+15+sw(side->map, y), side->fgcolor, G_OUTLINE);
#endif
}	


draw_hex_icon(side, win, sx, sy, color, ch)
Side *side;
Window win;
int sx, sy, color;
char ch;
{
	clipwin(side, win);
	put_text(side, win, sx, sy, color, side->bgcolor, sd()->iconfont, &ch, 1);
}	


draw_side_number(side, win, sx, sy, n, color)
Side *side;
Window win;
int sx, sy, n, color;
{
    char ch = n%22;

    ch += (ch<10)?'0':('A' - 10);

    if (n >= 0 && n!=MAXSIDES) {
	    clipwin(side, win);
	    put_text(side, win, sx, sy, color, side->bgcolor, sd()->iconfont, &ch, 1);
    }
}	


draw_blast_icon(side, win, sx, sy, type, color)
Side *side;
Window win;
int sx, sy, color;
char type;
{
	clipwin(side, win);
	put_text(side, win, sx, sy, color, side->bgcolor, sd()->iconfont, &type, 1);
}	


invert_whole_map(side)
Side *side;
{	char buf[3];

	g_getpal(buf, grey_color(side), 1);
	g_setpal("\0\0\0", grey_color(side), 1, 1);
	nap(25);
	g_setpal("\077\0\0", grey_color(side), 1, 1);
	nap(25);
	g_setpal("\0\077\0", grey_color(side), 1, 1);
	nap(25);
	g_setpal("\0\0\077", grey_color(side), 1, 1);
	nap(25);
	g_setpal("\077\077\077", grey_color(side), 1, 1);
	nap(25);
	g_setpal(buf, grey_color(side), 1, 1);
}	


draw_mushroom(side, x, y, i)
Side *side;
int x, y, i;
{   int sx, sy;
    int color;

    clipwin(side, side->map);
    color = ((side->monochrome || i == 3) ? side->fgcolor : side->bgcolor);
	/* xform !!! */
    xform(side, unwrap(side, x, y), y, &sx, &sy);
	put_pixmap(side, side->map, sx-BH/4, sy-BH/2, side->fgcolor, side->bgcolor, &(sd()->bombpics[i]));
}	

#if 0
bar_graphs(side)
Side *side;
{
	return TRUE;
}	


draw_graph(side, number, amount, total, critical, title)
Side *side;
int number, amount, total, critical;
char *title;
{
    int boxwidth, boxheight, boxoffset, boxleft, barwidth, barheight;
	int color;

    if (total > 0) {
	boxwidth = 5*side->fw;
	boxheight = (INFOLINES-1)*side->fh - 2*side->margin;
	boxoffset = side->margin;
	boxleft = 30*side->fw + number * boxwidth;
	barwidth = boxwidth / 3;
	barheight = (boxheight * amount) / total;
	clipwin(side, side->info);
#define XSetForeground(a,b,c) color = c
#define XFillRectangle(a,win,b,x0,y0,x1,y1) g_box(x0+sw(win, x),y0+sw(win, y),x0+x1+sw(win, x),y0+y1+sw(win, y),color,G_FILL)
	XSetForeground(sdd(), sd()->gc, side->fgcolor);
	XFillRectangle(sdd(), side->info, sd()->gc,
		       boxleft + boxwidth/3 - 1, boxoffset - 1,
		       barwidth + 2, boxheight + 2);
	XSetForeground(sdd(), sd()->gc, side->bgcolor);
	XFillRectangle(sdd(), side->info, sd()->gc,
		       boxleft + boxwidth/3, boxoffset,
		       barwidth, boxheight);
	if ( amount > critical)
	  XSetForeground(sdd(), sd()->gc, side->goodcolor);
	else
	  XSetForeground(sdd(), sd()->gc, side->badcolor);
	XFillRectangle(sdd(), side->info, sd()->gc,
		       boxleft + boxwidth/3, boxoffset + boxheight - barheight,
		       barwidth, barheight);
	draw_text(side, side->info,
		  boxleft+(boxwidth-strlen(title)*side->fw)/2,
		  (INFOLINES-1)*side->fh, title, side->fgcolor);
    }
#undef XSetForeground
#undef XFillRectangle
}	
#endif


draw_unit_icon(side, win, x, y, u, color)
Side *side;
Window win;
int x, y, u, color;
{   char buf;

	clipwin(side, win);
	y += 3;
	x += 2;
    if (utypes[u].bitmapname != NULL ) {
		put_pixmap(side, win, x, y, color, side->bgcolor, sd()->unitpics[u]);
    } else {
		buf = utypes[u].uchar;
		put_text(side, win, x, y, color, side->bgcolor, sd()->unitfont, &buf, 1);
    }
	
}	


draw_text(side, win, x, y, str, color)
Side *side;
Window win;
int x, y, color;
char *str;
{	int w, h;
	int bg;

	clipwin(side, win);
	wr_mode = 0;

	if (color == side->bgcolor)
    		bg = side->fgcolor;
    	else
    		bg = side->bgcolor;
    		
	if (win == side->help)
		put_text(side, win, x, y, color, bg, sd()->helpfont, str, strlen(str));
	else
		put_text(side, win, x, y, color, bg, sd()->textfont, str, strlen(str));

	wr_mode = 1;
}	


draw_scratchout(side, pos)
Side *side;
int pos;
{
	clipwin(side, side->sides);
	g_hline(pos+sw(side->sides, y), 0+sw(side->sides, x),
		30*side->fw + sw(side->sides, x), side->fgcolor);
}	


beep(side)
Side *side;
{
	putchar('\007');
}	


reveal_help(side)
Side *side;
{
	sd()->help_on = 2;
	return TRUE;
}	


conceal_help(side)
Side *side;
{
	sd()->help_on = 3;
}	


dump_view(side)
Side *side;
{
	notify(side, "dump view not implemented.");
}	


close_display(side)
Side *side;
{
	g_mode(GTEXT);
}	

