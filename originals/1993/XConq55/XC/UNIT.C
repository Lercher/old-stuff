/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file contains all code relating specifically to units. */

/* Since units appear and disappear with distressing regularity (so it seems */
/* to the player trying to keep up with all of them!), we have a little */
/* storage manager for them.  Since this sort of thing is tricky, there is a */
/* two-level procedure for getting rid of units.  First the hit points are */
/* reduced to zero, at which point the unit is considered "dead".  At the */
/* end of a turn, we actually GC the dead ones.  At this point their type */
/* slot becomes NOTHING, and they are available for allocation. */

#include "config.h"
#include "misc.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "map.h"
#include "global.h"

char *ordinal();

Unit *freeunits;           /* list of available units */
/* Unit *unitlist = NULL; */        /* pointer to head of list of units */
Unit *tmpunit;         /* global temporary used in several places */

char unitbuf[BUFSIZE];

int numunits = 0;          /* total number of units in existence */
int nextid = 0;            /* next number to be used for ids */
int occdeath[MAXUTYPES];  /* buffer for remembering occupant death */

bool recent_dead_flushed = TRUE; /* Have units died since dead unit lists made */

/* Init all the unit entries so we can find them when allocating. */

void init_units()
{
    int i;
    Unit *units;
    Side *side;

    units = (Unit *) malloc(INITMAXUNITS * sizeof(Unit));
    freeunits = units;
    for (i = 0; i < INITMAXUNITS; ++i) {
      units[i].type = NOTHING;
      units[i].next = &units[i+1];
    }
    units[INITMAXUNITS-1].next = NULL;

    side = &neutral_placeholder;
    if (side->unithead == NULL) {
      side->unithead = freeunits;
      freeunits = freeunits->next;
      side->unithead->next = side->unithead;
      side->unithead->prev = side->unithead;
      side->unithead->type = -1;
    }

}

/* Create a new unit of given type, with given name.  Default all the other */
/* slots - routines that need to will fill them in properly.  This routine */
/* will return a valid unit or NULL.  Note that unit morale starts out at */
/* max, signifying hopeful but inexperienced recruits marching off to war. */

Unit *
create_unit(type, name)
int type;
char *name;
{
    int r;
    Unit *newunit;

    enter_procedure("create_unit");
    if (freeunits == NULL) {
	if (!grow_unit_array()) {
	  return NULL;
	}
    }
    newunit = freeunits;
    freeunits = freeunits->next;
    newunit->type = type;
    if (name == NULL || strlen(name) == 0) {
	newunit->name = NULL;
    } else {
	newunit->name = copy_string(name);
    }
    newunit->x = newunit->y = -1;
    newunit->prevx = newunit->prevy = -1;
    newunit->number = 0;
    newunit->side = NULL;
    newunit->id = nextid++;
    newunit->trueside = NULL;
    newunit->hp = (type>=0) ? utypes[type].hp : 1;
    newunit->product = NOTHING;
    newunit->schedule = 0;
    newunit->built = 0;
    newunit->transport = NULL;
    for_all_resource_types(r) newunit->supply[r] = 0;
    newunit->movesleft = 0;
    newunit->goal = 0;
    newunit->area = -1;
    newunit->awake = FALSE;   /* i.e., is usually not *temporarily* awake */
    newunit->standing = NULL;
    newunit->occupant = NULL;
    newunit->nexthere = NULL;
    newunit->mlist = NULL;
    newunit->nextinarea = NULL;
    newunit->plan = NULL;
    newunit->priority = 0;
    newunit->next = newunit;
    newunit->prev = newunit;
    if (type != -1) {
      insert_unit(neutral_placeholder.unithead, newunit);
      ++numunits;
    }
    wake_unit(newunit, FALSE, WAKEFULL, (Unit *) NULL);
    newunit->move_turn = -1;
    exit_procedure();
    return newunit;
}

