/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Handling of unit orders. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"

char *ordernames[] = ORDERNAMES;   /* full names of orders */
char *dirnames[] = DIRNAMES;       /* short names of directions */
char orderbuf[BUFSIZE];            /* buffer for printed form of an order */
char oargbuf[BUFSIZE];             /* buffer for order's arguments */

int orderargs[] = ORDERARGS;       /* types of parameters for each order */

/* General routine to wake a unit up (and maybe all its cargo). */

wake_unit(unit, wakeocc, reason, other)
Unit *unit, *other;
int reason;
bool wakeocc;
{
    Unit *occ;

/* make sure that a side always gets to move a unit that is woken up
late in the turn */

    if (unit->side != NULL && unit->movesleft > 0 &&
	alive(unit) && !unit->side->more_units) {
      unit->side->more_units = TRUE;
      update_sides(unit->side);
      move_mode(unit->side);
      unit->side->movunit = unit;
      cancel_request(unit->side);
    }

    unit->orders.type = AWAKE;
    unit->orders.rept = 0;
    unit->orders.flags = NORMAL;
    unit->wakeup_reason = reason;
    if (reason == WAKEFULL) {
      unit->orders.morder = FALSE;
    }
    if (reason == WAKEOWNER)
      unit->area = area_index(unit->x, unit->y);
    if (reason == WAKEENEMY && other!=NULL) {
      unit->waking_side = other->side;
      unit->waking_type = other-> type;
      unit->area = area_index(unit->x, unit->y);
    }
    if (wakeocc) {
	for_all_occupants(unit, occ) wake_unit(occ, wakeocc, reason, other);
    }
}

/* Stash a "wakeup call" - will only be for main unit, not occupants. */

cache_awake(side)
Side *side;
{
    side->tmporder->type = AWAKE;
    side->tmporder->rept = 0;
    side->tmporder->morder = FALSE;
    finish_teach(side);
}

/* Give a unit sentry orders. */

order_sentry(unit, n)
Unit *unit;
int n;
{
    unit->orders.type = SENTRY;
    unit->orders.rept = n;
}

/* Stash sentry orders. */

cache_sentry(side, n)
Side *side;
int n;
{
    side->tmporder->type = SENTRY;
    side->tmporder->rept = n;
    finish_teach(side);
}

/* Fill in the given unit with direction-moving orders. */

order_movedir(unit, dir, n)
Unit *unit;
int dir, n;
{
    unit->orders.type = MOVEDIR;
    unit->orders.p.dir = dir;
    unit->orders.rept = n;
}

cache_movedir(side, dir, n)
Side *side;
int dir, n;
{
    side->tmporder->type = MOVEDIR;
    side->tmporder->p.dir = dir;
    side->tmporder->rept = n;
    finish_teach(side);
}

/* Give the unit orders to move to a given place - it only needs to do this */
/* once, repetition is nonsensical. */

order_moveto(unit, x, y)
Unit *unit;
int x, y;
{
    unit->orders.type = MOVETO;
    unit->orders.rept = 1;
    unit->orders.p.pt[0].x = x;
    unit->orders.p.pt[0].y = y;
}

cache_moveto(side, x, y)
Side *side;
int x, y;
{
    side->tmporder->type = MOVETO;
    side->tmporder->rept = 1;
    side->tmporder->p.pt[0].x = x;
    side->tmporder->p.pt[0].y = y;
    finish_teach(side);
}

/* order a unit to move toward another unit */

order_movetounit(unit, dest, n)
Unit	*unit, *dest;
int	n;
{
  unit->orders.type = MOVETOUNIT;
  unit->orders.rept = n;
  unit->orders.p.leader_id = dest->id;
}

cache_movetounit(side, dest, n)
Side	*side;
Unit	*dest;
int	n;
{
  side->tmporder->type = MOVETOUNIT;
  side->tmporder->rept = n;
  side->tmporder->p.leader_id = dest->id;
  finish_teach(side);
}

