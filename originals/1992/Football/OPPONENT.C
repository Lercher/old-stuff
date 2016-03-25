/* opponent.c - Die "Intelligenz" der Gegner fuer football.c
 * Martin Lercher
 * Schloesslgartenweg 40
 * 8415 Nittenau
 * 09436/8475 oder 091/943-2793
 * September 1992
 */
 
#include <stdlib.h>
#include <stddef.h>
#include "football.h"
#include "opponent.h"
#include "menu.h"

struct translate {
	int fnr;
	move_func *doit;
	char *name;
};

static struct player *delta_kollision(struct player *p)
{	struct player *o;
	int x, y;
	
	x = p->pos.x+p->delta.x;
	y = p->pos.y+p->delta.y;
	foreach(o)
		if (o!=p && o->active &&
		  o->pos.x == x &&
		  o->pos.y == y)
			return o;
	return NULL;
}

int intervall(int u, int o)
{
	return (o>u ? rand()%(o-u+1) : -rand()%(u-o+1)) + u;
}

static int norm(int x)
{
	if (x<0)
		return -1;
	if (x>0)
		return 1;
	return 0;
}

static void mk_delta(struct position *delta,
	struct position *v, struct position *n)
{	int x, y;
	
	x = n->x - v->x;
	delta->x = norm(x);
	y = n->y - v->y;
	delta->y = norm(y);
}

static void no_kollision(struct player *p)
{	struct player *fp;

	fp = delta_kollision(p);
	if (fp && fp->typ==OPPONENT)
		p->delta.y = p->delta.x = 0;
}

static void random_walk(struct player *p, struct player *fp)
{
	p->delta.x = intervall(-1, 1);
	p->delta.y = intervall(-1, 1);
	no_kollision(p);
	p = fp;
}

static void ball_grabber(struct player *p, struct player *fp)
{
	mk_delta(&p->delta, &p->pos, &fp->pos);
	no_kollision(p);
}

static void terminator(struct player *p, struct player *fp)
{	int x;
	
	x = p->pos.x - fp->pos.x;
	if (direction*x < 1)
	{
		p->sleep = intervall(0,2);
		p->doit = (move_func *)ball_grabber;
	}
	if (intervall(0, 40) < p->sleep) p->sleep--;
	if (direction*x < 6)
		p->delta.x = norm(intervall(-3+direction, 3+direction));
	p->delta.y = norm(intervall(0, fp->pos.y - p->pos.y));
	no_kollision(p);
}

/* has initialy sleep 0! */
static void side_blocker(struct player *p, struct player *fp)
{
	if (p->pos.y == fp->pos.y)
	{
		p->sleep = intervall(1,3);
		p->doit = (move_func *)ball_grabber;
	}
	if (!intervall(0, 5))
		random_walk(p, fp);
}

#if 0
void setup(int x, int y)
{	struct player *p;

	p = first_player();
	init(&p, x,   y,   PLAYER,   PL1, 0, (move_func *)NULL);
	init(&p, x+1*direction, y+1, BLOCKER,  PL2, 0, (move_func *)NULL);
	init(&p, x+1*direction, y-1, BLOCKER,  PL2, 0, (move_func *)NULL);
	init(&p, x+3*direction, y-1, OPPONENT, PL3, 6, (move_func *) random_walk);
	init(&p, x+3*direction, y-5, OPPONENT, PL3, 0, (move_func *) side_blocker);
	init(&p, x+4*direction, y-1, OPPONENT, PL3, 10,(move_func *) random_walk);
	init(&p, x+4*direction, y+5, OPPONENT, PL3, 0, (move_func *) side_blocker);
	init(&p, x+5*direction, y-1, OPPONENT, PL3, 12,(move_func *) terminator);
	init(&p, x+6*direction, y,   OPPONENT, PL3, 4, (move_func *) ball_grabber);
}
#endif


static void do_init(struct player *p, int x, int y, struct data *d)
{
	if (!(p->active = d->active))
		return;
	p->deactivate = 0;
	p->active = testinfield(
	p->pos.x = x + direction*d->pos.x,
	p->pos.y = y + d->pos.y
	);
	p->typ = d->typ;
	p->symbol = d->symbol;
	p->sleep = d->sleep;
	p->ticks = d->sleep;
	p->doit = fnr2func(d->fnr);
}

void setup(int x, int y)
{	struct scheme *s;
	struct data *d;
	struct player *p;

	s = getscheme();
	d = s->data;
	foreach(p)
	{
		do_init(p, x, y, d);
		d++;
	}
}


static struct translate translate[MAXFUNCS+1] = {
 { 1, (move_func *)random_walk,  "Random Walker" },
 { 2, (move_func *)ball_grabber, "Ballgrabber" },
 { 3, (move_func *)side_blocker, "Side Blocker" },
 { 4, (move_func *)terminator,   "Terminator"},
 { 0, (move_func *)NULL,         "None"}
};

move_func *fnr2func(int n)
{	struct translate *t = translate;

	while(t->fnr && t->fnr != n)
		t++;
	return t->doit;
}

char *fnr2string(int n)
{	struct translate *t = translate;

	while(t->fnr && t->fnr != n)
		t++;
	return t->name;
}