/* Just call the same initial code and get a fresh bunch of unused units */

grow_unit_array()
{

#ifdef GROWABLE
    init_units();
    notify_all("The unit array has been grown.");
    return TRUE;
#else
    notify_all("Can't make any more units!");
    return FALSE;
#endif
}

/* Find a good initial unit for the given side.  Only requirement these */
/* days is that it be of the type specified in the period file, and not */
/* already used by somebody.  If enough failed tries to find one, */
/* something may be wrong, but not necessarily. */

Unit *
random_start_unit()
{
  Unit *unit, *choices = NULL;
  int numchoices = 0, choice;
  Side *side = &neutral_placeholder;

  for_all_side_units(side, unit) 
      if (unit->type == period.firstutype) {
	unit->mlist = choices;
	choices = unit;
	numchoices++;
      }
    if (numchoices == 0) return NULL;
    choice = RANDOM(numchoices);
    for (unit = choices; choice > 0 ; unit = unit->mlist) 
      choice--;
    return unit;
}

/* A unit occupies a hex either by entering a unit on that hex or by having */
/* the occupant pointer filled in.  If something goes wrong, return false. */
/* This is heavily used. */

bool occupy_hex(unit, x, y)
Unit *unit;
int x, y;
{
    register int u = unit->type, o;
    register Unit *other = unit_at(x, y);

    if (unit->x >= 0 || unit->y >= 0) {
      notify_all("Unit %d occupying hex (%d, %d), was at (%d %d)",
	     unit->id, x, y, unit->x, unit->y);
      printf("Unit %d occupying hex (%d, %d), was at (%d %d)\n",
	     unit->id, x, y, unit->x, unit->y);
    }
    if (!mobile(unit->type) && unit->prevx != -1 && !midturnrestore
	&& global.time > 1) {
      notify_all("nonmover %d has moved", unit->id);
      printf("nonmover %d has moved", unit->id);
    }
    if (other) {
	o = other->type;
	if (could_carry(o, u)) {
	    occupy_unit(unit, other);
	} else if (could_carry(u, o)) {
	    leave_hex(other);
	    occupy_hex(unit, x, y);
	    occupy_hex(other, x, y);
	} else {
	    return FALSE;
	}
    } else {
	set_unit_at(x, y, unit);
	occupy_hex_aux(unit, x, y);
	all_see_occupy(unit, x, y);
	all_see_hex(x, y);
    }
    return TRUE;
}

/* Recursive helper to update everybody's position.  This should be one of */
/* two routine that modify unit positions (leaving is the other). */
/* *Every* occupant will increment viewing coverage - strains realism, */
/* but prevents strange bugs. */

occupy_hex_aux(unit, x, y)
Unit *unit;
int x, y;
{
    register Unit *occ;

    unit->x = x;  unit->y = y;
    cover_area(unit, x, y, 1);
    for_all_occupants(unit, occ) occupy_hex_aux(occ, x, y);
}

/* Decide whether transport has the capability to house the given unit. */
/* Check both basic capacity and relative volumes. */

can_carry_aux(transport, utype)
     Unit	*transport;
     int	utype;
{
  int	ttype = transport->type, total=0, volume=0;
  int	hold = utypes[transport->type].holdvolume;
  Unit	*occ;

  for_all_occupants(transport, occ) {
    if (occ->type == utype) total++;
    volume += utypes[occ->type].volume;
  }
  if (cripple(transport)) {
    hold = (hold * transport->hp) / (utypes[ttype].crippled + 1);
  }
  return ((total + 1 <= utypes[ttype].capacity[utype]) &&
	  (volume + utypes[utype].volume <= hold));
}

can_carry(transport, unit)
Unit *transport, *unit;
{
  if (transport == unit) return FALSE;
  return can_carry_aux(transport, unit->type);
}

/* Check for units waiting to embark */

