/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* The "side" structure is the repository of information about players. */
/* Surprisingly, there is not much code to manipulate side directly.  */
/* Viewing code is somewhat tricky, since any hex may be viewed by any */
/* number of sides at once. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "map.h"
#include "global.h"

Side neutral_placeholder; /* just has a pointer to next side and the neutral units. */
Side sides[MAXSIDES];  /* array containing all sides (not very many) */
Side *sidelist;        /* head of list of all sides */
Side *tmpside;         /* temporary used in many places */

int numsides;          /* number of sides in the game */

char *reasonnames[] = REASONNAMES;  /* names of columns in unit record */

extern x_command();

/* Reset any side structures that need it. */

init_sides()
{
    int i;

    for (i = 0; i < MAXSIDES; ++i) {
	sides[i].name = NULL;
	sides[i].laststarttime = 0;
	sides[i].unithead = NULL;
    }
    sidelist = NULL;
    numsides = 0;
}

/* Create an object representing a side. Checking to make sure all human */
/* players have displays has been done by now, so problems mean bugs. */

Side *
create_side(name, person, host)
char *name, *host;
bool person;
{
    int s, i;
    Side *newside;

    /* Can't have humans without displays */
    if (person && host == NULL) 
      {
      fprintf(stderr, "Can not have humans without displays...\n");
      abort();
      }
    if (name == NULL) name = "???";
    for (s = 0; s < MAXSIDES; ++s) {
	if (sides[s].name == NULL) {
	    newside = &(sides[s]);
            if (newside == NULL)
              {
              if (Debug) printf("&sides[%d] is NULL\n", s);
              }
	    newside->name = copy_string(name);
	    newside->humanp = person;
	    if (host == NULL || strcmp(host, "*") == 0) {
		newside->host = NULL;
	    } else {
		newside->host = copy_string(host);
	    }
	    newside->lost = FALSE;
	    for_all_unit_types(i) {
		newside->counts[i] = 1;
		newside->units[i] = 0;
		newside->building[i] = 0;
	    }
	    for_all_resource_types(i) {
		newside->resources[i] = 0;
	    }
            if (newside == NULL)
              {
              if (Debug) printf("newside = NULL before initializing\n");
              }
	    newside->showmode = BORDERHEX; /* overridden by X11.c
					      get_resources() */
	    newside->itertime = 100;
	    newside->startbeeptime = DEFAULT_STARTBEEPTIME;
	    newside->view =
		(viewdata *) malloc(world.width*world.height*sizeof(viewdata));
#ifdef PREVVIEW
	    newside->prevview =
		(viewdata *) malloc(world.width*world.height*sizeof(viewdata));
	    newside->viewtimestamp =
		(short *) malloc(world.width*world.height*sizeof(short));
#endif
	    newside->coverage =
		(short *) malloc(world.width*world.height*sizeof(short));
	    init_view(newside);
	    newside->deadunits = NULL;
	    newside->graphical = GRAPHICAL;
	    newside->display = 0L;
	    newside->bottom_note = 0;
            if (newside == NULL)
              {
              if (Debug) printf("newside = NULL before init_requests.\n");
              }
	    init_requests(newside);
            if (newside == NULL)
              {
              if (Debug) printf("newside = NULL before link_in_side.\n");
              }
	    link_in_side(newside);
            if (newside == NULL)
              {
              if (Debug) printf("newside = NULL after link_in_side.\n");
              }
	    ++numsides;
	    return newside;
	}
    }
    fprintf(stderr, "Cannot have more than %d sides total!\n", MAXSIDES);
    abort();
}

/* Add the new side to the end of the list of sides - this keeps our */
/* list traversals going from top to bottom (the things we do to keep */
/* users happy...). */

link_in_side(side)
Side *side;
{
    Side *head, *last;

    if (sidelist == NULL) {
	sidelist = side;
	neutral_placeholder.next = side;
    } else {
	for_all_sides(head) {
	    if (head->next == NULL) last = head;
	}
	last->next = side;
    }
    side->next = NULL;
    side->unithead = create_unit(-1, (char *) NULL);
}

/* Initialize basic viewing structures for a side, in preparation for the */
/* placement of units. */

