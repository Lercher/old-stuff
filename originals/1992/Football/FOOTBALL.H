/* football.h - Structs... fuer football.c
 * Martin Lercher
 * Schloesslgartenweg 40
 * 8415 Nittenau
 * 09436/8475 oder 091/943-2793
 * September 1992
 */

#define foreach(p)	for((p) = first_player(); (p); (p) = next_player(p))
#define HIGHCOLOR (MaxColor>1)

enum type { PLAYER, BLOCKER, OPPONENT, THE_BALL, BALLKEEPER, RUNNER };
enum {SHOW, HIDE, FX=100, FY=20, AREA=5, PLAYERS=15};
enum symbol { CLEAR, PL1, PL2, PL3, BALL, CIRCLES, INTENSE};

typedef void move_func(struct player *p, struct player *fp);

struct position {
	int x, y;
};

struct player {
	struct position pos, delta;
	enum type typ;
	int active;
	int deactivate;
	int sleep;
	int ticks;
	enum symbol symbol;
	move_func *doit;
};

struct info {
	struct position pos, pos1;
	char *text, *format;
	int *number, displayed, factor, offset;
};

extern int direction, X, Y, quit, kbrepeat, MaxColor, Color;

struct player *first_player(void);
struct player *next_player(struct player *p);
void the_player(struct player *p, int mode);
void clear(int color);
int testinfield(int x, int y);
int mygetch(void);

#if 0
void init(struct player **p, int x, int y,
	enum type typ, enum symbol symbol, int sleep, move_func *doit);
#endif