check_for_embarkies(unit, top_unit)
Unit *unit, *top_unit;
{
  Unit *occ, *next;
  int timeleft;

  next = top_unit->occupant;
  while (next != NULL) {
    occ = next;
    next = occ->nexthere;
    if (occ->orders.type == EMBARK && can_carry(unit, occ)) {
      timeleft = occ->orders.rept;
      leave_hex(occ);
      occupy_unit(occ, unit);
      /* put unit to sleep unless it was given standing orders */
      if (unit->standing == NULL ||
	  unit->standing->orders[occ->type] == NULL)
	order_sentry(occ, min(timeleft, 5));
    }
  }
}

/* Wakeup if we were waiting to fill up with occupants and we actually
   did fill up. */

wake_if_full(unit)
     Unit	*unit;
{
  Unit	*occ;
  int	volume, u;

  volume = 0;

  if (unit->orders.type != FILL) return;

  for_all_occupants(unit, occ) {
    volume += utypes[occ->type].volume;
  }

  if ( volume < utypes[unit->type].holdvolume ) {
    /* we have room in the hold, but could we fit anything
       in that space? */
    for_all_unit_types(u) {
      if ( can_carry_aux(unit, u) ) {
	/* we are not full */
	/*printf("could fit a %d on unit 0x%x\n", u, unit);*/
	return;
      }
    }
  }
  /* couldn't find room for anything else
     We're full! */
  if (Debug) printf("unit 0x%x is full\n", unit);
  wake_unit(unit, FALSE, WAKEFULL, NULL);
}

/* check for and give out standing orders */

get_standing_orders(unit, transport)
Unit *unit, *transport;
{
  Order *newords;
  int r;

  enter_procedure("get_standing_orders");
  if (transport->standing != NULL && !midturnrestore) {
    newords = (transport->standing->orders)[unit->type];
    if (newords && newords->type != NONE) {
      if (Debug) printf("%s getting orders %s",
			unit_desig(unit), order_desig(newords));
      if (newords->type != SENTRY && unit->movesleft > 0)
	for_all_resource_types(r) {
	  try_transfer(unit, transport, r);
	  try_transfer(transport, unit, r);
	}
      copy_orders(&(unit->orders), newords);
      if (newords->type == EMBARK) {
	unit->side->markunit = NULL;
	do_embark(unit->side, unit, newords->rept);
      }
    }
  }
  exit_procedure();
}

/* Units become passengers by linking into front of transport's passenger */
/* list.  They only wake up if the transport is a moving type, but always */
/* pick up standing orders if any defined.  A passenger will get sorted to */
/* move after transport, at end of turn. */

void occupy_unit(unit, transport)
Unit *unit, *transport;
{

    if (!utypes[unit->type].occproduce) cancel_build(unit);
    if (!midturnrestore)
      check_for_embarkies(unit, transport);
    unit->nexthere = transport->occupant;
    transport->occupant = unit;
    unit->transport = transport;
    sort_occupants_by_speed(transport);
    if (!midturnrestore)
      wake_if_full(transport);
    if (mobile(transport->type) && !midturnrestore) 
      wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
    /* make sure not to give out standing orders to units after a restore
       from savefile */
    occupy_hex_aux(unit, transport->x, transport->y);
    all_see_occupy(unit, transport->x, transport->y);
    get_standing_orders(unit, transport);
}

/* Unit departs from a hex by zeroing out pointer if in hex or by being */
/* removed from the list of transport occupants. */
/* Dead units (hp = 0) may be run through here, so don't error out. */

void leave_hex(unit)
Unit *unit;
{
    int ux = unit->x, uy = unit->y;

    if (ux < 0 || uy < 0) {
	/* Sometimes called twice */
    } else if (unit->transport != NULL) {
	leave_unit(unit, unit->transport);
	leave_hex_aux(unit);
	all_see_leave(unit, ux, uy);
    } else {
	set_unit_at(ux, uy, NULL);
	see_exact(unit->side, ux, uy);
	see_hex(unit->side, ux, uy);
	leave_hex_aux(unit);
	all_see_leave(unit, ux, uy);
	all_see_hex(ux, uy);
    }
}

