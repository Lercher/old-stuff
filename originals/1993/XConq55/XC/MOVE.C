/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Nothing happens in xconq without movement, and it involves some rather */
/* complicated control structures in order to get the details right. */
/* Units only actually move by following preset orders; when they appear */
/* to be moving under manual control, they are just following orders that */
/* only last for one move before new orders are requested from the player. */

/* Order of movement is sequential by sides, but non-moving sides are in */
/* survey mode and can do things. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "map.h"
#include "global.h"


/* A perimeter is a list of hexes that can be reached in n moves. */

typedef struct _position {
    short x, y;		/* location of hex */
    char dir;		/* first move direction */
} Position;

static Position *perimeters[2];
static char *visited_flags;

extern x_command();

/* Complaints about inability to do various things should only appear if */
/* a human player issues the order to an awake unit. */

#define human_order(u) ((u)->side->directorder && humanside((u)->side))

/* Unit can be awake permanently or temporarily. */

#define is_awake(u) ((u)->orders.type == AWAKE || (u)->awake)

Side *curside;     /* current side for when turns are sequential */


/* Notify the user that his turn is beginning if appropriate */

void start_side_turn(side)
Side *side;
{
/*  bool other_human = FALSE; */
  bool units_move = FALSE;
  Unit *unit;
  Side *loop_side;


  if (humanside(side) && !side->lost) {
    notify(side, "Turn %d beginning ", global.time);
    if ((time(0) - side->laststarttime) > side->startbeeptime) {
      for_all_units(loop_side, unit)
	if ((unit->side == side) && (utypes[unit->type].speed >0)) {
	  units_move = TRUE;
	  continue;
	}
      if (active_display(side) && /* other_human && */ units_move) beep(side);
    }
  }
}

/* Return TRUE if there are units that can move this turn. */

static bool
more_units_to_move ()
{
  Side *side;

    for_all_sides(side) {
	if (side->more_units && !side->lost) 
	    return TRUE;
    }
    return FALSE;
}

/* Find the next unit that the side can move this turn. */

static Unit *
next_unit_to_move (side)
Side *side;
{
    Unit *unit;

    unit = next_unit(side, side->last_unit, TRUE);
    if (unit==NULL) {
      unit = next_unit(side, side->last_unit, FALSE);
      if (Debug)
	printf("%s: no movable units in_middle\n",side->name);
    }
    if (unit == NULL) {
	side->more_units = FALSE;
    } else {
	side->last_unit = unit;
    }
    return unit;
}

set_side_movunit(side)
Side *side;
{
  side->movunit = NULL;
  side->curunit = NULL;
  side->last_unit = first_unit(side);
  side->more_units = TRUE;
  side->movunit = next_unit_to_move(side);
}

/* The movement phases starts by precomputing theoretical maxima for moves, */
/* then does each side one-by-one.  Before iterating, each human side must */
/* be wedged (to ignore input). */

movement_phase()
{
    Unit *unit;
    Side *side, *side2;
    bool human_moving;

    routine("Movement phase");
    enter_procedure("movement_phase");
    if (Debug) printf("Entering movement phase\n");
    compute_moves();
if (Debug) printf("Compute_Moves done.\n");
    for_all_sides(side) 
      if (!side->lost) {
if (Debug) printf("Doing Side Lost.\n");
	side->directorder = FALSE;
	set_side_movunit(side);
if (Debug) printf("Done with set_side_movunit.\n");
	if (can_move_unit(side, side->movunit)) {
	  move_mode(side);
	} else if (humanside(side)) {
	  survey_mode(side);
	  request_command(side);
	}
if (Debug) printf("Before init_machine_turn.\n");
	init_machine_turn(side);  /* Humans could be part robot */
if (Debug) printf("Before start_side_turn.\n");
	start_side_turn(side);
if (Debug) printf("After start_side_turn.\n");
    }

    for_all_sides(side2) {
	update_sides(side2);
	side2->lasttime = time(0);
    }
    /* Move the units. */
    while (more_units_to_move()) {
	human_moving = FALSE;
	for_all_sides(side) {
	    if (!side->lost) {
	        if (!can_move_unit(side, side->movunit) &&
		    side->mode == MOVE) {
		  /* unit probably died while moving or was captured. */
		  cancel_request(side);
		}

		if (!side->reqactive) {
		  if (can_move_unit(side, side->movunit)) {
		    flush_input(side);
		    move_1(side, side->movunit);
		  }
		  if (!can_move_unit(side, side->movunit) &&
		      side->more_units) {
		    side->movunit = next_unit_to_move(side);
		    if (!side->more_units) {
		      for_all_sides(side2) {
			update_sides(side2);
		      }
		    }
		  }
		  if (!can_move_unit(side, side->movunit) &&
		      humanside(side) && side->mode != SURVEY) {
		    survey_mode(side);
		  }
		}
		if (humanside(side) &&
		    (can_move_unit(side, side->movunit) ||
		     side->reqhandler != x_command) 
		    && side->reqactive)
		    human_moving = TRUE;
		if (!side->reqactive && humanside(side) &&
		    side->mode == SURVEY) {
		    request_command(side);
		}
	    }
	}
	if (human_moving)
	    handle_requests();
    }
    while (someone_doing_something()) 
      handle_requests();

    /* Remove dead units that have been dead more than a turn. */
    for_all_sides (side) {
      flush_side_dead(side);
      /* Clear wakeup reasons for non-movers. */
      /* need to fix this so that these stay around for a turn. */
      for_all_side_units(side, unit) 
	if (utypes[unit->type].speed == 0 && unit->side == side)
	  unit->wakeup_reason = WAKEOWNER;
      side->laststarttime = time(0);
    }

    /* For display hacking, also to find misc bugs :-) */
    curside = NULL;
    for_all_sides(side)  {
	if (humanside(side) && !side->lost) {
	    update_sides(side);
	    cancel_request(side);
	}
    }
    /* Do all sentrys now. */
    for_all_sides(side)
      for_all_side_units(side, unit) {
	if (delayed_order(unit->orders.type)) {
	  unit->orders.rept--;
	  maybe_wakeup(unit);
	}
      }
    exit_procedure();
}