init_view(side)
Side *side;
{
    int x, y, cov;
    viewdata seen;

    cov = (period.allseen ? 100 : 0);
    seen = (FALSE /*(period.allseen || world.known)*/ ? EMPTY : UNSEEN);
    for (x = 0; x < world.width; ++x) {
	for (y = 0; y < world.height; ++y) {
	    set_cover(side, x, y, cov);
	    side_view(side, x, y) = seen;
#ifdef PREVVIEW
	    side_prevview(side, x, y) = UNSEEN;
#endif
	}
    }
}

/* Given a side, get its relative position in array of sides (the "number"). */
/* Neutrals are -1, for lack of any better ideas. */

side_number(side)
Side *side;
{
    return (side == NULL ? MAXSIDES : (side - sides));
}

/* The inverse function - given a number, figure out which side it is. */
/* Return NULL for failure; hopefully callers will check on this! */

Side *
side_n(n)
int n;
{
    return ((n >= 0 && n < numsides) ? &sides[n] : NULL);
}

/* Put the given unit on the given side, without all the fancy effects. */
/* Important to handle neutrals, because this gets called during init. */

extern bool sidecountsread;

assign_unit_to_side(unit, side)
Unit *unit;
Side *side;
{
    unit->side = side;
    delete_unit(unit);  /* Make sure it is not in any list now. */
    if (side != NULL)
      insert_unit(side->unithead, unit);
    else insert_unit(neutral_placeholder.unithead, unit);
    if (!sidecountsread)
	unit->number = (side != NULL ? (side->counts)[unit->type]++ : 0);
}

/* Being at war requires only ones of the sides to consider itself so. */

enemy_side(s1, s2)
Side *s1, *s2;
{
    if (s1 == s2) return FALSE;
    return (s1 != NULL && s2 != NULL &&
	    (s1->attitude[side_number(s2)] <= ENEMY ||
	     s2->attitude[side_number(s1)] <= ENEMY));
}

/* A formal alliance requires the agreement of both sides. */

allied_side(s1, s2)
Side *s1, *s2;
{
    if (s1 == s2) return TRUE;
    return (s1 != NULL && s2 != NULL &&
	    s1->attitude[side_number(s2)] >= ALLY &&
	    s2->attitude[side_number(s1)] >= ALLY);
}

/* Neutralness is basically anything else. */

neutral_side(s1, s2)
Side *s1, *s2;
{
    return (!enemy_side(s1, s2) && !allied_side(s1, s2));
}

/* Formal declarations of war need to do a transitive closure, as part of */
/* dragging allies in. */

declare_war(side1, side2)
Side *side1, *side2;
{
    Side *side3;

    notify_all("The %s and the %s have declared war!!",
	       copy_string(plural_form(side1->name)),
	       copy_string(plural_form(side2->name)));
    make_war(side1, side2);
    for_all_sides(side3) {
	if (allied_side(side3, side1)) make_war(side3, side2);
	if (allied_side(side3, side2)) make_war(side3, side1);
    }
}

/* Internal non-noisy function. */

make_war(side1, side2)
Side *side1, *side2;
{
    side1->attitude[side_number(side2)] = ENEMY;
    side2->attitude[side_number(side1)] = ENEMY;
}

/* Establish neutrality for both sides. */

declare_neutrality(side1, side2)
Side *side1, *side2;
{
    notify_all("The %s and the %s have agreed to neutrality.",
	       copy_string(plural_form(side1->name)),
	       copy_string(plural_form(side2->name)));
    make_neutrality(side1, side2);
}

/* Internal non-noisy function. */

make_neutrality(side1, side2)
Side *side1, *side2;
{
    side1->attitude[side_number(side2)] = NEUTRAL;
    side2->attitude[side_number(side1)] = NEUTRAL;
}

/* Establish the alliance for both sides, then extend it to include */
/* every other ally (only need one pass over sides to ensure transitive */
/* closure, because alliances formed one at a time). */

declare_alliance(side1, side2)
Side *side1, *side2;
{
    Side *side3;

    notify_all("The %s and the %s enter into an alliance.",
	       copy_string(plural_form(side1->name)),
	       copy_string(plural_form(side2->name)));
    make_alliance(side1, side2);
    for_all_sides(side3) {
	if (allied_side(side3, side1)) make_alliance(side3, side2);
	if (allied_side(side3, side2)) make_alliance(side3, side1);
    }
}

/* Internal non-noisy function. */

make_alliance(side1, side2)
Side *side1, *side2;
{
    if (side1 != side2) {
	side1->attitude[side_number(side2)] = ALLY;
	side2->attitude[side_number(side1)] = ALLY;
    }
}