/* Trash old coordinates (recursively) just in case.  Catches many bugs... */

leave_hex_aux(unit)
Unit *unit;
{
    register Unit *occ;

    cover_area(unit, unit->x, unit->y, -1);
    unit->prevx = unit->x; unit->prevy = unit->y;
    unit->x = -1;  unit->y = -1;
    for_all_occupants(unit, occ) leave_hex_aux(occ);
}

/* Disembarking unlinks from the list of passengers. */

void leave_unit(unit, transport)
Unit *unit, *transport;
{
    Unit *occ;

    if (unit == transport->occupant) {
	transport->occupant = unit->nexthere;
    } else {
	for_all_occupants(transport, occ) {
	    if (unit == occ->nexthere) {
		occ->nexthere = occ->nexthere->nexthere;
		break;
	    }
	}
    }
    unit->transport = NULL;
}

/* Handle the general situation of a unit changing allegiance from one side */
/* to another.  This is a common internal routine, so no messages here. */

unit_changes_side(unit, newside, reason1, reason2)
Unit *unit;
Side *newside;
int reason1, reason2;
{
  static int dummy[MAXUTYPES];
  unit_changes_side_Ncount(unit, newside, reason1, reason2, dummy, dummy);
}

unit_changes_side_Ncount(unit, newside, reason1, reason2, captured, killed)
Unit *unit;
Side *newside;
int reason1, reason2;
int captured[MAXUTYPES], killed[MAXUTYPES]; /* arrays to increment */
{
    Side *oldside = unit->side;
    Unit *occ;

    delete_unit(unit);  /* no longer on this side. */

    if (newside != NULL)
      insert_unit(newside->unithead, unit);
    else insert_unit(neutral_placeholder.unithead, unit);
    if (reason1 == CAPTURE && !period.capturemoves)
      unit->movesleft = 0;
    if (oldside != NULL) {
	oldside->units[unit->type]--;
	if (producing(unit)) oldside->building[unit->product]--;
	if (reason2 >= 0) oldside->balance[unit->type][reason2]++;
	update_state(oldside, unit->type);
	if (reason2 == PRISONER)
	  oldside->areas[area_index(unit->x, unit->y)].units_lost +=
	    2 * unit_strength(unit->type);
      }
    if (newside == NULL && !utypes[unit->type].isneutral) {
	kill_unit(unit, -1);
    } else {
	if (newside != NULL) {
	    newside->units[unit->type]++;
	    if (producing(unit)) newside->building[unit->product]++;
	    if (reason1 >= 0) newside->balance[unit->type][reason1]++;
	    update_state(newside, unit->type);
	}
	for_all_occupants(unit, occ) {
	  if ((occ->side == unit->side || reason2==PRISONER)
	      && utypes[occ->type].changeside) {
	    unit_changes_side_Ncount(occ, newside, reason1, reason2,
				     captured, killed);
	    captured[occ->type]++;	/* count capture here */
	  } else if (reason2>0 && reason2!=GIFT) {
	    kill_unit(occ, COMBAT);
	    killed[occ->type]++; /* count slaughter here */
	  } else {
	    /* unit remains loyal */
	  }
	}
    }
    if (alive(unit)) {
	cover_area(unit, unit->x, unit->y, -1);
	assign_unit_to_side(unit, newside);
	cover_area(unit, unit->x, unit->y, 1);
    }
}

/* Change the product of a unit.  Displays will need appropriate mods. */