/* Compute moves for all the units at once. */

compute_moves()
{
    Unit *unit;
    Side *side;

    if (midturnrestore)
      midturnrestore = FALSE;
    else /* for all non-neutral units. */
      for_all_sides(side)
	for_all_side_units(side, unit) {
	  unit->movesleft = compute_move(unit);
	  unit->actualmoves = 0;
	}
}

/* Compute number of moves available to the units.  This is complicated by */
/* reduction of movement due to damage and the effect of occupants on */
/* mobility.  Also, we never let a moving unit have a movement of zero, */
/* unless it is out of movement supplies. */

/* Moves should be recomputed and possibly adjusted downward after a hit. */

compute_move(unit)
Unit *unit;
{
    int u = unit->type, r, moves = 0;
    Unit *occ;

    if (!neutral(unit) && (moves = utypes[u].speed) > 0) {
	if (cripple(unit)) {
	    moves = (moves * unit->hp) / (utypes[u].crippled + 1);
	}
	for_all_occupants(unit, occ) {
	    if (utypes[u].mobility[occ->type] != 100) {
		moves = (moves * utypes[u].mobility[occ->type]) / 100;
		break;
	    }
	}
	moves = max(1, moves);
	for_all_resource_types(r) {
	    if (utypes[u].tomove[r] > 0 && unit->supply[r] <= 0) {
		moves = 0;
		break;
	    }
	}
    }
    return moves;
}

/* To move a single unit, keep iterating until all its moves are used up, */
/* or it dies, or its movement has been postponed until the other units */
/* have been done.  A single move is either under preset orders, or must be */
/* supplied by a human player, or computed by a machine player. */

move_1(side, unit)
Side *side;
Unit *unit;
{

    if (Debug) printf("%s going to move\n", unit_handle((Side *) NULL, unit));
    if (can_move_unit(side, unit)) {
	if (side->mode == MOVE) make_current(side, unit);
	if (idled(unit) && global.setproduct) {
	  request_new_product(unit);
	} else if (!is_awake(unit) && side->mode == MOVE) {
	    follow_order(unit);
	} else if (humanside(side) && under_control(unit)) {
	  side->directorder = TRUE;
	  request_command(side);
	  return; /* why? */
	} else {
	    show_info(side);
	    machine_move(unit);
	}
    }
}

/* Switching to move mode involves shifting from wherever the cursor is, */
/* back to the unit that was being moved earlier. */

move_mode(side)
Side *side;
{
    int oldmode = side->mode;

    if (Debug) printf("%s side in move mode\n", side->name);
    side->mode = MOVE;
    if (side->mode != oldmode) show_timemode(side,TRUE);
}

/* Switching to survey mode */

survey_mode(side)
Side *side;
{
    int oldmode = side->mode;

    if (Debug) printf("%s side in survey mode\n", side->name);
    side->mode = SURVEY;
    if (side->movunit == NULL) side->movunit = side->curunit;
    make_current(side, unit_at(side->curx, side->cury));
    /* always do show info to get up to dateness info. */
    /* if (side->curunit != side->movunit) */ show_info(side);
    if (side->mode != oldmode) show_timemode(side,TRUE);
}

