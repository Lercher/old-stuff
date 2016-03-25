/* graph.h (emx+gcc) */

#if !defined (_GRAPH_H)
#define _GRAPH_H

#if defined (__cplusplus)
extern "C" {
#endif

/* g_mode() and g_modeset() arguments (MODE) */

#define GTEXT          0x003

#define G320x200x256   0x013
#define G640x400x256   0x100
#define G640x480x256   0x101
#define G800x600x256   0x103
#define G1024x768x256  0x105
#define G1280x1024x256 0x107

#define G320x200x32k   0x10d
#define G320x200x64k   0x10e
#define G320x200x16m   0x10f

#define G640x480x32k   0x110
#define G640x480x64k   0x111
#define G640x480x16m   0x112

#define G800x600x32k   0x113
#define G800x600x64k   0x114
#define G800x600x16m   0x115

#define G1024x768x32k  0x116
#define G1024x768x64k  0x117
#define G1024x768x16m  0x118

#define G1280x1024x32k 0x119
#define G1280x1024x64k 0x11a
#define G1280x1024x16m 0x11b

#define G512x768x32k   0x200
#define G512x768x64k   0x201
#define G341x768x16m   0x202

#define G_MODE_OFF     GTEXT
#define G_MODE_VGA_L   G320x200x256
#define G_MODE_VGA_S   G640x480x256
#define G_MODE_VGA_M   G800x600x256
#define G_MODE_VGA_H   G1024x768x256
#define G_MODE_VGA_32k G800x600x32k
#define G_MODE_VGA_64k G800x600x64k
#define G_MODE_VGA_16m G640x480x16m

#define G_SET_KEEP    0         /* No change         */

/* g_wmode() arguments */

#define G_NORM 0                /* Normal write mode */
#define G_AND  1                /* `AND' mode        */
#define G_OR   2                /* `OR' mode         */
#define G_XOR  3                /* `XOR' mode        */

/* Fill or draw outline */

#define G_OUTLINE      0             /* Draw outline */
#define G_FILL         1             /* Fill         */

/* EGA colors */

#define G_BLACK        0
#define G_BLUE         1
#define G_GREEN        2
#define G_CYAN         3
#define G_RED          4
#define G_MAGENTA      5
#define G_YELLOW       6
#define G_BROWN        6
#define G_WHITE        7

#define G_INTENSITY    8

/* HI-/TRUE-Colors   */

#define RGB15(r,g,b)   ((((b) & 0xf8) >> 3) | (((g) & 0xf8) << 2) | (((r) & 0xf8) << 7))
#define RGB16(r,g,b)   ((((b) & 0xf8) >> 3) | (((g) & 0xfc) << 3) | (((r) & 0xf8) << 8))
#define RGB24(r,g,b)   (( (b) & 0xff)       | (((g) & 0xff) << 8) | (((r) & 0xff) << 16))


/* Screen dimensions */

extern int g_xsize;             /* Screen width             */
extern int g_ysize;             /* Screen height            */
extern int g_colors;            /* Number of colors         */
extern int g_memsize;           /* Size of Display Buffer   */

void g_clear(int color);
void g_box (int x0, int y0, int x1, int y1, int color, int fill_flag);
void g_clip (int x0, int y0, int x1, int y1);
void g_ellipse (int cx, int cy, int rx, int ry, int color, int fill_flag);
int  g_get(int x, int y);
void g_getimage(int x0, int y0, int x1, int y1, void *graph_buf);
void g_getpal (char *pal, int first, int n);
void g_hline(int y, int x0, int x1, int color);
int  g_imagesize(int width, int height);
void g_line(int x0, int y0, int x1, int y1, int color);
void g_lock (void);
int  g_mode (int mode);
int  g_modeset (int mode, int flag);
void g_polygon (__const__ int *x, __const__ int *y, int n, int color,
                int fill_flag);
void g_putimage(int x0, int y0, int x1, int y1, void *graph_buf);
void g_restore(void *buf);
void g_save(void *buf);
void g_set(int x, int y, int color);
void g_setpal (const char *pal, int first, int n, int wait_flag);
void g_triangle (int x0, int y0, int x1, int y1, int x2, int y2, int color,
                 int fill_flag);
void g_unlock (void);
void g_unlockall (void);
void g_vline(int x, int y0, int y1, int color);
void g_waitv (void);
void g_wmode (int wmode);

int  g_virtual(int copy_buf, int copy_pal);
void g_refresh(int x0, int y0, int x1, int y1, int x2, int y2, int pal);
void g_real(int copy_buf, int copy_pal);

#define g_vgapal g_setpal

#if defined (__cplusplus)
}
#endif

#endif /* !defined (_GRAPH_H) */
