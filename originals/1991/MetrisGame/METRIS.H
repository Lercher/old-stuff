/* @(#)TETRIS Header file */

#define	WIDTH		10		/* Anzahl K„stchen */
#define	HEIGHT		17
#define MAXLEVEL 	16
#define	XS		31
#define	YS		5


#define	WH		(WIDTH/2)
#define TRUE		1
#define	FALSE		0

typedef struct {
	signed char c;
	int vg, hg;
} ttyp;

typedef	ttyp	feld[WIDTH][HEIGHT];

char	rawin(void);
void	clear_feld(feld a);
void	cls(void);
void	eeol(void);
void	pfat(int erase, int fg, int hg, int x, int y, char *format, ...);
void	dstats(int index);
long	dpoints(int dp);
int	dlines(int dl);
int	dlevel(int dl);
void	rev(int i);
void	wechsel(void);
void	get_name(int update_flag);
int	main_menu(void);
void	rein(void);
void	raus(void);
void	tetris(void);
void	goto_xy(int x, int y);
void	compu(void);
void	display(void);
void 	user(void);
int	get(int x, int y, char *s);
void	at(int x, int y, ttyp *c);
void	rahmen(void);
char	set_pos(int piece,int x,int y,int r);
int	next(int *x, int *y);
int	fits(int piece, int x, int y, int rot);
void	enter(int piece, int x, int y, int rot);
void	fix(void);
void	delpic(void);
void	fer(void);
int	eintrag(long score, char *name);
void	friss(void);
void	show_score(int position);
void	pflegen(void);

#define foreach(a,b)	for(b=0 ; b<HEIGHT ; b++) for(a=0 ; a<WIDTH ; a++)

#define NAMELEN		14
#define HIGHSCORE "METRIS.HSC"

struct entry {
	struct entry *link;
	long score;
	char name[NAMELEN];
} *high_start, lastman = { NULL, 0L, "Last man" };

int	super, game_on, on_line, cp, cx, cy, cr;
char	player[NAMELEN] = "Tetris Player";

struct pd {
	int dx, dy;
};
static int  xofs, yofs, cnt;
static struct pd *pptr;
#define PICS	7
#define PICSE	7
#define ROTS	4
#define DOTS	4
#define DOTSE	1
static struct pd pics[PICS+PICSE][ROTS][DOTS+DOTSE] =
							/* Stck, Rots, dots */