/* order a unit to return to base */

order_return(unit, n)
Unit *unit;
int n;
{
  unit->orders.type = RETURN;
  unit->orders.rept = n;
}

cache_return(side, n)
Side *side;
int n;
{
  side->tmporder->type = RETURN;
  side->tmporder->rept = n;
  finish_teach(side);
}

/* Order to follow an edge needs to know the direction and which side the
   obstacle is on. */

order_edge(unit, d, ccw, n)
Unit *unit;
int d, ccw, n;
{
    unit->orders.type = EDGE;
    unit->orders.rept = n;
    unit->orders.p.edge.forward = d;
    unit->orders.p.edge.ccw = ccw;
}

cache_edge(side, d, ccw, n)
Side *side;
int d, ccw, n;
{
    side->tmporder->type = EDGE;
    side->tmporder->rept = n;
    side->tmporder->p.edge.forward = d;
    side->tmporder->p.edge.ccw = ccw;
    finish_teach(side);
}

/* Order to move to the nearest filling transport */

order_movetotransport(unit, n)
     Unit	*unit;
     int	n;
{
  unit->orders.type = MOVETOTRANSPORT;
  unit->orders.rept = n;
}

cache_movetotransport(side, n)
     Side	*side;
     int	n;
{
  side->tmporder->type = MOVETOTRANSPORT;
  side->tmporder->rept = n;
  finish_teach(side);
}

/* Order to follow a unit just needs the unit to follow. */

order_follow(unit, leader, n)
Unit *unit, *leader;
int n;
{
    unit->orders.type = FOLLOW;
    unit->orders.rept = n;
    unit->orders.p.leader_id = leader->id;;
}

cache_follow(side, leader, n)
Side *side;
Unit *leader;
int n;
{
    side->tmporder->type = FOLLOW;
    side->tmporder->rept = n;
    side->tmporder->p.leader_id = leader->id;
    finish_teach(side);
}

/* A two-waypoint patrol suffices for many purposes. */
/* Should have a more general patrol routine eventually (> 2 waypoints). */

order_patrol(unit, x0, y0, x1, y1, n)
Unit *unit;
int x0, y0, x1, y1, n;
{
    unit->orders.type = PATROL;
    unit->orders.rept = n;
    unit->orders.p.pt[0].x = x0;
    unit->orders.p.pt[0].y = y0;
    unit->orders.p.pt[1].x = x1;
    unit->orders.p.pt[1].y = y1;
}

cache_patrol(side, x0, y0, x1, y1, n)
Side *side;
int x0, y0, x1, y1, n;
{
    side->tmporder->type = PATROL;
    side->tmporder->rept = n;
    side->tmporder->p.pt[0].x = x0;
    side->tmporder->p.pt[0].y = y0;
    side->tmporder->p.pt[1].x = x1;
    side->tmporder->p.pt[1].y = y1;
    finish_teach(side);
}


/* Wait around to embark on some unit when it comes to our location. */

order_embark(unit, n)
Unit *unit;
int n;
{
    unit->orders.type = EMBARK;
    unit->orders.rept = n;
}

cache_embark(side, n)
Side *side;
int n;
{
    side->tmporder->type = EMBARK;
    side->tmporder->rept = n;
    finish_teach(side);
}

order_fill(unit, n)
Unit *unit;
int n;
{
    unit->orders.type = FILL;
    unit->orders.rept = n;
}

cache_fill(side, n)
Side *side;
int n;
{
    side->tmporder->type = FILL;
    side->tmporder->rept = n;
    finish_teach(side);
}

/* Note that this leaves all other orders the same. */

cache_auto(side)
Side *side;
{
  Order *oldorder;

  if ((oldorder = side->sounit->standing->orders[side->soutype]) != NULL) 
    copy_orders(side->tmporder, oldorder);
  else {
    side->tmporder->type = AWAKE;
    side->tmporder->rept = 0;
  }
  side->tmporder->morder = TRUE;
  finish_teach(side);
}

