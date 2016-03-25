/* sconq.c - A graphic slave to the master mconq
 *
 * (c) 1993 by Martin Lercher
 *
 */

#include "vesa/graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <assert.h>
#include <time.h>

#define OPENING "%s: Waiting for Connection (^C to cancel) ...\n", host

/* transmit.h goes here */
/* Definition of Messages 
 * for master - slave Communication
 */

typedef struct a_Pixmap {
	short w, h;
	unsigned char *bytes; /* linksbuendig */	
} Pixmap;

typedef struct a_Font {
	short w, h;
	Pixmap pixels[256];
} Font;

enum xmit {
	X_EVENT,
	X_OPEN_DISPLAY,
	X_PUT_PIXMAP,
	X_PUT_TEXT,
	X_CLIP,
	X_SETPAL,
	X_BOX,
	X_INVERT_V,
	X_INVERT_H,
	X_INV_LINE,
	X_INVERT_WHOLE_MAP,
	X_NAP,
	X_BEEP,
	X_HLINE,
	X_CLOSE_DISPLAY,
	X_LOAD_FONT,
	X_LOAD_PIXMAP,
	X_LOAD_BOMBS,
	X_FREEZE_WAIT
};

enum repl {
	Y_NONE,
	Y_GEOMETRY,
	Y_MISCDATA,
	Y_FONT,
	Y_PIXMAP,
	Y_FREEZE_WAIT,
	Y_CLOSE_DISPLAY,
	Y_EVENT
};

enum lpmode {
	LP_OPEN,
	LP_LOAD,
	LP_CLOSE
};

struct pm { /* put_pixmap */
	int x, y, color, bgcolor, wr_mode;
	Pixmap *pixmap;
};

struct pt { /* put_text */
	int x, y, color, bgcolor, wr_mode, len;
	Font *font;
	char buf[128];
};

struct lp { /* load_pixmap */
	enum lpmode mode;
	char name[128];
};

union data { /* transmit data */
	struct pm pm;
	struct pt pt;
	struct lp lp;
	int i[6];
};

struct Xmit {
	enum xmit type;
	union data data;
};


/* reply */

struct od { /* open_display */
	int xsize, ysize, colors;
};

struct lb { /* load_bombs */
	Pixmap *bombpics;
	int showmode;
};

struct font { /* load font */
	Font *font;
	int w, h;
};

enum event_type { EV_EMPTY, EV_MS, EV_KBD };

struct event {
	enum event_type type;
	int ch, x, y;
};

union rdata { /* reply data */
	struct od od;
	struct lb lb;
	struct font font;
	struct event event;
	Pixmap *pixmap;
	int i;
};

struct reply {
	enum repl type;
	union rdata data;
};
/* ende transmit.h */

#ifndef MASTER
# define MASTER 0
#endif

#if !MASTER
static char *host, in[100], out[100];
#endif

static struct reply Reply;
static handleReply;

/* mouse and keyboard events */
enum { PLACE, REMOVE };
enum { ON, OFF } mouse;
static mouse_flags, mouse_pos_x, mouse_pos_y, mouse_initialized = 0;
struct event Event; 
int cx=0, cy=0, cw=1, ch=1;
/* events end */

static struct bombPixmap
{ unsigned char bytes[4*32]; /* 32x32 */ }
bombPics[4] = {  /* mushroom clouds */ 
{{
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
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00}},
{{
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
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00}},
{{
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
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00}},
{{
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
   0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00}}
};