void set_product(unit, type)
Unit *unit;
int type;
{
    if (!neutral(unit) && global.setproduct && type != unit->product) {
	if (unit->product != NOTHING) {
	    unit->side->building[unit->product]--;
	    update_state(unit->side, unit->product);
	}
	if (type != NOTHING) {
	    unit->side->building[type]++;
	    update_state(unit->side, type);
	    if (!utypes[unit->type].maker) {
	      order_sentry(unit, build_time(unit, type));
	    }
	}
	unit->product = type;
	unit->next_product = type;
	unit->built = 0;
	}
}

/* Set product completion time.  Startup development cost incurred only */
/* if directed. */
/* Technology development cost only incurred for very first unit that the */
/* side constructs.  Both tech and startup are percent addons. */

void set_schedule(unit)
Unit *unit;
{
    if (producing(unit)) unit->schedule = build_time(unit, unit->product);
}

/* Basic routine to compute how long a unit will take to build something. */

build_time(unit, prod)
Unit *unit;
int prod;
{
    int schedule = utypes[unit->type].make[prod];
    int u, research_delay;

    if (unit->built == 0)
	schedule += ((schedule * utypes[prod].startup) / 100);
    if (unit->side->counts[prod] <= 1) {
	research_delay = ((schedule * utypes[prod].research) / 100);
	for_all_unit_types(u)
	  if (unit->side->counts[u] > 1) {
	    research_delay -= (utypes[unit->type].make[u] *
			       utypes[prod].research_contrib[u]) / 100;
	  }
	if (research_delay > 0)
	  schedule += research_delay;
      }
    return schedule;
}

/* Remove a unit from play.  This is different from making it available for */
/* reallocation - only the unit flusher can do that.  We remove all the */
/* passengers too, recursively.  Sometimes units are "killed twice", so */
/* be sure not to run all this twice.  Also count up occupant deaths, being */
/* sure not to count the unit itself as an occupant. */

kill_unit(unit, reason)
Unit *unit;
int reason;
{
    int u = unit->type, u2;

    enter_procedure("kill unit");
    if (alive(unit)) {
	for_all_unit_types(u2) occdeath[u2] = 0;
	/* avoid potential problems caused by displays already being closed. */
	if (reason != ENDOFWORLD) leave_hex(unit);
	kill_unit_aux(unit, reason);
	occdeath[u]--;
	recent_dead_flushed = FALSE;
    }
    exit_procedure();
}

/* Trash it now - occupant doesn't need to leave_hex.  Also record a reason */
/* for statistics, and update the apropriate display.  The unit here should */
/* be known to be alive. */

kill_unit_aux(unit, reason)
Unit *unit;
int reason;
{
    int u = unit->type;
    Unit *occ;
    
    unit->hp = 0;
    occdeath[u]++;
    free_plan(unit->plan);
    unit->plan = NULL;
    if (unit->side != NULL && reason >= 0) {
	unit->side->balance[u][reason]++;
	unit->side->units[u]--;
	if (producing(unit)) unit->side->building[unit->product]--;
	update_state(unit->side, u);
	unit->side->areas[area_index(unit->prevx, unit->prevy)].units_lost +=
	  unit_strength(u);
    }
    for_all_occupants(unit, occ) if (alive(occ)) kill_unit_aux(occ, reason);
}

/* Get rid of all dead units at once.  It's important to get rid of all */
/* of the dead units, so they don't come back and haunt us. */
/* (This routine is basically a garbage collector, and should not be called */
/* during a unit list traversal.) The process starts by finding the first */
/* live unit, making it the head, then linking around all in the middle. */
/* Dead units stay on the dead unit list for each side until that side has had */
/* a chance to move.  Then they are finally flushed in a permanent fashion. */ 

void flush_dead_units()
{
    Unit *unit, *prevunit;
    Side *side;

    /* This never ends up flushing neutral units, no big loss. */
    if (!recent_dead_flushed) {
      recent_dead_flushed = TRUE;
      for_all_units(side, unit) {
	if (!alive(unit)) {
	  prevunit = unit->prev;
	  delete_unit(unit);
	  put_unit_on_dead_list(unit);
	  unit = prevunit;
	}
      }
    }
}