/* Test if a unit (on a human side) is actually under manual control. */

under_control(unit)
Unit *unit;
{
    return (!unit->orders.morder);
}

/* Set the "current" unit of a side - the one being displayed, moved, etc. */

make_current(side, unit)
Side *side;
Unit *unit;
{
    if (unit != NULL && alive(unit) &&
	(allied_side(unit->side,side) || Debug || Build)) {
	side->curunit = unit;
	side->curx = unit->x;  side->cury = unit->y;
    } else {
	side->curunit = NULL;
    }
}

unowned_p(x, y)
int x, y;
{
    Unit *unit = unit_at(x, y);

    return (unit == NULL || unit->side != tmpside);
}

goto_empty_hex(side)
Side *side;
{
    int x, y;

    tmpside = side;
    if (search_area(side->curx, side->cury, 20, unowned_p, &x, &y, 1)) {
	side->curx = x;  side->cury = y;
	side->curunit = NULL;
    }
}

/* Do single move of a single order for a given unit. */

char *unit_desig();

follow_order(unit)
Unit *unit;
{
    bool success = FALSE;
    int u = unit->type;
    Side *us = unit->side;

    if (Debug) printf("%d: %s doing %s with %d moves left\n",
		      global.time, unit_desig(unit),
		      order_desig(&(unit->orders)), unit->movesleft);
/* somewhere this has a right home */
/*    unit->awake = FALSE;  */
    switch (unit->orders.type) {
    case FILL:
    case SENTRY:
    case EMBARK:
      /* handled in code at end of movement phase */
	break;
    case MOVEDIR:
	success = move_dir(unit, unit->orders.p.dir);
	break;
    case MOVETO:
	success = move_to_dest(unit);
	break;
    case MOVETOTRANSPORT:
	success = move_to_transport(unit);
	break;
    case MOVETOUNIT:
	success = move_toward_unit(unit,unit->orders.p.leader_id);
	break;
    case EDGE:
	success = follow_coast(unit);
	break;
    case FOLLOW:
	success = follow_leader(unit);
	break;
    case PATROL:
	success = move_patrol(unit);
	break;
    case RETURN:
	success = return_to_base(unit);
	break;
    case AWAKE:
    case NONE:
    default:
        case_panic("order type", unit->orders.type);
    }
    if (alive(unit)) {
	if (success) {
	    unit->movesleft -=
		max(1,1 + utypes[u].moves[terrain_at(unit->x, unit->y)]);
	}
	/* clear all wakeup messages */
	unit->wakeup_reason = WAKEOWNER;
	if (!us->directorder || success) {
	    maybe_wakeup(unit);
	}
    }
    us->directorder = FALSE;
    if (!success) unit->move_tries++;
}

/* Force unit to try to move in given direction. */

move_dir(unit, dir)
Unit *unit;
int dir;
{
    if (unit->orders.rept-- > 0) {
	return move_to_next(unit, dirx[dir], diry[dir], TRUE, TRUE);
    }
    return FALSE;
}

/* Have unit try to move to its ordered position. */

move_to_dest(unit)
Unit *unit;
{
  return move_to(unit, unit->orders.p.pt[0].x, unit->orders.p.pt[0].y,
		 (unit->orders.flags & SHORTESTPATH));
}

/* move to transport */

bool
filling_transport(transport, unit)
     Unit	*transport, *unit;
{
  return transport->orders.type == FILL &&
    can_carry(transport, unit);
}

/* static variable set whenever reached_filling_transport_aux finds a filling
   transport */
static Unit	*fillingtransport;

reached_filling_transport_aux(unit, maybe)
     Unit	*unit, *maybe;
{
  Unit	*occ;

  if (maybe==NULL)
    return FALSE;

  if (filling_transport(maybe, unit)) {
    fillingtransport = maybe;
    return TRUE;
  }
  for_all_occupants(maybe, occ) {
    if (reached_filling_transport_aux(unit, occ)) {
      return TRUE;
    }
  }
  return FALSE;
}
reached_filling_transport(unit, x, y)
     Unit	*unit;
     int	x,y;
{
  return reached_filling_transport_aux(unit, unit_at(x,y));
}

