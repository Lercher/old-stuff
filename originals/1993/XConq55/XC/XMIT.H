/* xmit.h - Definition of Messages 
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
	X_OPEN_DISPLAY,
	X_PUT_PIXMAP,
	X_PUT_TEXT,
	X_CLIP,
	X_SETPAL,
	X_LOAD_BOMB,
	X_BOX,
	X_FREESE_WAIT,
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
	X_FLUSH_INPUT
};

enum repl {
	Y_NONE,
	Y_GEOMETRY,
	Y_MISCDATA,
	Y_FONT,
	Y_PIXMAP,
	Y_MOUSE_EVENT,
	Y_KEYBOARD_EVENT,
	Y_FLUSH_INPUT
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

struct pal {
	int co;
	unsigned char buf[3];
};

struct lp { /* load_pixmap */
	enum lpmode mode;
	char name[128];
};

union data { /* transmit data */
	struct pm pm;
	struct pt pt;
	struct pal pal;
	struct lp lp;
	int i[10];
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

struct event {
	int ch, x, y;
}

union rdata { /* reply data */
	struct od od;
	struct lb lb;
	struct font font;
	struct event event;
	Pixmap *pixmap;
	int escape;
};

struct reply {
	enum repl type;
	union rdata data;
};