/* General method for passing along info about one side to another. */
/* If sender is NULL, it means to pass along info about *all* sides. */

reveal_side(sender, recipient, chance)
Side *sender, *recipient;
int chance;
{
    Unit *unit;
    int x, y;
    Side *loop_side;

    if (chance >= 100) {
      for (x = 0 ; x < world.width; x++)
	for (y = 0; y < world.height; y++) {
	  viewdata view = side_view(recipient, x, y);
#ifdef PREVVIEW
	   int ts = side_view_timestamp(recipient, x, y);
#endif
	  
	  if (view != EMPTY && view != UNSEEN
	      && side_n(vside(view)) == sender) { 
	    see_exact(recipient, x, y);
#ifdef PREVVIEW
	    side_view_timestamp(recipient, x, y) = ts;
#endif
	  }
	}
    }
    for_all_units(loop_side, unit) {
	if (alive(unit) &&
	    (unit->side == sender || sender == NULL) &&
	    probability(chance)) {
	    see_exact(recipient, unit->x, unit->y);
	    draw_hex(recipient, unit->x, unit->y, TRUE);
	}
    }
}

/* An always-seen unit has builtin spies to inform of movements. */
/* When such a unit occupies a hex, coverage is turned on and remains */
/* on until the unit leaves that hex. */

all_see_occupy(unit, x, y)
Unit *unit;
int x, y;
{
    Side *side;

    if (utypes[unit->type].seealways) {
	for_all_sides(side) {
	    if (side_view(side, x, y) != UNSEEN) {
		add_cover(side, x, y, 100);
		see_hex(side, x, y);
	    }
	}
    }
}

/* Departure results in coverage being decremented, AFTER the side sees */
/* that the hex is now empty. */

all_see_leave(unit, x, y)
Unit *unit;
int x, y;
{
    Side *side;

    if (utypes[unit->type].seealways) {
	for_all_sides(side) {
	    if (side_view(side, x, y) != UNSEEN) {
		see_hex(side, x, y);
		add_cover(side, x, y, -100);
	    }
	}
    }
}

/* Unit's beady eyes are now covering the immediate area.  The iteration */
/* covers a hex area;  since new things may be coming into view, we have */
/* to check and maybe draw lots of hexes (but only need the one flush, */
/* fortunately). */

cover_area(unit, x0, y0, onoff)
Unit *unit;
int x0, y0, onoff;
{
    int u = unit->type, range, x, y, x1, y1, x2, y2, best, diff, dist, cov;
    Unit *eunit;
    Side *side = unit->side;

    if (neutral(unit) || period.allseen || unit->x < 0 || unit->y < 0) return;
    range = utypes[u].seerange;
    if (range<1)
      return;
    best = utypes[u].seebest;
    diff = best - utypes[u].seeworst;
    y1 = y0 - range;
    y2 = y0 + range;
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, world.height-1)) {
	    x1 = x0 - (y < y0 ? (y - y1) : range);
	    x2 = x0 + (y > y0 ? (y2 - y) : range);
	    for (x = x1; x <= x2; ++x) {
		dist = distance(x0, y0, x, y);
		cov = (onoff * (best - (dist * diff) / range)) +
		    cover(side, wrap(x), y);
		set_cover(side, wrap(x), y, max(0, cov));
		if (onoff > 0 && see_hex(side, wrap(x), y)) {
		    if ((eunit = unit_at(wrap(x), y)) != NULL) {
			if (unit->orders.flags & ENEMYWAKE && !midturnrestore)
			    if (!allied_side(eunit->side, side))
				wake_unit(unit, TRUE, WAKEENEMY, eunit);
		    }
		}
	    }
	}
    }
    if (onoff > 0 && active_display(side)) flush_output(side);
}

/* Update the view of this hex for everybody's benefit.  May have to write */
/* to many displays, sigh. */

all_see_hex(x, y)
int x, y;
{
    register Side *side;

    for_all_sides(side) see_hex(side, x, y);
}

/* Look at the given position, possibly not seeing anything.  Return true if */
/* a unit was spotted. */