move_to_transport(unit)
     Unit	*unit;
{
  int	dir;
  if (reached_filling_transport(unit, unit->x, unit->y)) {
    wake_unit(unit, FALSE, WAKEOWNER, NULL); /* wake now in case there are
						standing orders */
    leave_hex(unit);
    occupy_unit(unit, fillingtransport);
    return FALSE;
  }

  if (--unit->orders.rept <= 0) {
    wake_unit(unit, FALSE, WAKETIME, NULL);
    return FALSE;
  }

  dir = search_path(unit, 30, reached_filling_transport);
  if (dir<0) {
    notify(unit->side, "%s couldn't find filling transport",
	   unit_handle(unit->side, unit));
    wake_unit(unit, FALSE, WAKEOWNER, NULL);
    return FALSE;
  }
  return move_to_next(unit, dirx[dir], diry[dir], TRUE, TRUE);
}

/* move toward a unit */
move_toward_unit(unit, lID)
     Unit	*unit;
     int	lID; /* leader ID */
{
  Unit	*dest;

  if (--unit->orders.rept <= 0) {
    wake_unit(unit, FALSE, WAKETIME, NULL);
    return FALSE;
  }
  dest = find_unit(lID);
  if (dest==NULL) {
    notify(unit->side, "Destination unit is dead!");
    wake_unit(unit, FALSE, WAKELEADERDEAD, (Unit *) NULL);
    return FALSE;
  } else if (!can_carry(dest, unit)) {
    notify(unit->side, "%s can't carry us!", unit_handle(unit->side, dest));
    wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
    return FALSE;
  } else if (unit->x == dest->x && unit->y == dest->y) {
    leave_hex(unit);
    occupy_unit(unit, dest);
    wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
    return FALSE;
  } else {
    return move_to(unit, dest->x, dest->y,
		   (unit->orders.flags & SHORTESTPATH));
  }
}

/* this algorithm requires the direction of movement and the position
of the border hex.
  forward is the direction the unit last moved in.
  ccw tells us where the border hex WAS. */

/* The direction-munging things should be abstracted. */

follow_coast(unit)
Unit *unit;
{
    Edge	*edge = &unit->orders.p.edge;
    int ccw = edge->ccw, olddir = edge->forward;
    int newdir;

    if (ccw==0) {
      /* we are trapped in an isthmus */
      int	count=0, workingdir = (-1);

#define trydir(d) newdir = (d); \
      if (could_move_in_dir(unit,newdir)) \
	  { count++; workingdir = newdir; }

      trydir(left_of(olddir));
      trydir(olddir);
      trydir(right_of(olddir));
#undef trydir
      if (count>1) {/* too many choices */
	wake_unit(unit, FALSE, WAKELOST, NULL);
	notify(unit->side, "%s can't decide which coast to follow!",
	       unit_handle(unit->side, unit));
	return FALSE;
      } else if (count == 1) /* follow the isthmus */
	move_dir(unit, edge->forward = workingdir);
      else /* found the end.  backtrack */
	move_dir(unit, edge->forward = opposite_dir(olddir));

      return TRUE; /* the failure returns FALSE; for itself */

    } else {
      /* we have an edge direction */
      int	lastchance = normalize_dir(olddir + ccw*2);
      for ( newdir = normalize_dir(olddir + ccw);
	   newdir != lastchance; newdir = normalize_dir(newdir-ccw)) {
	if (could_move_in_dir(unit, newdir))
	  break;
      }
      if (could_move_in_dir(unit, newdir)) {
	move_dir(unit, edge->forward = newdir);
	return TRUE;
      } else {
	notify(unit->side, "ERROR: forward:%d ccw:%d @%d,%d", edge->forward,
	       edge->ccw, unit->x, unit->y);
	wake_unit(unit, FALSE, WAKELOST, NULL);
	notify(unit->side, "%s can't figure out where to move to!",
	       unit_handle(unit->side, unit));
	return FALSE;
      }
    }
}

direction_works(unit, dir)
Unit *unit;
int dir;
{
    int nx, ny;

    nx = wrap(unit->x + dirx[dir]);  ny = limit(unit->y + diry[dir]);
    if (could_move(unit->type, terrain_at(nx, ny)) &&
	adj_obstacle(unit->type, nx, ny)) {
	move_dir(unit, dir);
	return TRUE;
    } else {
	return FALSE;
    }
}

adj_obstacle(type, x, y)
int type, x, y;
{
    int d, x1, y1;

    for_all_directions(d) {
	x1 = wrap(x + dirx[d]);  y1 = limit(y + diry[d]);
	if (!could_move(type, terrain_at(x1, y1))) return TRUE;
    }
    return FALSE;
}

/* Unit attempts to follow its leader around, but not too closely. */