/* Switch from standing order teaching mode back to normal, and confirm */
/* that the standing order has been set. */

finish_teach(side)
Side *side;
{
    Unit *occ, *next;

    side->teach = FALSE;
    if (side->sounit->standing->orders[side->soutype] != NULL)
      free(side->sounit->standing->orders[side->soutype]);
    side->sounit->standing->orders[side->soutype] = side->tmporder;
    if (side->tmporder==NULL) {
      notify(side, "standing orders for %s deleted",
	     utypes[side->soutype].name);
    } else {
      notify(side, "%s has orders for %s to %s%0s.",
	     unit_handle(side, side->sounit), utypes[side->soutype].name,
	     order_desig(side->tmporder),
	     ((side->tmporder->morder) ? " and auto" : ""));
      for_all_occupants(side->sounit, occ) {
	if (occ->type == side->soutype) {
	  get_standing_orders(occ, side->sounit);
	}
      }
    }
    show_timemode(side,TRUE);
}

/* Display orders in some coherent fashion.  Use the information about the */
/* types of order parameters to decide how to display them. */

char *
order_desig(orders)
Order *orders;
{
    switch (orderargs[orders->type]) {
    case NOARG:
	sprintf(oargbuf, "");
	break;
    case DIR:
	sprintf(oargbuf, "%s ", dirnames[orders->p.dir]);
	break;
    case POS:
	sprintf(oargbuf, "%d,%d ", orders->p.pt[0].x, orders->p.pt[0].y);
	break;
    case EDGE_A:
	sprintf(oargbuf, "%d/%d ", orders->p.edge.forward, orders->p.edge.ccw);
	break;
    case LEADER:
	sprintf(oargbuf, "%s ", unit_handle((Side *) NULL,
					    find_unit(orders->p.leader_id)));
	break;
    case WAYPOINTS:
	sprintf(oargbuf, "%d,%d %d,%d ",
		orders->p.pt[0].x, orders->p.pt[0].y,
		orders->p.pt[1].x, orders->p.pt[1].y);
	break;
    default:
	case_panic("order arg type", orderargs[orders->type]);
    }
    if (orders->rept > 1)
	sprintf(orderbuf, "%s %s(%d)",
		ordernames[orders->type], oargbuf, orders->rept);
    else
	sprintf(orderbuf, "%s %s",
		ordernames[orders->type], oargbuf);
    return orderbuf;
}

/* Yeah yeah, assignment statements supposedly copy structures. */

copy_orders(dst, src)
Order *dst, *src;
{
  /* the first two options should work fine since the Order
     is (theoretically) contiguous with no pointers. */
#ifdef BCOPY
  bcopy(src, dst, sizeof(*dst));
#else
  memcpy(dst, src, sizeof(*dst));
#endif
#if 0  /* fallback code */
    dst->type = src->type;
    dst->rept = src->rept;
    dst->flags = src->flags;
    switch (orderargs[src->type]) {
    case NOARG:
	break;
    case DIR:
	dst->p.dir = src->p.dir;
	break;
    case POS:
	dst->p.pt[0].x = src->p.pt[0].x;
	dst->p.pt[0].y = src->p.pt[0].y;
	break;
    case EDGE_A:
	dst->p.edge.forward = src->p.edge.forward;
	dst->p.edge.ccw     = src->p.edge.ccw;
	break;
    case LEADER:
	dst->p.leader_id = src->p.leader_id;
	break;
    case WAYPOINTS:	
	dst->p.pt[0].x = src->p.pt[0].x;
	dst->p.pt[0].y = src->p.pt[0].y;
	dst->p.pt[1].x = src->p.pt[1].x;
	dst->p.pt[1].y = src->p.pt[1].y;
	break;
    default:
	case_panic("order arg type", orderargs[src->type]);
    }
    dst->morder = src->morder;
#endif /* 0 */
}