see_hex(side, x, y)
Side *side;
int x, y;
{
    register bool yes = FALSE;
    register int u, chance, terr;
    viewdata newview;
    register Unit *unit;
    viewdata curview;

    if (side == NULL) return FALSE;
    curview = side_view(side, x, y);
    if (cover(side, x, y) > 0) {
	if ((unit = unit_at(x, y)) != NULL) {
	    u = unit->type;
	    if (unit->side == side) {
		yes = TRUE;
	    } else {
		chance = (cover(side, x, y) * utypes[u].visibility) / 100;
		terr = terrain_at(x, y);
		chance = (chance * (100 - utypes[u].conceal[terr])) / 100;
		if (probability(chance)) yes = TRUE;
	    }
	    if (yes) {
		newview = buildview(side_number(unit->side), u);
		if (curview != newview) {
		  set_side_view(side, x, y, newview);
		  draw_hex(side, x, y, FALSE);
		  if (side->followaction && curview != newview
		      && side->mode == SURVEY) {
		    cancel_request(side);
		    side->curx = x; side->cury = y;
		    make_current(side, unit_at(x, y));
		  }
		}
		return TRUE;
	      } else {
	        if (side_view(side, x, y) == UNSEEN) {
		  set_side_view(side, x, y, EMPTY);
		  draw_hex(side, x, y, FALSE);
		}
		return FALSE;
	    }
	} else {
	  if (curview != EMPTY) {
            set_side_view(side, x, y, EMPTY);
	    draw_hex(side, x, y, FALSE);
	    if (side->followaction && side != curside
	        && curview != newview)
	      put_on_screen(side, x, y);
	  }
	  return FALSE;
	}
    } else {
	/* preserve old image */
	return FALSE;
    }
}

/* "Bare-bones" viewing, for whenever you know exactly what's there. */
/* This is the lowest level of all viewing routines, and executed a *lot*. */

see_exact(side, x, y)
Side *side;
int x, y;
{
    register viewdata newview;
    register Unit *unit = unit_at(x, y);

    if (side == NULL) return;
    newview =
      ((unit != NULL)
       ? buildview(side_number(unit->side), unit->type) : EMPTY);
    set_side_view(side, x, y, newview);
    draw_hex(side, x, y, FALSE);
}

/* Utility to clean up images of units from a lost side. */

remove_images(side, n)
Side *side;
int n;
{
    int x, y;
    viewdata view;

    for (x = 0; x < world.width; ++x) {
	for (y = 0; y < world.height; ++y) {
	    view = side_view(side, x, y);
	    if (view != EMPTY && view != UNSEEN && vside(view) == n) {
		set_side_view(side, x, y, EMPTY);
		draw_hex(side, x, y, TRUE);
	    }
	}
    }
}

/* Show some overall numbers on performance of a side. */

print_side_results(fp, side)
FILE *fp;
Side *side;
{
    fprintf(fp, "The %s (%s):\n",
	    plural_form(side->name), (side->host ? side->host : "machine"));
    fprintf(fp, "\n");
}

/* Display what is essentially a double-column bookkeeping of unit gains */
/* and losses.  Tricks here include the use of "dummy reason" flags to */
/* display sums of several columns. */

print_unit_record(fp, side)
FILE *fp;
Side *side;
{
    int atype, reason, sum;

    fprintf(fp, "Unit Record (gains and losses by cause and unit type)\n");
    fprintf(fp, "   ");
    for (reason = 0; reason < NUMREASONS; ++reason) {
	fprintf(fp, " %3s", reasonnames[reason]);
    }
    fprintf(fp, "  Total\n");
    for_all_unit_types(atype) {
	sum = 0;
	fprintf(fp, " %c ", utypes[atype].uchar);
	for (reason = 0; reason < NUMREASONS; ++reason) {
	    if (side->balance[atype][reason] > 0) {
		fprintf(fp, " %3d", side->balance[atype][reason]);
		sum += side->balance[atype][reason];
	    } else if (reason == DUMMYREAS) {
		fprintf(fp, " %3d", sum);
		sum = 0;
	    } else {
		fprintf(fp, "    ");
	    }
	}
	fprintf(fp, "   %3d\n", sum);
    }
    fprintf(fp, "\n");
}

void set_side_view(s,x,y,v)
Side *s;
int x,y;
viewdata v;
{

#ifdef PREVVIEW
  s->viewtimestamp[world.width*y+x] = global.time;
/*   if (v != side_view(s,x,y)) */
    s->prevview[world.width*y+x] = side_view(s,x,y);
#endif
  s->view[world.width*y+x] = v;
}

/*
viewdata buildview(side, utype)
int side,utype;
{
  viewdata newview;
  
  vside(newview) = side;
  vtype(newview) = utype;
  return newview;
}
*/