follow_leader(unit)
Unit *unit;
{
    Unit *leader = find_unit(unit->orders.p.leader_id);
    int	dx;

    if ( --unit->orders.rept <=0 ) {
      wake_unit(unit, FALSE, WAKETIME, NULL);
      return FALSE;
    }
    if (leader == NULL) {   /* find unit won't return dead leader */
	notify(unit->side, "Leader is dead.  I am without guidance!");
	wake_unit(unit, FALSE, WAKELEADERDEAD, (Unit *) NULL);
	return FALSE;
    } else if ( ( (dx=abs(unit->x - leader->x)) < 2 ||
		 /* handle map wraps! */ dx > world.width-2 )  &&
	       abs(unit->y - leader->y) < 2) {
	unit->movesleft = 0;
	return TRUE;
    } else {
	return move_to(unit, leader->x, leader->y, FALSE);
    }
}

/* Patrol just does move_to, but cycling waypoints around when the first */
/* one has been reached. */

move_patrol(unit)
Unit *unit;
{
    int tx, ty;

    if (unit->orders.rept-- > 0) {
	if (unit->x == unit->orders.p.pt[0].x &&
	    unit->y == unit->orders.p.pt[0].y) {
	    tx = unit->orders.p.pt[0].x;
	    ty = unit->orders.p.pt[0].y;
	    unit->orders.p.pt[0].x = unit->orders.p.pt[1].x;
	    unit->orders.p.pt[0].y = unit->orders.p.pt[1].y;
	    unit->orders.p.pt[1].x = tx;
	    unit->orders.p.pt[1].y = ty;
	}
	return move_to(unit, unit->orders.p.pt[0].x, unit->orders.p.pt[0].y,
		       (unit->orders.flags & SHORTESTPATH));
    }
    return TRUE;
}


/* Auxiliary stuff used when searching for place to return to.  Note that */
/* a good refueling spot will be woken up, so it won't get too far away */
/* before unit has a chance to get there. */
/* Won't find refueling places inside other units, sigh. */

refuel_here(unit, x, y)
     Unit	*unit;
     int	x, y;
{
    Unit *transport = unit_at(x, y);

    enter_procedure("refuel_here");
    if (transport != NULL && transport->side == unit->side &&
	can_carry(transport, unit)) {
      wake_unit(transport, FALSE, WAKEOWNER, (Unit *) NULL);
      exit_procedure();
      return TRUE;
    }
    exit_procedure();
    return FALSE;
}

return_to_base(unit)
Unit *unit;
{
  int	dir;
  if (refuel_here(unit, unit->x, unit->y)) {
    wake_unit(unit, FALSE, WAKEOWNER, NULL);
    return FALSE;
  }

  if (--unit->orders.rept <= 0) {
    wake_unit(unit, FALSE, WAKETIME, NULL);
    return FALSE;
  }

  dir = search_path(unit, 30, refuel_here);
  if (dir<0) {
    notify(unit->side, "%s couldn't find refueling point",
	   unit_handle(unit->side, unit));
    wake_unit(unit, FALSE, WAKEOWNER, NULL);
    return FALSE;
  }
  return move_to_next(unit, dirx[dir], diry[dir], TRUE, TRUE);
}

/* Retreat is a special kind of movement. */
/* Veterans should get several tries at retreating to a good place, perhaps */
/* one try per point of "veteranness"? */

retreat_unit(unit)
Unit *unit;
{
    int dir;
    bool success;

    dir = random_dir();
    success = move_to_next(unit, dirx[dir], diry[dir], FALSE, FALSE);
    return success;
}


static int	unitdestx, unitdesty;
static bool
reached_dest(unit, x, y)
     Unit	*unit;
     int	x,y;
{
  return x == unitdestx
    && y == unitdesty;
}

/* This is the general routine for finding a path to a given point. */

move_to(unit, tx, ty, shortest)
     Unit *unit;
     int tx, ty;
     bool shortest;
{
    Side *side = unit->side;
    int	dir;
    int r;

    unitdestx = tx;  unitdesty = ty;
    if (unit->x == tx && unit->y == ty) {
	wake_unit(unit, FALSE, WAKEOWNER, NULL);
	return FALSE;
    }
    dir = search_path(unit, 30, reached_dest);
    if (dir < 0) {
	/* We failed to find a path, but could just try moving anyway. */
#if 0
	if (side_view(unit->side, tx, ty) == UNSEEN) {
	    return wander_to(unit, tx, ty, shortest);
	} else {
#endif
	    wake_unit(unit, FALSE, WAKEOWNER, NULL);
#ifdef VERBOSE
	    notify(unit->side, "%s couldn't find path from %d,%d to %d,%d",
		   unit_handle(side, unit), unit->x, unit->y, tx, ty);
#endif VERBOSE
	    if (Debug) {
		printf("%s couldn't find path from %d,%d(%s) to %d,%d(%s)\n",
		       unit_handle(side, unit), unit->x, unit->y,
		       ttypes[terrain_at(unit->x,unit->y)].name,
		       tx, ty, ttypes[terrain_at(tx, ty)].name);
		for_all_resource_types (r) {
		    printf("%s:%d ",rtypes[r].name, unit->supply[r]);
		}
		printf("\n");
	    }
	    return FALSE;
#if 0
	}
#endif
    } else {
	/* I'm completely ignoring any orders flags */
	return move_to_next(unit, dirx[dir], diry[dir], TRUE, TRUE);
    }
}