/* Put dead units on sides dead unit lists. */
put_unit_on_dead_list(unit)
Unit *unit;
{
  enter_procedure("put unit on dead list");
  if (unit->side == NULL)
    flush_one_unit(unit);
  else {
    unit->next = unit->side->deadunits;
    unit->side->deadunits = unit;
  }
  --numunits;
  exit_procedure();
}

/* Clean up dead units and put them back on free list. */

flush_side_dead(side)
Side *side;
{
  Unit *unit, *next;
  
  next = side->deadunits;
  while (next != NULL) {
    unit = next;
    next = unit->next;
    flush_one_unit(unit);
  }
  side->deadunits = NULL;
  side->curdeadunit = NULL;
}

/* Keep it clean - hit all links to other places.  Some might not be */
/* strictly necessary, but this is not an area to take chances with. */

flush_one_unit(unit)
Unit *unit;
{
    unit->type = NOTHING;
    unit->occupant = NULL;
    unit->transport = NULL;
    unit->nexthere = NULL;
    if (unit->standing != NULL) {
      free(unit->standing);
      unit->standing = NULL;
    }
    unit->next = freeunits;
    freeunits = unit;
}

/* Do multiple passes of bubble sort.  This is intended to improve locality */
/* among unit positions and reduce the amount of scrolling around. */
/* Data is generally coherent, so bubble sort not too bad if we allow */
/* early termination when everything in order. */
/* If slowness objectionable, replace with something clever. */

void sort_units(doall)
bool doall;
{
    bool flips;
    int passes = 0;
    register Unit *unit, *nextunit;
    Side *side;

    for_all_sides(side) {
      passes = 0;
      flips = TRUE;
      while (flips && (doall || passes < numunits / 10)) {
	flips = FALSE;
	for_all_side_units(side,unit) 
	  if (unit->next != side->unithead && !in_order(unit, unit->next)) {
	    flips = TRUE;
	    nextunit = unit->next;
	    unit->prev->next = nextunit;
	    nextunit->next->prev = unit;
	    nextunit->prev = unit->prev;
	    unit->next = nextunit->next;
	    nextunit->next = unit;
	    unit->prev = nextunit;
	  }
	passes++;
      }
    }
    if (Debug) printf("Sorting passes = %d\n", passes);
}

in_order(unit1, unit2)
Unit *unit1, *unit2;
{
    int d1, d2;

    if (side_number(unit1->side) < side_number(unit2->side)) {
      printf("Sides mixed");
      return TRUE;
    }
    if (side_number(unit1->side) > side_number(unit2->side)) {
      printf("sides mixed");
      return FALSE;
    }
    if (!neutral(unit1)) {
	d1 = distance(unit1->side->cx, unit1->side->cy, unit1->x, unit1->y);
	d2 = distance(unit2->side->cx, unit2->side->cy, unit2->x, unit2->y);
	if (d1 < d2) return TRUE;
	if (d1 > d2) return FALSE;
    }
    if (unit1->y > unit2->y) return TRUE;
    if (unit1->y < unit2->y) return FALSE;
    if (unit1->x < unit2->x) return TRUE;
    if (unit1->x > unit2->x) return FALSE;
    if (unit1->transport == NULL && unit2->transport != NULL) return TRUE;
    if (unit1->transport != NULL && unit2->transport == NULL) return FALSE;
    if (unit1 == unit2->transport) return TRUE;
    if (unit1->transport == unit2) return FALSE;
    return TRUE;
}

/* A unit runs low on supplies at the halfway point, but only worries about */
/* the essential items.  Formula is the same no matter how/if occupants eat */
/* transports' supplies. */