/* mouse routines */
#define mouse_w 11
#define mouse_h 11
#define mouse_x 5
#define mouse_y 5
static unsigned char mouse_q[mouse_w][mouse_h] = {
{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1 },
{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
{ 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 }
};

static unsigned char mouse_p[mouse_w][mouse_h] = {
{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
{ 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0 },
{ 0, 0, 1, 2, 1, 2, 1, 2, 1, 0, 0 },
{ 0, 1, 2, 1, 1, 2, 1, 1, 2, 1, 0 },
{ 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1 },
{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
{ 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1 },
{ 0, 1, 2, 1, 1, 2, 1, 1, 2, 1, 0 },
{ 0, 0, 1, 2, 1, 2, 1, 2, 1, 0, 0 },
{ 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0 },
{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 }
};

static void mouse_init(void)
{	union REGS in, out;

	in.x.ax = 0;
	_int86(0x33, &in, &out);
	in.x.ax = 7;
	in.x.cx = 0;
	in.x.dx = g_xsize;
	_int86(0x33, &in, &out);
	in.x.ax = 8;
	in.x.cx = 0;
	in.x.dx = g_ysize;
	_int86(0x33, &in, &out);
	mouse_initialized = 1;
}

static int mouse_pos(int *x, int *y)
{	union REGS in, out;

	in.x.ax = 3;
	_int86(0x33, &in, &out);
	*x=out.x.cx; *y=out.x.dx;
	return out.x.bx;
}


static void mouse_do(int mode, int x, int y)
{	int a, b;
static unsigned char save[mouse_w * mouse_h];

	x -= mouse_x;
	y -= mouse_y;
	
	g_clip(0, 0, g_xsize, g_ysize);
	if (mode == PLACE)
	{
		g_getimage(x, y, x+mouse_w-1, y+mouse_h-1, save);
		if (Event.type == EV_EMPTY)
		{
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
			for(a=0; a<mouse_h; a++)
			{
				for(b=0; b<mouse_w; b++)
				{
					if (mouse_p[a][b])
						g_set(x+a, y+b, mouse_q[a][b]);
				}
			}
		}
	}
	else
	{
		g_putimage(x, y, x+mouse_w-1, y+mouse_h-1, save);
	}
	g_clip(cx, cy, cw, ch);
}

#define mx mouse_pos_x
#define my mouse_pos_y
#define mf mouse_flags
static void mouse_show(int on)
{
	if (!mouse_initialized)
		return;
	if (on && mouse == OFF)
	{
		mf = mouse_pos(&mx, &my) & 1;
		mouse_do(PLACE, mx, my);
		mouse = ON;
		return;
	}
	if (!on && mouse == ON)
	{
		mouse_do(REMOVE, mx, my);
		mouse = OFF;
	}
}

/* event routines */

static void check_mouse(void)
{	int f, x, y;

	if (!mouse_initialized)
		return;
	f = mouse_pos(&x, &y) & 1;
	if ((mx != x || my != y) && mouse == ON)
	{
		mouse_do(REMOVE, mx, my);
		mx = x;
		my = y;
		mouse_do(PLACE, mx, my);
	}	
	if (f != mf)
	{
		mf = f;
		if (mf & Event.type == EV_EMPTY)
		{
			Event.type = EV_MS;
			Event.x = mx;
			Event.y = my;
			mouse_do(REMOVE, mx, my);
			mouse_do(PLACE, mx, my);
		}
	}
}
#undef mx
#undef my
#undef mf
/* end event */

static open_here=0;
static void Xclose_display(void)
{
	open_here = 0;
	mouse_initialized = 0;
	g_mode(GTEXT);
	Reply.type = Y_CLOSE_DISPLAY;
	handleReply = 1;
}	

static void Xopen_display()
{	int mode;

	if (open_here)
	{
		fprintf(stderr, "No second display!\n");
		exit(1);
	}
	open_here = 1;
	
	mode = G_MODE_VGA_L; /* 300x200x256 */
	if (getenv("VESAMODE"))
		mode = atoi(getenv("VESAMODE"));
	/* g_mode(0x212); / * 360x480x256 */
	if (!g_mode(mode)) 
	{
		printf("Cannot open graphics mode. This is fatal.\n");
		exit(1); /* this is fatal */
	}
	mouse_init();
	Event.type = EV_EMPTY;
	mouse = OFF;

	Reply.type = Y_GEOMETRY;
	Reply.data.od.xsize = g_xsize;
	Reply.data.od.ysize = g_ysize;
	Reply.data.od.colors = g_colors;
	assert(handleReply == 0);
	handleReply = 1;
}


static void Xput_pixmap(struct pm *pm)
{	int color, bgcolor, x, y, w, h, ww, i, j, len, wr_mode;
	unsigned bit;
	unsigned char *p;
	Pixmap *pixmap;


	
	x = pm->x;
	y = pm->y;
	pixmap = pm->pixmap;
	wr_mode = pm->wr_mode;
	color = pm->color;
	bgcolor = pm->bgcolor;
	
	if (!pixmap->bytes)
		return;
	w = pixmap->w;
	h = pixmap->h;
	ww = (w+7)>>3;
	len = w*h;
	assert(len < 50*50);
	if (wr_mode)
	{
		p = pixmap->bytes;
		for(j=0; j<h; j++)
		{	unsigned t=0;
		
			bit = 1;
			for(i=0; i<w; i++)
			{	

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
		{	unsigned t=0;

			bit = 1;
			for(i=0; i<w; i++)
			{

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
}

static void Xput_text(struct pt *pt)
{	struct pm pm;

	int x, y, len;
	Font *font;
	char *buf;
	int xx;

	x = pt->x;
	y = pt->y;
	pm.wr_mode = pt->wr_mode;
	pm.color = pt->color;
	pm.bgcolor = pt->bgcolor;
	
	font = pt->font;
	buf = pt->buf;
	len = pt->len;
	
	xx = x;
	while(len--)
	{
		if (*buf != '\n')
		{
			pm.x = x;
			pm.y = y + font->h - font->pixels[(int)*buf].h;
			pm.pixmap = &(font->pixels[(int)*buf]);
			Xput_pixmap(&pm);
/*
			put_pixmap(side, win, x, y + font->h - font->pixels[(int)*buf].h,
				c, bc, &(font->pixels[(int)*buf]));
*/
			x += font->w;
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

static void Xinvert_v(int x, int y, int y2)
{
	while(y<=y2)
	{
		g_set(x, y, ~g_get(x, y));
		y++;
	}
}


static void Xinvert_h(int y, int x, int x2)
{
	while(x<=x2)
	{
		g_set(x, y, ~g_get(x, y));
		x++;
	}
}

static void Xinv_line(int x, int y, int x1, int y1)
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


static void Xnap(int t)
{	int n;

	if (t>=1000)
		sleep(t/1000);
	else
		for(n=0; n<t*1000; n++)
			/**/;
}


static void Xinvert_whole_map(void)
{	char buf[3];

	g_getpal(buf, 0, 1);
	g_setpal("\0\0\0", 0, 1, 1);
	Xnap(25);
	g_setpal("\077\0\0", 0, 1, 1);
	Xnap(25);
	g_setpal("\0\077\0", 0, 1, 1);
	Xnap(25);
	g_setpal("\0\0\077", 0, 1, 1);
	Xnap(25);
	g_setpal("\077\077\077", 0, 1, 1);
	Xnap(25);
	g_setpal(buf, 0, 1, 1);
}	



static void Xload_bombs(void)
{	int i;
	Pixmap *bombs;
	
	assert(handleReply == 0);
	handleReply = 1;
	Reply.type = Y_MISCDATA;
	Reply.data.lb.bombpics = bombs = calloc(sizeof(Pixmap), 4);

	for(i=0; i<4; i++)
	{
		bombs[i].w = 32;
		bombs[i].h = 32;
		bombs[i].bytes = bombPics[i].bytes;
	}
	Reply.data.lb.showmode = 3; /* BothIcons */
}	



static void Xload_font(const char *fn)
{	FILE *f;
	Font *font;

	font = (Font *) calloc(sizeof(Font), 1);
	assert(font);
	f = fopen(fn, "rb");
	if (!f)
	{
		perror(fn);
		exit(1);
	}
	
	fread(&font->w, sizeof(font->w), 1, f);
	fread(&font->h, sizeof(font->h), 1, f);
	font->h++;
	
	while(!feof(f))
	{	short ch, w, h, ww;

	
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
	}
	fclose(f);

	assert(handleReply == 0);
	handleReply = 1;

	Reply.type = Y_FONT;
	Reply.data.font.font = font;
	Reply.data.font.w = font->w;
	Reply.data.font.h = font->h;
}


static void Xload_pixmap(struct lp *lp)
{	char buf[20], buf1[20], *s, *name;
	short w, h, ww;
static FILE *f;

	Reply.type = Y_PIXMAP;
switch(lp->mode)
{
case LP_OPEN:
	f = fopen(lp->name, "rb");
	if (!f)
	{
		perror(lp->name);
		exit(1);
	}
	break;
case LP_LOAD:
	assert(handleReply == 0);
	handleReply = 1;

	name = lp->name;
	strncpy(buf1, name, 8);
	buf1[8]=0;
	for(s = buf1; *s; s++)
		*s = tolower(*s);
	strcat(buf1, ".b");
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
			Reply.data.pixmap = p;
			return;
		}
		fseek(f, ww, SEEK_CUR);
	}
	fprintf(stderr, "Bitmap %s not found\n", name);
	Reply.data.pixmap = NULL;
	break;
case LP_CLOSE:
	fclose(f);
	break;
}
}

static void Xevent(void)
{	char ch;

	check_mouse();
	if (Event.type == EV_EMPTY)
	{
		if (-1 != (ch = _read_kbd(0,0,0)))
		{
			if (ch == 0)
				ch = getch();
			Event.type = EV_KBD;
			Event.ch = ch;
		}
	}
	Reply.type = Y_EVENT;
	Reply.data.event = Event;
	assert(handleReply == 0);
	handleReply = 1;
	Event.type = EV_EMPTY;	/* 1 cell queue cleared */
}



static void process(struct Xmit *X)
{
	unsigned char buf[3];
	
	check_mouse();
	switch(X->type)
	{
	case X_EVENT:
		mouse_show(1);
		Xevent();
		break;
	case X_OPEN_DISPLAY:
		Xopen_display();
		break;
	case X_PUT_PIXMAP:
		Xput_pixmap(&X->data.pm);
		break;
	case X_PUT_TEXT:
		Xput_text(&X->data.pt);
		break;
	case X_CLIP:
		mouse_show(0);
		g_clip(cx=X->data.i[0], cy=X->data.i[1], cw=X->data.i[2], ch=X->data.i[3]);
		break;
	case X_SETPAL:
		buf[0] = X->data.i[0];
		buf[1] = X->data.i[1];
		buf[2] = X->data.i[2];
		g_setpal(buf, X->data.i[3], 1, 1);
		break;
	case X_LOAD_BOMBS:
		Xload_bombs();
		break;
	case X_BOX:
		mouse_show(0);
		g_box(X->data.i[0], X->data.i[1], X->data.i[2], X->data.i[3], X->data.i[4], X->data.i[5]);
		break;
	case X_FREEZE_WAIT:
		mouse_show(0);
		Reply.data.i = getch();
		handleReply = 1;
		break;
	case X_INVERT_V:
		Xinvert_v(X->data.i[0], X->data.i[1], X->data.i[2]);
		break;
	case X_INVERT_H:
		Xinvert_h(X->data.i[0], X->data.i[1], X->data.i[2]);
		break;
	case X_INV_LINE:
		Xinv_line(X->data.i[0], X->data.i[1], X->data.i[2], X->data.i[3]);
		break;
	case X_INVERT_WHOLE_MAP:
		Xinvert_whole_map();
		break;
	case X_NAP:
		Xnap(X->data.i[0]);
		break;
	case X_BEEP:
		putchar('\007');
		break;
	case X_HLINE:
		g_hline(X->data.i[0], X->data.i[1], X->data.i[2], X->data.i[3]);
		break;
	case X_CLOSE_DISPLAY:
		mouse_show(0);
		Xclose_display();
#if !MASTER
		printf(OPENING);
#endif
		break;
	case X_LOAD_FONT:
		Xload_font(X->data.lp.name);
		break;
	case X_LOAD_PIXMAP:
		Xload_pixmap(&X->data.lp);
		break;
	default:
		fprintf(stderr, "Invalid message type. Aborting ...\n");
		exit(1);
		break;
	}
	check_mouse();
}

static FILE *waitfor(const char *fn)
{	FILE *f;

	f = NULL;
	while(!f)
	{
		check_mouse();
		f = fopen(fn, "rb");
	}
	return f;
}

static void post(const char *from, const char *to)
{
	while(rename(from, to)) /* 0 is succes */
	{
		check_mouse();
	}	
}





#if !MASTER
static void loop(void)
{	FILE *f;
	struct Xmit X;
	
	for(;;)
	{
		f = waitfor(in);
		handleReply=0;
		while(fread(&X, sizeof(X), 1, f))
			process(&X);
		f = freopen(in, "wb", f);
		fwrite(&Reply, sizeof(struct reply), handleReply, f);
		handleReply = 0;
		fclose(f);
		post(in, out);
	}
}



int main(int argc, char **argv)
{
	printf("This is SConq 1.0, a Graphic Slave to MConq\n");
	host = NULL;
	if (argc==2)
	{
		host = argv[1];
	}
	else
	{
		host = getenv("NAME");
	}
	if (!host)
	{
		printf("Please state Hostname via\n\t1. SConq <name>\n2. set NAME=<name>\n\t   SConq\n");
		return 1;
	}

	printf(OPENING);

	strcpy(in, host);	strcat(in, ".s");
	strcpy(out, host);	strcat(out, ".m");
	
	loop();
	return 0;
}
#endif /* !MASTER */






#if MASTER

#include <signal.h>

#include "config.h"
#include "misc.h"
#include "period.h"
#include "global.h"
#include "side.h"
#include "map.h"

#define MAXWINDOWS 12
#define MAXRGB 1024 /* Min 750 */

#define INFOLINES 5
#define BH 32

int hw = 20;
int hh = 22;
int hch = 17;
int margin = 2;
int bd = 1;

typedef long Window;  /* a window handle */
struct aWindow {
	int x, y, w, h;
}; 

char *routine_executing = "Program Start";

int helpwinlines = 1; /* size of welp window */

#define sd() ((Screeno *) side->display)
#define sw(win, ext) sd()->wins[win].ext

enum help_mode { H_ON, H_OFF };

typedef struct a_screen {
    enum help_mode help_on;	/* is help window visible? has 4 states! */
    short sending;
    short xsize, ysize, colors;
    char *in, *out;     /* in and out filenames, in==NULL if localhost */
    FILE *file;		/* the output file for communic */
    Font *textfont;	/* font for text display */
    Font *iconfont;	/* utility font with assorted icons */
    Font *unitfont;	/* font for unit characters */
    Font *helpfont;      /* font for help window */
    Pixmap *unitpics[MAXUTYPES];	/* used instead of font sometimes */
    Pixmap *bombpics; /* the mushrooms */
    struct aWindow wins[MAXWINDOWS];
    Window lastwindow, clipwin;
} Screeno;


typedef struct a_rgb {
	unsigned char r, g, b, pixel;
	char *name;
} RGB;

static RGB rgb[MAXRGB];
static lastrgb;
static wr_mode;
static struct Xmit X;

static void Xflush(Side *side)
{
	if (sd()->file == NULL)
		return;
	fclose(sd()->file);
	sd()->file = NULL;
	post(sd()->in, sd()->out);
}

active_display(side)
Side *side;
{
	return (side && side->host && !side->lost && side->display);
}


static void send(Side *side, struct Xmit *X, enum repl wait)
{
	if (!active_display(side) || !sd()->sending)
		return;
	if (sd()->sending++ > (32500/sizeof(struct Xmit)))
	{
		sd()->sending = 1;
		Xflush(side);
	}
	if (sd()->in == NULL)
	{
		process(X);
		assert(wait == Y_NONE || wait == Reply.type);
		handleReply = 0;
	}
	else
	{
		if (sd()->file == NULL)
		{	FILE *f;
			f = waitfor(sd()->in); /* last post pending */
			sd()->file = freopen(sd()->in, "wb", f); /* clear it */
			assert(sd()->file);
		}
		fwrite(X, sizeof(*X), 1, sd()->file);
		if (wait != Y_NONE)
		{	FILE *f;

			Xflush(side);
			f = waitfor(sd()->in);
			fread(&Reply, sizeof(Reply), 1, f);
			assert(wait == Reply.type);
			sd()->file = freopen(sd()->in, "wb", f);
		}
	}
}


static void put_pixmap(Side *side, Window win, int x, int y, int color, int bgcolor, Pixmap *pixmap)
{
	if (!pixmap)
		return;
	X.type = X_PUT_PIXMAP;
	X.data.pm.x = x + sw(win, x);
	X.data.pm.y = y + sw(win, y);
	X.data.pm.wr_mode = wr_mode;
	X.data.pm.color = color;
	X.data.pm.bgcolor = bgcolor;
	X.data.pm.pixmap = pixmap;
	send(side, &X, Y_NONE);
}

static void put_text(Side *side, Window win, int x, int y,
	int c, int bc, Font *font, char *buf, int len)
{
	if (!font)
		return;

	if (sd()->help_on == H_ON && win != side->help && win != side->clock)
		return;
		
	X.type = X_PUT_TEXT;
	X.data.pt.x = x + sw(win, x);
	X.data.pt.y = y + sw(win, y);
	X.data.pt.wr_mode = wr_mode;
	X.data.pt.color = c;
	X.data.pt.bgcolor = bc;
	X.data.pt.font = font;
	X.data.pt.len = len;
	strncpy(X.data.pt.buf, buf, len);
	send(side, &X, Y_NONE);
}


static void clipwin(Side *side, Window win)
{	int x,y,w,h;

	if (sd()->clipwin == win)
		return;
	sd()->clipwin = win;

	x = sw(win, x);
	y = sw(win, y);
	w = sw(win, w);
	h = sw(win, h);

	X.type = X_CLIP;
	X.data.i[0] = x;
	X.data.i[1] = y+1;
	X.data.i[2] = x+w-1;
	X.data.i[3] = y+h-1;
	send(side, &X, Y_NONE);
}

long request_color(side, name, class, fallback)
Side *side;
char *name, *class, *fallback;
{
static int co=0;
int i;

	for(i=0; i<lastrgb; i++)
	{
		if (0 == stricmp(fallback, rgb[i].name))
		{	int ac;

			if (rgb[i].pixel)
				ac = rgb[i].pixel;
			else
				rgb[i].pixel = ac = co++;
			if (co>=0xff)
				break;
			X.type = X_SETPAL;
			X.data.i[0] = rgb[i].r >> 2;
			X.data.i[1] = rgb[i].g >> 2;
			X.data.i[2] = rgb[i].b >> 2;
			X.data.i[3] = ac;
			send(side, &X, Y_NONE);
			return ac;
		}
	}
	return 2;
}	

add_default_player()
{
	char *myname;

	myname = getenv("NAME");
	if (myname == NULL)
		myname = "Unknown";
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

	if (lastrgb) /* load this only once in the network */
		return;
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
{	char *name;

	side->display = (long) calloc(sizeof(struct a_screen), 1);
	assert(side->display);

	load_rgb();
	name = getenv("NAME");
	if (!name)
		name = "Unknown";
	if (0 == stricmp(side->host, name)) /* local host */
	{
		sd()->in = NULL;
		sd()->out = NULL;
	}
	else /* remot host */
	{	int len;

		len = strlen(side->host);
		sd()->in  = malloc(len + sizeof(".?"));
		sd()->out = malloc(len + sizeof(".?"));
		strcpy(sd()->in,  side->host);
		strcpy(sd()->out, side->host);
		strcat(sd()->in,  ".m");
		strcat(sd()->out, ".s");
		unlink(sd()->out);
		sd()->file = fopen(sd()->in, "wb"); /* generate first "token" */
	}
	sd()->clipwin = -1;
	sd()->help_on = H_OFF;
	sd()->sending = 1;
	
	X.type = X_OPEN_DISPLAY;
	send(side, &X, Y_GEOMETRY);
	sd()->xsize = Reply.data.od.xsize;
	sd()->ysize = Reply.data.od.ysize;
	sd()->colors = Reply.data.od.colors;

	std_color(side);
	return 1;
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


static Font *load_font(Side *side, char *fn, short *fw, short *fh)
{

	
	if (Debug) printf("Loading %s ... ", fn);
	X.type = X_LOAD_FONT;
	make_pathname(xconqlib, fn, NULL, X.data.lp.name);
	send(side, &X, Y_FONT);
	*fw = Reply.data.font.w;
	*fh = Reply.data.font.h;
	if (Debug) printf("Done.\n");
	return Reply.data.font.font;
}


static Pixmap *load_pixmap(enum lpmode mode, Side *side, char *name)
{
	X.type = X_LOAD_PIXMAP;
	X.data.lp.mode = mode;
	strcpy(X.data.lp.name, name);
	send(side, &X, mode == LP_LOAD ? Y_PIXMAP : Y_NONE);
	return Reply.data.pixmap;
}


void init_misc(side)
Side *side;
{
	int u;

    side->margin = margin;
    side->bd = bd;
    side->hw = hw;
    side->hh = hh;
    side->hch = hch;

	sd()->helpfont = load_font(side, "HelpFont", &(side->hfw), &(side->hfh));
	sd()->textfont = load_font(side, "TextFont", &(side->fw), &(side->fh));
	sd()->iconfont = load_font(side, "IconFont", &(side->hw), &(side->hh));
	if (period.fontname != NULL && strlen(period.fontname) > 0)
		sd()->unitfont = load_font(side, period.fontname, &(side->uw), &(side->uh));
	else
	      	sd()->unitfont = load_font(side, "Standard", &(side->uw), &(side->uh));

	make_pathname(xconqlib, "pixmaps", NULL, spbuf);
	(void) load_pixmap(LP_OPEN, side, spbuf);
	for_all_unit_types(u) 
	{
		if (utypes[u].bitmapname && (strlen(utypes[u].bitmapname) > 0)) 
		{
			sd()->unitpics[u] = load_pixmap(LP_LOAD, side, utypes[u].bitmapname);
		} else {
			utypes[u].bitmapname = NULL;
		}
    	}
	(void) load_pixmap(LP_CLOSE, side, spbuf);

	X.type = X_LOAD_BOMBS;
	send(side, &X, Y_MISCDATA);
	sd()->bombpics = Reply.data.lb.bombpics;
	side->showmode = Reply.data.lb.showmode; /* BothIcons */

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
}	


void get_color_resources(side)
Side *side;
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

void set_retain(side, window)
Side *side;
long window;
{
	/* ignore this */
}	


static void Xg_box(Side *side, int x, int y, int x1, int y1, int c, int mode)
{
	X.type = X_BOX;
	X.data.i[0] = x;
	X.data.i[1] = y;
	X.data.i[2] = x1;
	X.data.i[3] = y1;
	X.data.i[4] = c;
	X.data.i[5] = mode;
	send(side, &X, Y_NONE);
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
		Xg_box(side, x-1,y,x+w,y+h, side->fgcolor, G_OUTLINE);
	}
}


void fixup_windows(side)
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
	
}	


void enable_input(side)
Side *side;
{

}	

clear_window(side, win)
Side *side;
Window win;
{	int x,y,w,h;

	if (sd()->help_on == H_ON && win != side->help && win != side->clock)
		return;
		
	map_window(side, win);

	x = sw(win, x);
	y = sw(win, y);
	w = sw(win, w);
	h = sw(win, h);
	Xg_box(side, x,y+1,x+w-1,y+h-1, side->bgcolor, G_FILL);
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
}	


display_colors(side)
Side *side;
{
	return sd()->colors;
}	


static int grey_color(Side *side)
{
	return 0;
}	


white_color(side)
Side *side;
{
	return 2;
}	


black_color(side)
Side *side;
{
	return 1;
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

static int get_event(Side *side)
{	int rawx, rawy;
	Window window;

	X.type = X_EVENT;
	send(side, &X, Y_EVENT);
	
	side->reqtype = GARBAGE;

	switch(Reply.data.event.type)
	{
	case EV_KBD:
		side->reqtype = KEYBOARD;
		side->reqch = Reply.data.event.ch;
		return 1;
	case EV_MS:
		window = get_window(side, Reply.data.event.x, Reply.data.event.y, &rawx, &rawy);
		if (window == side->map) {
			side->reqtype = MAPPOS;
			side->reqch = 1;
			/* the modifier is used by x_command */
			deform(side, rawx, rawy, &(side->reqx), &(side->reqy));
			if (Debug) printf("Host %s returns map %d %d\n",
		      		side->host, side->reqx, side->reqy);
		      	return 1;
		} else if (window == side->world) {
			side->reqtype = MAPPOS;
			w_deform(side, rawx, rawy, &(side->reqx), &(side->reqy));
			if (Debug) printf("Host %s returns world %d %d\n",
				side->host, side->reqx, side->reqy);
			return 1;
		} else if (window == side->state) {
			side->reqtype = UNITTYPE;
			side->requtype = rawy / max(side->hh, side->fh);
			if (Debug) printf("Host %s returns unit type %d\n",
				side->host, side->requtype);
			return 1;
		} else if (window == side->msg) {
			if (rawy < side->nh * side->fh / 2)
				side->reqtype = SCROLLUP;
			else side->reqtype = SCROLLDOWN;
			return 1;
		}
		return 0;
	case EV_EMPTY:
		return 0;
	default:
		assert(0);
		break;
	}
	return 0;
}


get_input()
{	Side *side;
	int evt, n;

for(n=0; n<10; n++)
{
	evt = 0;
	for_all_sides(side)
	{
		int delta_time = (time(0) - side->lasttime);

		side->lasttime = time(0);
		check_mouse();
		if (side->more_units && delta_time)
		{
			side->timeleft -= delta_time;
			if (side->timeleft<0)
				side->timeleft = 0;
			side->timetaken += delta_time;
			if (delta_time)
				update_clock(side);
  		}
	}

	for_all_sides(side)
	{
		check_mouse();
		if (side->reqactive && active_display(side))
		{
			side->redisplay_ok = TRUE;
       			evt |= get_event(side);
			if (sd()->help_on == H_ON)
				x_help(side);
		}
	}
	if (evt)
		break;
}
	return TRUE;
}	


/* Allow a player to exit robot mode, even if he is the only player. */
void exit_robot_check()
{
  Side *side;
  
  for_all_sides(side)
  {
    if (active_display(side))
    {
        side->redisplay_ok = TRUE;
 	if (!humanside(side) && side->display)
	{
	  get_event(side);
 	  if (side->reqtype == KEYBOARD && side->reqch == '\033')
	  {
		side->humanp = TRUE;
	  	numhumans++;
	  	return;
	  }
	}
    } 
  }
}




void flush_input(side)
Side *side;
{
	if (humanside(side) && active_display(side))
	{
		while (get_event(side))
			/* */;
	}
	side->reqtype = GARBAGE;
	side->reqchange = TRUE;
}	


void flush_output(side) 
Side *side; 
{
	if (humanside(side) && active_display(side))
		Xflush(side);
	side->reqchange = TRUE;
}	


int freeze_wait(side)
Side *side;
{
	if (active_display(side))
	{
		flush_input(side);
		X.type = X_FREEZE_WAIT;
		send(side, &X, Y_FREEZE_WAIT);
		return Reply.data.i;
	}
	return '\0';
}	


/* Draw a single horizontal constant-color bar on the world map.  If part */
/* would not be drawn because of the map's obliqueness, cut it in two and */
/* wrap one of the pieces around. */

typedef unsigned int unint;

void draw_bar(side, x, y, len, color)
Side *side;
int x, y, len, color;
{
    int sx1, sx2, sy, sww;

    w_xform(side, x, y, &sx1, &sy);
    w_xform(side, x + len, y, &sx2, &sy);
    sww = side->mm * world.width;
    clipwin(side, side->world);
    if (sx1 < sww && sx2 >= sww) {
    	Xg_box(side, sx1+sw(side->world, x)  , sy           +sw(side->world, y), 
    	      sww+sw(side->world, x)-1, sy+ side->mm +sw(side->world, y)-1,
	      color, G_FILL);
    	Xg_box(side,     sw(side->world, x)  , sy           +sw(side->world, y), 
    	  sx2-sww+sw(side->world, x)-1, sy+ side->mm +sw(side->world, y)-1,
          color, G_FILL);
    } else {
		sx1 %= sww;
		sx2 %= sww;
		Xg_box(side, sx1+sw(side->world, x)  , sy         +sw(side->world, y), 
    		      sx2+sw(side->world, x)-1, sy+side->mm+sw(side->world, y)-1,
		      color, G_FILL);
    }
}	


static void invert_v(Side *side, int x, int y, int y2)
{
	X.type = X_INVERT_V;
	X.data.i[0] = x;
	X.data.i[1] = y;
	X.data.i[2] = y2;
	send(side, &X, Y_NONE);
}


static void invert_h(Side *side, int y, int x, int x2)
{
	X.type = X_INVERT_H;
	X.data.i[0] = y;
	X.data.i[1] = x;
	X.data.i[2] = x2;
	send(side, &X, Y_NONE);
}

/* Invert the outline box on the world map.  This is a little tricky, */
/* because we want the lines to run through the middle of the world's */
/* hexes, and because the line drawn should not overlap (or the overlaps */
/* will be doubly inverted and look strange). */

void invert_box(side, vcx, vcy)
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
	invert_h(side, sy1, sx1, sx2);
	invert_h(side, sy2, sx1, sx2);
	invert_v(side, sx1, sy2+1, sy1-1);
	invert_v(side, sx2, sy2+1, sy1-1);
}


void draw_terrain_row(side, sx, sy, buf, len, color)
Side *side;
int sx, sy, len, color;
char *buf;
{
	clipwin(side, side->map);
	put_text(side, side->map, sx, sy, color, side->bgcolor, sd()->iconfont, buf, len);
}	

/* Napping is like sleeping, but maybe shorter.  Arg is milliseconds. */

static void nap(Side *side, int t)
{
	X.type = X_NAP;
	X.data.i[0] = t;
	send(side, &X, Y_NONE);
}

static void inv_line(Side *side, int x, int y, int x1, int y1)
{
	X.type = X_INV_LINE;
	X.data.i[0] = x;
	X.data.i[1] = y;
	X.data.i[2] = x1;
	X.data.i[3] = y1;
	send(side, &X, Y_NONE);
}

/* Flash a pair of lines up, slow enough to draw the eye, but not so slow */
/* as to get in the way. */

void flash_position(side, sx, sy, tm)
Side *side;
int sx, sy, tm;
{    int sx1, sy1, sx2, sy2;
	
#define XDrawLine(a,win,b,x0,y0,x1,y1) inv_line(side,x0+sw(win, x),y0+sw(win, y),x1+sw(win, x),y1+sw(win, y))
    if (tm > 0) {
	clipwin(side, side->map);
	sx1 = sx - 50 + side->hw/2;  sy1 = sy + 50 + side->hch/2;
	sx2 = sx + 50 + side->hw/2;  sy2 = sy - 50 + side->hch/2;
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy1, sx2, sy2);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy2, sx2, sy1);
	flush_output(side);
	nap(side, tm);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy1, sx2, sy2);
	XDrawLine(sdd(), side->map, sd()->flashgc, sx1, sy2, sx2, sy1);
    }
#undef XDrawLine
}	

void draw_cursor_icon(side, sx, sy)
Side *side;
int sx, sy;
{
	if (sd()->help_on == H_ON)
		return;

	clipwin(side, side->map);
#if 0
	draw_text(side->map, sx, sy, side->bgcolor, sd()->iconfont, "[", 1);
	draw_text(side->map, sx, sy, side->fgcolor, sd()->iconfont, "]", 1);
#else
	Xg_box(side, 5+sx+sw(side->map, x), 5+sy+sw(side->map, y),
		sx+15+sw(side->map, x), sy+15+sw(side->map, y), side->fgcolor, G_OUTLINE);
#endif
}	


void draw_hex_icon(side, win, sx, sy, color, ch)
Side *side;
Window win;
int sx, sy, color;
char ch;
{
	clipwin(side, win);
	put_text(side, win, sx, sy, color, side->bgcolor, sd()->iconfont, &ch, 1);
}	


void draw_side_number(side, win, sx, sy, n, color)
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


void draw_blast_icon(side, win, sx, sy, type, color)
Side *side;
Window win;
int sx, sy, color;
char type;
{
	clipwin(side, win);
	put_text(side, win, sx, sy, color, side->bgcolor, sd()->iconfont, &type, 1);
}	


void invert_whole_map(side)
Side *side;
{
	X.type = X_INVERT_WHOLE_MAP;
	send(side, &X, Y_NONE);
}	


void draw_mushroom(side, x, y, i)
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


void draw_unit_icon(side, win, x, y, u, color)
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


void draw_text(side, win, x, y, str, color)
Side *side;
Window win;
int x, y, color;
char *str;
{
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


void draw_scratchout(side, pos)
Side *side;
int pos;
{
	clipwin(side, side->sides);
	X.type = X_HLINE;
	X.data.i[0] = pos+sw(side->sides, y);
	X.data.i[1] = 0+sw(side->sides, x);
	X.data.i[2] = 30*side->fw + sw(side->sides, x);
	X.data.i[3] = side->fgcolor;
	send(side, &X, Y_NONE);
}	


void beep(side)
Side *side;
{
	X.type = X_BEEP;
	send(side, &X, Y_NONE);
}	


int reveal_help(side)
Side *side;
{
	sd()->help_on = H_ON;
	return TRUE;
}	


void conceal_help(side)
Side *side;
{
	sd()->help_on = H_OFF;
	redraw(side);
}	


void dump_view(side)
Side *side;
{
	notify(side, "Dump-view not implemented.");
}	


void close_display(side)
Side *side;
{
	X.type = X_CLOSE_DISPLAY;
	send(side, &X, Y_CLOSE_DISPLAY);
	if (sd()->in)
		unlink(sd()->in);
	sd()->sending = 0;
}	

#endif