/* This weird-looking routine computes next directions for moving to a */
/* given spot.  The basic strategy is to prefer to go in the x or y distance */
/* that is the greatest difference, so as to even the two displacements out. */
/* (This leaves more options open if blockage several hexes away.)  Make it */
/* probabilistic, so repeated travel will eventually trace the envelope of */
/* possible moves.  The number of directions ranges from 1 to 4, depending */
/* on whether there is a straight-line path to the dest, and whether we are */
/* required to take a direct path or are allowed to move in dirs that don't */
/* the unit any closer (we never increase our distance though). */
/* Some trickinesses:  world is cylindrical, so must resolve ambiguity about */
/* getting to the same place going either direction (we pick shortest). */

wander_to(unit, tx, ty, shortest)
Unit *unit;
int tx, ty;
bool shortest;
{
    bool closer, success;
    int dx, dxa, dy, dist, d1, d2, d3, d4, axis = -1, hextant = -1, tmp;

    dist = distance(unit->x, unit->y, tx, ty);
    dx = tx - unit->x;  dy = ty - unit->y;

    dxa = (tx + world.width) - unit->x;
    if (abs(dx) > abs(dxa)) dx = dxa;
    dxa = (tx - world.width) - unit->x;
    if (abs(dx) > abs(dxa)) dx = dxa;
    if (dx == 0 && dy == 0) {
	wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
	return FALSE;
    }
    axis = hextant = -1;
    if (dx == 0) {
	axis = (dy > 0 ? NE : SW);
    } else if (dy == 0) {
	axis = (dx > 0 ? EAST : WEST);
    } else if (dx == (0 - dy)) {
	axis = (dy > 0 ? NW : SE);
    } else if (dx > 0) {
	hextant = (dy > 0 ? EAST : (abs(dx) > abs(dy) ? SE : SW));
    } else {
	hextant = (dy < 0 ? WEST : (abs(dx) > abs(dy) ? NW : NE));
    }
    if (axis >= 0) {
	d1 = d2 = axis;
    }
    if (hextant >= 0) {
	d1 = left_of(hextant);
	d2 = hextant;
    }
    d3 = left_of(d1);
    d4 = right_of(d2);
    closer = (shortest || dist == 1);
    if (flip_coin()) {
        tmp = d1;  d1 = d2; d2 = tmp;
    }
    success = move_to_next(unit, dirx[d1], diry[d1], FALSE, 1);
    if (!success)
	success = move_to_next(unit, dirx[d2], diry[d2], closer, 1);
    if (!success && !closer) {
	if (opposite_dir(unit->lastdir) == d3) {
	    success = move_to_next(unit, dirx[d4], diry[d4], TRUE, 1);
	} else if (opposite_dir(unit->lastdir) == d4) {
	    success = move_to_next(unit, dirx[d3], diry[d3], TRUE, 1);
	} else {
	    success = move_to_next(unit, dirx[d3], diry[d3], FALSE, 1);
	    if (!success)
		success = move_to_next(unit, dirx[d4], diry[d4], TRUE, 1);
	}
    }
    return success;
}

/* This function and a couple auxes encode most of the rules about how */
/* units can move.  Attempts to move onto other units are handled */
/* by other functions below.  Unit will also refuse to move onto the edge of */
/* the map or into the wrong kind of terrain.  Otherwise, it succeeds in its */
/* movement and we put it at the new spot.  If at any time, the unit */
/* could not move and yet was supposed to, it will wake up. */