low_supplies(unit)
Unit *unit;
{
    int u = unit->type, r;

    for_all_resource_types(r) {
	if (((utypes[u].consume[r] > 0) || (utypes[u].tomove[r] > 0)) &&
	    /* Really should check that the transport is adequate for */
	    /* supplying the fuel */ 
	    (unit->transport == NULL)) {
	    if (2 * unit->supply[r] <= utypes[u].storage[r]) return TRUE;
	}
    }
    return FALSE;
}

/* useful for the machine player to know how long it can move this
piece before it should go home. */

moves_till_low_supplies(unit)
Unit *unit;
{
    int u = unit->type, r, moves = 1000;

    for_all_resource_types(r) {
	if ((utypes[u].tomove[r] > 0)) {
	  moves = min(moves,
		      (unit->supply[r] - (utypes[u].storage[r] / 2)) /
		      utypes[u].tomove[r]);
	}
    }
    return moves;
}

/* Cancel the build of a unit because it attacked or moved.  Does */
/* nothing if not building or if a maker. */

void cancel_build(unit)
Unit *unit;
{
    Side *us = unit->side;

    if (global.setproduct && producing(unit) && !utypes[unit->type].maker) {
      notify(us, "%s moved, cancelling its build!", unit_handle(us, unit));
      if (active_display(us) && humanside(us)) beep(us);
      set_product(unit, NOTHING);
      unit->schedule = 0;
    }
}

/* Display the standing orders of the given unit. */

show_standing_orders(side, unit)
Side *side;
Unit *unit;
{
    int u;
    Order *ords;

    enter_procedure("show standing orders");
    if (unit->standing != NULL) {
	sprintf(spbuf, "Orders:  ");
	for_all_unit_types(u) {
	    ords = unit->standing->orders[u];
	    if (ords && ords->type != NONE) {
		sprintf(tmpbuf, "%s to %s%0s;  ",
			utypes[u].name, order_desig(ords),
			((ords->morder) ? " and auto" : ""));
		if (strlen(spbuf) + strlen(tmpbuf) > 
		    side->nw) {
		  notify(side, "%s", spbuf);
		  sprintf(spbuf, "Orders:  ");
		}
		strcat(spbuf, tmpbuf);
	    }
	}
	notify(side, "%s", spbuf);
    } else {
	notify(side, "No standing orders defined yet.");
    }
    exit_procedure();
}

/* Build a short phrase describing a given unit to a given side. */
/* First we supply identification of side, then of unit itself. */

char *
unit_handle(side, unit)
Side *side;
Unit *unit;
{
    char *utypename = utypes[unit->type].name;

    if (unit == NULL || (!alive(unit) && unit != side->curdeadunit))
      return "???";
    if (utypes[unit->type].named == 2 && unit->name) return unit->name;
    if (unit->side == NULL) {
	sprintf(unitbuf, "the neutral ");
    } else if (unit->side == side) {
	sprintf(unitbuf, "your%0s ",
		((unit->orders.morder) ? "*" : ""));
    } else {
	sprintf(unitbuf, "the %s ", unit->side->name);
    }
    if (unit->name != NULL) {
	sprintf(tmpbuf, "%s %s", utypename, unit->name);
    } else if (unit->number > 0) {
	sprintf(tmpbuf, "%s %s", ordinal(unit->number), utypename);
    } else {
	sprintf(tmpbuf, "%s", utypename);
    }
    strcat(unitbuf, tmpbuf);
    return unitbuf;
}

/* Shorter unit description omits side name, but uses same buffer. */

char *
short_unit_handle(unit)
Unit *unit;
{
    if (unit->name == NULL) {
	sprintf(unitbuf, "%s %s",
		ordinal(unit->number), utypes[unit->type].name);
    } else {
	sprintf(unitbuf, "%s", unit->name);
    }
    return unitbuf;
}

/* General-purpose routine to take an array of anonymous unit types and */
/* summarize what's in it, using numbers and unit chars. */