#define E 1
#define M -1
#define N 0
/* Die Stcke drehen hier IM Uhrzeigersinn! */
{
	{
		{ 	{M,N}, {N,N}, {E,N}, {2,N}, {N,N}  },	/* 0 */
		{ 	{N,M}, {N,N}, {N,E}, {N,2}, {N,N}  },
		{ 	{M,N}, {N,N}, {E,N}, {2,N}, {N,N}  },
		{ 	{N,M}, {N,N}, {N,E}, {N,2}, {N,N}  },
	},
	{
		{ 	{M,M}, {N,M}, {M,N}, {N,N}, {N,N}  },	/* 1 */
		{ 	{M,M}, {N,M}, {M,N}, {N,N}, {N,N}  },
		{ 	{M,M}, {N,M}, {M,N}, {N,N}, {N,N}  },
		{ 	{M,M}, {N,M}, {M,N}, {N,N}, {N,N}  },
	},
	{
		{ 	{M,M}, {N,M}, {N,N}, {E,N}, {N,N}  },	/* 2 */
		{ 	{E,M}, {E,N}, {N,N}, {N,E}, {N,N}  },
		{ 	{M,M}, {N,M}, {N,N}, {E,N}, {N,N}  },
		{ 	{E,M}, {E,N}, {N,N}, {N,E}, {N,N}  },
	},
	{
		{ 	{M,N}, {N,N}, {N,M}, {E,M}, {N,N}  },	/* 3 */
		{ 	{N,M}, {N,N}, {E,N}, {E,E}, {N,N}  },
		{ 	{M,N}, {N,N}, {N,M}, {E,M}, {N,N}  },
		{ 	{N,M}, {N,N}, {E,N}, {E,E}, {N,N}  },
	},
	{
		{ 	{M,N}, {N,N}, {E,N}, {N,M}, {N,N}  },	/* 4 */
		{ 	{N,M}, {N,N}, {N,E}, {E,N}, {N,N}  },
		{ 	{M,N}, {N,N}, {E,N}, {N,E}, {N,N}  },
		{ 	{M,N}, {N,N}, {N,M}, {N,E}, {N,N}  },
	},
	{
		{ 	{M,N}, {N,N}, {E,N}, {E,M}, {N,N}  },	/* 5 */
		{ 	{N,M}, {N,N}, {N,E}, {E,E}, {N,N}  },
		{ 	{M,E}, {M,N}, {N,N}, {E,N}, {N,N}  },
		{ 	{M,M}, {N,M}, {N,N}, {N,E}, {N,N}  },
	},
	{
		{ 	{M,M}, {M,N}, {N,N}, {E,N}, {N,N}  },	/* 6 */
		{ 	{N,M}, {E,M}, {N,N}, {N,E}, {N,N}  },
		{ 	{M,N}, {N,N}, {E,N}, {E,E}, {N,N}  },
		{ 	{N,M}, {N,N}, {N,E}, {M,E}, {N,N}  },
	},
	{
		{ 	{M,M}, {M,N}, {N,N}, {E,N}, {E,M}  },	/* 7 */
		{ 	{E,M}, {N,M}, {N,N}, {N,E}, {E,E}  },
		{ 	{M,E}, {M,N}, {N,N}, {E,N}, {E,E}  },
		{ 	{M,M}, {N,M}, {N,N}, {N,E}, {M,E}  },
	},
	{
		{ 	{M,M}, {N,M}, {E,M}, {N,N}, {N,E}  },	/* 8 */
		{ 	{M,N}, {N,N}, {E,M}, {E,N}, {E,E}  },
		{ 	{N,M}, {N,N}, {M,E}, {N,E}, {E,E}  },
		{ 	{M,M}, {M,N}, {M,E}, {N,N}, {E,N}  },
	},
	{
		{ 	{M,N}, {N,N}, {N,E}, {E,N}, {E,M}  },	/* 9 */
		{ 	{M,N}, {N,M}, {N,N}, {N,E}, {E,E}  },
		{ 	{M,E}, {M,N}, {N,N}, {N,M}, {E,N}  },
		{ 	{M,M}, {N,M}, {N,N}, {N,E}, {E,N}  },
	},
	{
		{ 	{M,M}, {M,N}, {N,N}, {N,E}, {E,N}  },	/* 10 */
		{ 	{M,N}, {N,M}, {N,N}, {N,E}, {E,M}  },
		{ 	{M,N}, {N,N}, {N,M}, {E,N}, {E,E}  },
		{ 	{M,E}, {N,E}, {N,N}, {N,M}, {E,N}  },
	},
	{
		{ 	{M,E}, {M,N}, {N,N}, {E,N}, {E,M}  },	/* 11 */
		{ 	{M,M}, {N,M}, {N,N}, {N,E}, {E,E}  },
		{ 	{M,E}, {M,N}, {N,N}, {E,N}, {E,M}  },
		{ 	{M,M}, {N,M}, {N,N}, {N,E}, {E,E}  },
	},
	{
		{ 	{M,M}, {M,N}, {N,N}, {E,N}, {E,E}  },	/* 12 */
		{ 	{M,E}, {N,E}, {N,N}, {N,M}, {E,M}  },
		{ 	{M,M}, {M,N}, {N,N}, {E,N}, {E,E}  },
		{ 	{M,E}, {N,E}, {N,N}, {N,M}, {E,M}  },
	},
	{
		{ 	{M,E}, {M,N}, {M,M}, {N,M}, {E,M}  },	/* 13 */
		{ 	{M,M}, {N,M}, {E,M}, {E,N}, {E,E}  },
		{ 	{E,M}, {E,N}, {E,E}, {N,E}, {M,E}  },
		{ 	{E,E}, {N,E}, {M,E}, {M,N}, {M,M}  },
	}
};

clock_t levc[MAXLEVEL] =
{ 1500*CLK_TCK/1000, 1251*CLK_TCK/1000, 751*CLK_TCK/1000, 551*CLK_TCK/1000,
  451 *CLK_TCK/1000, 401 *CLK_TCK/1000, 351*CLK_TCK/1000, 301*CLK_TCK/1000,
  251 *CLK_TCK/1000, 201 *CLK_TCK/1000, 151*CLK_TCK/1000, 126*CLK_TCK/1000,
  101 *CLK_TCK/1000, 76  *CLK_TCK/1000, 51 *CLK_TCK/1000, 26 *CLK_TCK/1000
};

int	lines, levl;
long	points;

#define ATlevel		62,5
#define ATLEVEL		69,5

#define ATlines		62,7
#define ATLINES		72,7

#define ATpoints	62,9
#define ATPOINTS	69,9

#define ATSTATS		60,11

#define ATinfo		0,1
#define ATinput		0,0

#define AThigh		0,15

static char 	picc[PICS+PICSE] = ":~\\=/><.+OX)('";
static int 	picback[PICS+PICSE]  = {0,2,3,4,5,6,7, 0,2,3,4,5,6,7};
static int      piccolor[PICS+PICSE] = {0,2,3,4,5,6,7, 3,4,5,6,7,0,1};
static int	mystat[PICS+PICSE];

#define RND		28
#define RNDE		7

static int	rnd[RND+RNDE] = {
	0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4,
	5,5,5,5, 6,6,6,6, 7, 8, 9, 10, 11, 12, 13
};