move_to_next(unit, dx, dy, mustgo, atk)
Unit *unit;
int dx, dy;
bool mustgo, atk;
{
    bool offcourse = FALSE, success = FALSE;
    int nx, ny, utype = unit->type;
    Unit *unit2;
    Side *us = unit->side;

    nx = wrap(unit->x + dx);  ny = unit->y + dy;

    if ((unit2 = unit_at(nx, ny)) != NULL) {
	success = move_to_unit(unit, unit2, dx, dy, mustgo, atk, offcourse);
    } else if (!between(1, ny, world.height-2)) {
	if (global.leavemap) {
	    kill_unit(unit, DISBAND);
	} else if (offcourse) {
	    notify(us, "%s has fallen off the edge of the world!",
		   unit_handle(us, unit));
	    kill_unit(unit, DISASTER);
	} else if (human_order(unit) && mustgo) {
	    cmd_error(us, "%s can't leave this map!",
		      unit_handle(us, unit));
	}
    } else if (!could_move(utype, terrain_at(nx, ny))) {
	if (offcourse) {
	    notify(us, "%s has met with disaster!", unit_handle(us, unit));
	    kill_unit(unit, DISASTER);
	} else if (human_order(unit) && mustgo) {
	    cmd_error(us, "%s won't go into the %s!",
		      unit_handle(us, unit), ttypes[terrain_at(nx, ny)].name);
	}
    } else {
	move_unit(unit, nx, ny);
	unit->lastdir = find_dir(dx, dy);
	success = TRUE;
    }
    /* Units don't get dead by failing to move, so test not needed here. */
    if (!success && mustgo) {
/*	unit->awake = TRUE;  */
	wake_unit(unit, FALSE, WAKELOST, (Unit *) NULL);
    }
    return success;
}

/* An enemy unit will be attacked, unless unit is on a transport *and* it */
/* cannot move to that hex anyway. */
/* Also will refuse if hit prob < 10% (can still defend tho) */
/* If the attackee is destroyed, then unit will attempt to move in again. */
/* A friendly transport will be boarded unless it is full. */
/* Blank refusal to move if any other unit. */

/* Allies treat each other's units as their own. */

move_to_unit(unit, unit2, dx, dy, mustgo, atk, offcourse)
Unit *unit, *unit2;
int dx, dy;
bool mustgo, atk, offcourse;
{
    int u = unit->type, u2 = unit2->type, u2x = unit2->x, u2y = unit2->y;
    Side *us = unit->side;

    if (!allied_side(us, unit2->side)) {
	if (unit->transport != NULL && impassable(unit, u2x, u2y)
	    && !utypes[u2].bridge[u]) {
	    if (human_order(unit) && mustgo) {
		cmd_error(us, "%s can't attack there!", unit_handle(us, unit));
	    }
	} else if (side_view(us, u2x, u2y) == EMPTY) {
	    notify(us, "%s spots something!", unit_handle(us, unit));
	    see_exact(us, u2x, u2y);
	    draw_hex(us, u2x, u2y, TRUE);
	} else if (atk && (unit->orders.flags & ATTACKUNIT)) {
	    if (!could_hit(u, u2) && !could_capture(u, u2)) {
		if (human_order(unit) && mustgo) {
		    cmd_error(us, "%s refuses to attack a %s!",
			      unit_handle(us, unit), utypes[u2].name);
		}
	    } else if (!enough_ammo(unit, unit2)) {
		if (human_order(unit) && mustgo) {
		    cmd_error(us, "%s is out of ammo!", unit_handle(us, unit));
		}
	    } else {
		if (attack(unit, unit2)) {
		    /* if battle won, can try moving again */
		    return move_to_next(unit, dx, dy, FALSE, FALSE);
		}
		return TRUE;
	    }
	} else {
	    /* here if aircraft blocked on return, etc - no action needed */
	}
    } else if (could_carry(u2, u)) {
	if (!can_carry(unit2, unit)) {
	    if (human_order(unit) && mustgo) {
		cmd_error(us, "%s is full already!", unit_handle(us, unit2));
	    }
	} else {
	    move_unit(unit, u2x, u2y);
	    unit->movesleft -= utypes[u].entertime[u2];
	    unit->lastdir = find_dir(dx, dy);
	    return TRUE;
 	}
    } else if (could_carry(u, u2)) {
	if (impassable(unit, u2x, u2y)) {
	    if (human_order(unit) && mustgo) {
		cmd_error(us, "%s can't pick up anybody there!",
			  unit_handle(us, unit));
	    }
	} else if (!can_carry(unit, unit2)) {
	    if (human_order(unit) && mustgo) {
		cmd_error(us, "%s is full already!", unit_handle(us, unit));
	    }
	} else if (unit->orders.flags == 0) {
	    /* blow out at the bottom */
	} else {
	    move_unit(unit, u2x, u2y);
	    unit->lastdir = find_dir(dx, dy);
	    return TRUE;
	}
    } else {
	if (offcourse) {
	    notify(us, "%s is involved in a wreck!", unit_handle(us, unit));
	    kill_unit(unit, DISASTER);
	    kill_unit(unit2, DISASTER);
	} else if (human_order(unit) && mustgo) {
	    cmd_error(us, "%s refuses to attack its friends!",
		      unit_handle(us, unit));
	}
    }
    return FALSE;
}