char *
summarize_units(buf, ucnts)
char *buf;
int *ucnts;
{
    char tmp[BUFSIZE];
    int u;

    sprintf(buf, "");
    for_all_unit_types(u) {
	if (ucnts[u] > 0) {
	    sprintf(tmp, " %d %c", ucnts[u], utypes[u].uchar);
	    strcat(buf, tmp);
	}
    }
    return buf;
}

/* Search for a unit with the given id number. */

Unit *
find_unit(n)
int n;
{
    Unit *unit;
    Side *side;

    for_all_units(side,unit)
      if (alive(unit) && unit->id == n) return unit;
    return NULL;
}

/* Given a unit character, find the type. */

find_unit_char(ch)
char ch;
{
    int u;

    for_all_unit_types(u) if (utypes[u].uchar == ch) return u;
    return NOTHING;
}

/* Generate a name for a unit, using best acrynomese.  This is invoked when */
/* a new named unit has been created. */

char *
make_unit_name(unit)
Unit *unit;
{
    sprintf(spbuf, "%c%c-%c-%02d",
	    uppercase(unit->side->name[0]), uppercase(unit->side->name[1]),
	    utypes[unit->type].uchar, unit->number);
    return copy_string(spbuf);
}

/* Reverse the order of the unit list.  This is needed since the */
 /* loaded units from map files are put into the list in reverse */
 /* order. */

reverse_unit_order()
{
  Unit *next, *unit;
  Side *side;

  for_all_sides(side) {
    next = side->unithead->next;
    do {
      unit = next;
      next = unit->next;
      unit->next = unit->prev;
      unit->prev = next;
    } while (unit != side->unithead);
  }
}    

sort_occupants_by_speed(trans)
Unit *trans;
{
  bool flips = TRUE;
  Unit *prevunit, *unit, *nextunit;

  while (flips) {
    flips = FALSE;
    prevunit = NULL;
    unit = trans->occupant;
    while (unit != NULL) {
      if (unit->nexthere != NULL &&
	  utypes[unit->type].speed < utypes[unit->nexthere->type].speed) {
	flips = TRUE;
	nextunit = unit->nexthere;
	if (prevunit != NULL) 
	  prevunit->nexthere = nextunit;
	else trans->occupant = nextunit;
	unit->nexthere = nextunit->nexthere;
	nextunit->nexthere = unit;
	prevunit = nextunit;
      } else {
	prevunit = unit;
	unit = unit->nexthere;
      }
    }
  }
}

/* Return the first unit on the list. */

Unit *
first_unit (side)
Side *side;
{
    return side->unithead;
}

/* Find the side's next unit that can move this turn.
 * Return NULL if not found.  This procedure is only
 * used by movement_phase.
 */
Unit *
next_unit (side, afterunit, onscreen)
Side *side;
Unit *afterunit;
bool onscreen;
{
    register Unit *unit, *start;

    unit = start = afterunit->next;

    do {
        if (!onscreen && unit->orders.type == DELAY)
	  wake_unit(unit, FALSE, WAKEOWNER, NULL);
	if (unit != side->unithead
	    && can_move_unit(side, unit)
	    && (!onscreen || in_middle(side, unit->x, unit->y))) {
	    return unit;
	}
	unit = unit->next;
    } while (unit != start); 
    return NULL;
}

void insert_unit(unithead, unit)
Unit *unithead, *unit;
{

    unit->next = unithead->next;
    unit->prev = unithead;
    unithead->next->prev = unit;
    unithead->next = unit;

}

void insert_unit_tail(unithead, unit)
Unit *unithead, *unit;
{

    unit->next = unithead;
    unit->prev = unithead->prev;
    unithead->prev->next = unit;
    unithead->prev = unit;

}

/* delete the unit pointed to from some list */

void delete_unit(unit)
Unit *unit;
{
  unit->next->prev = unit->prev;
  unit->prev->next = unit->next;
}