/* Perform the act of moving proper. (very simple, never fails) */
/* This is also the right place to put in anything that happens if the *
/* unit actually changes its location. */

move_unit(unit, nx, ny)
Unit *unit;
int nx, ny;
{

    cancel_build(unit);
    leave_hex(unit);
    occupy_hex(unit, nx, ny);
    wake_neighbors(unit);
    consume_move_supplies(unit);
}

/* This routine is too strict, doesn't account for resupply at start of next */
/* turn.  Hacked to check on transport at least. */
/* Only die now if will die during consumption phase. */

consume_move_supplies(unit)
Unit *unit;
{
    int u = unit->type, r;
    
    unit->actualmoves++;
    for_all_resource_types(r) {
	if (utypes[u].tomove[r] > 0) {
	    unit->supply[r] -= utypes[u].tomove[r];
	    if (unit->supply[r] <= 0 && unit->transport == NULL) {
		if (utypes[u].consume[r] > 0) {
		    exhaust_supply(unit);
		    return;
		}
	    }
	}
    }
}

/* When doing a survey, you can move the cursor anywhere and it will show */
/* what is at that hex, but not give away too much! */
extern Side *mside;
extern Unit *munit;
extern int max_range, munit_regions;
move_survey(side, nx, ny)
Side *side;
int nx, ny;
{
    if (between(1, ny, world.height-2)) {
	side->curx = nx;  side->cury = ny;
	make_current(side, unit_at(side->curx, side->cury));
    } else {
	if (active_display(side)) beep(side);
    }
    put_on_screen(side, side->curx, side->cury);
    show_info(side);
/*    if (side->movunit != NULL) {
      mside = side;
      munit = side->movunit;
      munit->goal = 1;
      munit_regions = regions_around(munit->type, nx, ny, FALSE);
      max_range = 1000;
      notify(side, "Cost for hex is %d ",
	     evaluate_hex(nx, ny));
    } */
}

/* This routine encodes nearly all of the conditions under which a unit */
/* following orders might wake up and request new instructions. */

maybe_wakeup(unit)
Unit *unit;
{
    bool goinghome;
    int tx, ty;

    if (unit->orders.rept <= 0) {
	wake_unit(unit, FALSE, WAKETIME, (Unit *) NULL);
    } else if ((unit->orders.flags & SUPPLYWAKE) && low_supplies(unit)) {
	goinghome = FALSE;
	if (unit->orders.type == MOVETO) {
	    tx = unit->orders.p.pt[0].x;  ty = unit->orders.p.pt[0].y;
	    if (refuel_here(unit,tx,ty)) { /* shouldn't wake the transport? */
		goinghome = TRUE;
	    }
	}
	if (!goinghome) {
	    wake_unit(unit, FALSE, WAKERESOURCE, (Unit *) NULL);
	    unit->orders.flags &= ~SUPPLYWAKE;
	}
    } /*
	Don't need this since a unit is woken up when another moves
	next to it or it moves next to another.  If you give a unit
	orders near the enemy, your problem.
	else if ((unit->orders.flags & ENEMYWAKE) && adj_enemy(unit)) {
	wake_unit(unit, FALSE, WAKEENEMY, other);
    } */
}

/* True if unit is adjacent to an unfriendly. */

adj_enemy(unit)
Unit *unit;
{
    int d, x, y;
    viewdata view;

    for_all_directions(d) {
	x = wrap(unit->x + dirx[d]);  y = unit->y + diry[d];
	if (!neutral(unit)) {
	    view = side_view(unit->side, x, y);
	    if (view != EMPTY && enemy_side(side_n(vside(view)), unit->side))
		return TRUE;
	}
    }
    return FALSE;
}

/* Wake up anyone who is next to us if they see us. */

wake_neighbors(unit)
Unit *unit;
{
  int d, x, y;
  viewdata view;
  Unit *other;
  
  for_all_directions(d) {
    x = wrap(unit->x + dirx[d]);  y = unit->y + diry[d];
    if (!neutral(unit)) {
      view = side_view(unit->side, x, y);
      if (view != EMPTY && enemy_side(side_n(vside(view)), unit->side)) 
	wake_unit(unit, FALSE, WAKEENEMY, unit_at(x,y));
    }
    if ((other = unit_at(x,y)) != NULL) 
      if (!neutral(other)) {
	view = side_view(other->side, unit->x, unit->y);
	if (view != EMPTY && enemy_side(other->side, unit->side)) 
	  wake_unit(other, TRUE, WAKEENEMY, unit);
      }
  }
}
