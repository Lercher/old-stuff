/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file contains almost all command functions. */
/* Help commands are in a separate file. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "map.h"
#include "global.h"
#include "mplay.h"

char *grok_string();

bool reconfig;          /* true when an option needs screen reconfigured */
bool remap;             /* true when only main map needs redrawing */
bool reinfo;            /* true when only info display needs redrawing */
bool tmparea = TRUE;    /* true for area painting, false for bar painting */

/* Things will be totally scrogged if two human players in build mode... */

int tmpterr = 0;	/* temporary terrain type for area operation */
int tmpdist = 0;        /* temporary argument for painting */
int tmpflag;		/* temporary int for area operation */

/* Move in given direction a given distance - used for both single and */
/* automatic multiple moves. */

do_dir(side, dir, n)
Side *side;
int dir, n;
{

  enter_procedure("do_dir");
    if (side->teach) {
	cache_movedir(side, dir, n);
    } else {
	switch (side->mode) {
	case MOVE:
	    if (side->curunit != NULL) order_movedir(side->curunit, dir, n);
	    break;
	case SURVEY:
	    move_survey(side,
			wrap(side->curx + n*dirx[dir]),
			side->cury + n*diry[dir]);
	    break;
	default:
	    case_panic("mode", side->mode);
	}
    }
  exit_procedure();
}

/* Wake *everything* (that's ours) within the given radius.  Two commands */
/* actually; lowercase is transports only, uppercase is everybody. */

do_wakeup(side, n)
Side *side;
int n;
{

  enter_procedure("do_wakeup");
    if (side->teach) {
	cache_awake(side);
    } else {
	wakeup_area(side, n, TRUE);
    }
  exit_procedure();
}

/* Wake up only the main/current unit in a hex. */

do_wakemain(side, n)
Side *side;
int n;
{

  enter_procedure("do_wakemain");
    if (side->teach) {
	cache_awake(side);
    } else {
        if (side->curunit != NULL)
	  wake_unit(side->curunit, FALSE, WAKEFULL, (Unit *) NULL);
	wakeup_area(side, n, FALSE);
    }
  exit_procedure();
}

/* The area wakeup. */

wake_at(x, y)
int x, y;
{
    Unit *unit = unit_at(x, y);

    enter_procedure("wake_at");
    if (unit != NULL && (unit->side == tmpside || Build))
	wake_unit(unit, tmpflag, WAKEFULL, (Unit *) NULL);
    exit_procedure();
}

wakeup_area(side, n, occs)
Side *side;
int n, occs;
{
    tmpside = side;
    tmpflag = occs;
    apply_to_area(side->curx, side->cury, n, wake_at);
}

/* Put unit to sleep for a while.  If we sleep it next to something that */
/* might wake it up, then adjust flags so it won't wake up on next turn. */

do_sentry(side, unit, n)
Side *side;
Unit *unit;
int n;
{
  enter_procedure("do_sentry");
    if (side->teach) {
	cache_sentry(side, n);
    } else {
	order_sentry(unit, n);
	if (n > 1 && adj_enemy(unit))
	    unit->orders.flags &= ~(ENEMYWAKE|NEUTRALWAKE);
    }
  exit_procedure();
}

/* Don't move for remainder of turn, but be awake next turn.  This also */
/* hooks into terrain painting, since the space bar is big and convenient. */

do_sit(side, n)
Side *side;
int n;
{
    if (side->mode == SURVEY && Build) {
	paint_terrain(side);
    } else if (side->curunit != NULL) {
	do_sentry(side, side->curunit, 1);
    } else {
	cmd_error(side, "No unit to operate on here!");
    }
}

/* Set unit to move to a given location.  */

x_moveto(side)
Side *side;
{

  enter_procedure("x_moveto");
    if (grok_position(side)) {
	if (side->teach) {
	    cache_moveto(side, side->reqposx, side->reqposy);
	} else if (Build) {
	    leave_hex(side->requnit);
	    occupy_hex(side->requnit, side->reqposx, side->reqposy);
	    make_current(side, side->requnit);
	    all_see_hex(side->curx, side->cury);
	} else {
	  order_moveto(side->requnit, side->reqposx, side->reqposy);
	}
	restore_cur(side);
    } else {
	request_input(side, side->requnit, x_moveto);
    }
  exit_procedure();
}

/* The command proper. */

do_moveto(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    ask_position(side, "Move to where?");
    request_input(side, unit, x_moveto);
    side->reqposx = side->curx;  side->reqposy = side->cury;
}

/* order a unit to move toward another unit */

x_movetounit(side)
Side *side;
{
  int	n = side->reqvalue2;
  side->reqvalue2 = FALSE;
  if (grok_bool(side)) {
    if (side->teach) {
      cache_movetounit(side, side->markunit, n);
    } else {
      order_movetounit(side->tmpcurunit, side->markunit, n);
    }
  }
}

do_movetounit(side, unit, n)
Side *side;
Unit *unit;
int n;
{
  static char buf[200];
  if (side->markunit==NULL) {
    cmd_error(side, "No marked unit.  Use 'x' to mark the destination");
    return;
  } else {
    sprintf(buf, "Move toward %s?", unit_handle(side, side->markunit));
  }
  move_survey(side, side->markunit->x, side->markunit->y);
  side->tmpcurunit = unit;
  ask_bool(side, buf, FALSE/* we override this in the next statement */);
  side->reqvalue2 = n;
  request_input(side, unit, x_movetounit);
}

/* Determine how far away another point is.  */

x_distance(side)
Side *side;
{

  enter_procedure("x_distance");
    if (grok_position(side)) {
      restore_cur(side);
      notify(side, "Distance from (%d, %d) to (%d, %d) is %d",
	     side->curx, side->cury, side->reqposx, side->reqposy,
	     distance(side->curx, side->cury, side->reqposx, side->reqposy));
    } else {
	request_input(side, side->requnit, x_distance);
    }
  exit_procedure();
}

/* The command proper. */

do_distance(side, n)
Side *side;
int n;
{
    ask_position(side, "Distance to where?");
    request_input(side, (Unit *) NULL, x_distance);
    side->reqposx = side->curx;  side->reqposy = side->cury;
}

/* Search for a friendly refueler within range and set course for it.  */
/* Warn player and refuse to move if nothing close enough. */

do_return(side, unit, n)
Side *side;
Unit *unit;
int n;
{
#if 1
  if (side->teach) {
    cache_return(side, n);
  } else {
    order_return(unit, n);
    unit->orders.flags &= ~(SUPPLYWAKE|ENEMYWAKE|NEUTRALWAKE);
  }
#else
    int x, y, u = unit->type, r, range = max(world.width, world.height);

    enter_procedure("do_return");
    for_all_resource_types(r) {
	if (utypes[u].tomove[r] > 0) {
	    range = min(range, unit->supply[r] / utypes[u].tomove[r]);
	}
    }
    tmpside = side;
    tmpunit = unit;
    if (refuel_here(unit->x, unit->y)) {
      cmd_error(side, "Already at a resupply point!");
    }
    else if (search_area(unit->x, unit->y, range, refuel_here, &x, &y, 1)) {
	    order_moveto(unit, x, y);
	    unit->orders.flags = SHORTESTPATH;
	  } else {
	    cmd_error(side, "No resupply point in range!");
	  }
    exit_procedure();
#endif
}

/* Set unit to attempt to follow a coast.  Needs a starting direction, */
/* which can be computed from a position. */

x_coast(side)
Side *side;
{
    int dir;

    enter_procedure("x_coast");
    if (grok_position(side)) {
      int	newx,newy, curx,cury;
      newx = side->reqposx;
      newy = side->reqposy;
      curx = side->requnit->x;
      cury = side->requnit->y;
      if (curx != newx || cury != newy) {
	int	ccw = 0, count=0;

	dir = find_dir(newx - curx, newy - cury);
	if (!could_move_in_dir(side->requnit, dir)) {
	  cmd_error(side, "%s can't move there!",
		    unit_handle(side,side->requnit));
	} else {
	  if (could_move_in_dir(side->requnit, left_of(dir))) {	  
	    ccw++; count++;
	  }
	  if (could_move_in_dir(side->requnit, right_of(dir))) {
	    ccw--; count++;
	  }
	  if (count>0 && ccw==0) {
	    cmd_error(side, "Those hexes don't share an edge!");
	  } else {
	    notify(side, "%s will follow %s%s",
		   unit_handle(side,side->requnit),
		   (ccw==0)?"isthsmus":"edge ",
		   (ccw==0)?"":(ccw>0)?"right":"left");
	    
	    if (side->teach) {
	      cache_edge(side, dir, ccw, side->reqvalue2);
	    } else {
	      order_edge(side->requnit, dir, ccw, side->reqvalue2);
	    }
	  }
	}
	restore_cur(side);
      } else {
	cmd_error(side, "No particular direction at own hex??");
      }
    } else {
      request_input(side, side->requnit, x_coast);
    }
    exit_procedure();
}

/* The command proper just sets up the interaction. */

do_coast(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    ask_position(side, "Move in which direction?");
    request_input(side, unit, x_coast);
    side->reqposx = side->curx;  side->reqposy = side->cury;
    side->reqvalue2 = n;
}

/* find the nearest filling transport and order unit towards it */

do_movetotransport(side, unit, n)
     Side	*side;
     Unit	*unit;
     int	n;
{
  enter_procedure("do_movetotransport");
  
  if (side->teach) {
    cache_movetotransport(side, n);
  } else {
    order_movetotransport(unit, n);
  }

  exit_procedure();
}

/* Set orders to follow a leader unit. */

x_follow(side)
Side *side;
{
    Unit *leader;

    enter_procedure("x_follow");
    if (grok_position(side)) {
	if ((leader = unit_at(side->reqposx, side->reqposy)) != NULL &&
	    leader->side == side) {
	    if (leader != side->requnit) {
		if (side->teach) {
		    cache_follow(side, leader, side->reqvalue2);
		} else {
		    order_follow(side->requnit, leader, side->reqvalue2);
		}
	    } else {
		cmd_error(side, "Unit can't follow itself!");
	    }
	} else {
	    cmd_error(side, "No unit to follow!");
	}
	restore_cur(side);
    } else {
	request_input(side, side->requnit, x_follow);
    }
    exit_procedure();
}

/* The command proper. */

do_follow(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    ask_position(side, "Which unit to follow?");
    request_input(side, unit, x_follow); 
    side->reqposx = side->curx;  side->reqposy = side->cury;
    side->reqvalue2 = n;
}

/* Patrolling goes back and forth between two points.  First point is the */
/* current position. */

x_patrol(side)
Side *side;
{

  enter_procedure("x_patrol");
    if (grok_position(side)) {
	if (side->teach) {
	    cache_patrol(side, side->sounit->x, side->sounit->y,
			 side->reqposx, side->reqposy, side->reqvalue2);
	} else {
	    order_patrol(side->requnit, side->requnit->x, side->requnit->y,
			 side->reqposx, side->reqposy, side->reqvalue2);
	}
	restore_cur(side);
    } else {
	request_input(side, side->requnit, x_patrol);
    }
  exit_procedure();
}

/* The command proper. */

do_patrol(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    ask_position(side, "What other endpoint for patrol?");
    request_input(side, unit, x_patrol);
    side->reqposx = side->curx;  side->reqposy = side->cury;
    side->reqvalue2 = n;
}

/* Delay a unit's move until a later time.  The set flag will be recognized */
/* by the movement loops, when deciding which unit to move next. */

do_delay(side, unit, n)
Side *side;
Unit *unit;
int n;
{
  if (side->mode == MOVE) {
    delete_unit(unit);
    insert_unit_tail(side->unithead, unit);
    unit->orders.type = DELAY;
    side->movunit = NULL;
    notify(side, "Delaying moving %s.", unit_handle(side, unit));
  }
}

/* Get rid of unit.  Some units cannot be disbanded, but if they can, the */
/* resources go to a transport if one is there.  Disbanding a transport */
/* also disbands all the occupants - oh well. */

do_disband(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    int u = unit->type;

    enter_procedure("do_disband");
    if (utypes[u].disband || Build) {
	notify(side, "%s goes home.", unit_handle(side, unit));
	if (unit->transport != NULL) recycle_unit(unit, unit->transport);
	kill_unit(unit, DISBAND);
	make_current(side, unit_at(side->curx, side->cury));
    } else {
	cmd_error(side, "You can't just get rid of the %s!", utypes[u].name);
    }
    exit_procedure();
}

/* Reclaim both the unit's supplies and anything used in its making, but */
/* only let a maker of the unit reclaim its ingredients. */

recycle_unit(unit, unit2)
Unit *unit, *unit2;
{
    int u = unit->type, u2 = unit2->type, r, scrap;

    for_all_resource_types(r) {
	transfer_supply(unit, unit2, r, unit->supply[r]);
	if (could_make(u2, u) > 0) {
	    scrap = utypes[u].tomake[r];
	    unit2->supply[r] += (scrap * period.efficiency) / 100;
	}
    }
}

/* Give a unit to another side (possibly to neutrals).  Units that won't */
/* change their sides when captured won't change voluntarily either. */

do_give_unit(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    int u = unit->type;

    if (utypes[u].changeside || Build) {
	unit_changes_side(unit, side_n(n), CAPTURE, GIFT);
	all_see_hex(unit->x, unit->y);
	if (global.setproduct) {
	  set_product(unit, NOTHING);
	  unit->schedule = 0;
	} 
   } else {
	cmd_error(side, "You can't just give away the %s!", utypes[u].name);
    }
}

/* Marking is for the purpose of rearranging units within a hex. */

do_mark_unit(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    side->markunit = unit;
    notify(side, "%s has been marked.", unit_handle(side, unit));
}

/* This is a clever (if I do say so myself) command to examine all occupants */
/* and suboccupants, in a preorder fashion. */

do_occupant(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    Unit *nextup;

    switch (side->mode) {
    case MOVE:
	cmd_error(side, "Can only look at occupants when in survey mode!");
	break;
    case SURVEY:
	if (unit->occupant != NULL) {
	    make_current(side, unit->occupant);
	} else if (unit->nexthere != NULL) {
	    make_current(side, unit->nexthere);
	} else {
	    nextup = unit->transport;
	    if (nextup != NULL) {
		while (nextup->transport != NULL && nextup->nexthere == NULL) {
		    nextup = nextup->transport;
		}
		if (nextup->nexthere != NULL)
		    make_current(side, nextup->nexthere);
		if (nextup->transport == NULL)
		    make_current(side, nextup);
	    }
	}
	break;
    default:
	case_panic("mode", side->mode);
	break;
    }
}

/* This can actually do general rearrangement, but defaults to putting the */
/* unit on the first available transport in the hex.  */
/* What about trying to embark a unit on itself or on its previous transp? */

do_embark(side, unit, n)
Side *side;
Unit *unit;
int n;
{
  Unit *mainunit = unit_at(unit->x, unit->y);
  Unit *transport = NULL;
  
  enter_procedure("do_embark");
  if (side->teach) {
    cache_embark(side, n);
  } else {
    if (mainunit != unit) {
      if (side->markunit == NULL ||
	  side->markunit->x != unit->x || side->markunit->y != unit->y) {
	for_all_occupants(mainunit, transport) {
	  if (can_carry(transport, unit)) break;
	}
      } else if (can_carry(side->markunit, unit)) { /* thx peterw@reed */
	transport = side->markunit;
      }
      if (transport != NULL) {
	if (can_carry(transport, unit)) {
	  leave_hex(unit);
	  occupy_unit(unit, transport);
	  if (n > 0) order_sentry(unit, n);
	  routine(" done embark");
	  
	  exit_procedure();
	  return TRUE;
	}
	else {
	  if (n > 0) order_embark(unit, n);
	  else cmd_error(side, "Marked unit already full.");
	}
      } else {
	if (n > 0) order_embark(unit, n);
	else cmd_error(side, "No plausible transport!");
      }
    } else {
      cmd_error(side, "Nothing for this unit to get into!");
    }
    exit_procedure();
    return FALSE;
  }
}

do_fill(side, unit, n)
     Side *side;
     Unit *unit;
     int n;
{
  enter_procedure("do_fill");
  if (side->teach)
    cache_fill(side,n);
  else {
    order_fill(unit, n);
    wake_if_full(unit);
  }
  exit_procedure();
}

/* Give supplies to a transport.  The argument tells how many to give. */

do_give(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    bool something = FALSE;
    int u = unit->type, m, r, gift, actual;
    Unit *main = unit->transport;

    if (main != NULL) {
	sprintf(spbuf, "");
	m = main->type;
	for_all_resource_types(r) {
	    gift = (n < 0 ? utypes[m].storage[r] - main->supply[r] : n);
	    if (gift > 0) {
		something = TRUE;
		/* Be stingy if we're low */
		if (2 * unit->supply[r] < utypes[u].storage[r])
		    gift = max(1, gift/2);
		actual = transfer_supply(unit, main, r, gift);
		sprintf(tmpbuf, " %d %s", actual, rtypes[r].name);
		strcat(spbuf, tmpbuf);
	    }
	}
	if (something) {
	    notify(side, "%s gave%s.", unit_handle(side, unit), spbuf);
	} else {
	    notify(side, "%s gave nothing.", unit_handle(side, unit));
	}
    } else {
	cmd_error(side, "Can't transfer supplies here!");
    }
}

/* Take supplies from transport.  Both the transport must have something */
/* left. */

do_take(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    bool something = FALSE;
    int u = unit->type, m, r, need, actual;
    Unit *main = unit->transport;

    if (main != NULL) {
	sprintf(spbuf, "");
	m = main->type;
	for_all_resource_types(r) {
	    need = (n < 0 ? utypes[u].storage[r] - unit->supply[r] : n);
	    if (need > 0) {
		something = TRUE;
		/* Be stingy if we're low */
		if (2 * main->supply[r] < utypes[m].storage[r])
		    need = max(1, need/2);
		actual = transfer_supply(main, unit, r, need);
		sprintf(tmpbuf, " %d %s", actual, rtypes[r].name);
		strcat(spbuf, tmpbuf);
	    }
	}
	if (something) {
	    notify(side, "%s got%s.", unit_handle(side, unit), spbuf);
	} else {
	    notify(side, "%s needed nothing.", unit_handle(side, unit));
	}
    } else {
	cmd_error(side, "Can't transfer supplies here!");
    }
}

/* Take the current player out of the game while letting everybody else */
/* continue on. */

x_resign(side)
Side *side;
{
    if (grok_bool(side)) resign_game(side, side->reqoside);
}

do_resign(side, n)
Side *side;
int n;
{
  Side	*other;
  other = side->reqoside = side_n(n);
  if (other == side || other->lost) {
    cmd_error(side, "You have to give units to an active side.");
    return;
  }
  ask_bool(side, "Do you really want to give up?", FALSE);
  request_input(side, (Unit *) NULL, x_resign);
}

/* Unconditional resignation - usable by everybody. */

resign_game(side, side2)
Side *side, *side2;
{
    notify_all("Those wimpy %s have given up!", plural_form(side->name));
    if (side2 != NULL) {
	notify_all("... and they gave all their stuff to the %s!",
		   plural_form(side2->name));
    }
    side_loses(side, side2);
}

/* Leave quickly when the boss walks by.  One person can kill a multi-player */
/* game, which isn't too great, but the alternatives are complicated. */
/* The stats file will be left behind, to foment argument about who would */
/* have won... This routine also includes a trapdoor for freezing/unfreezing */
/* machine players when building - mode display will invert to confirm this. */

x_exit(side)
Side *side;
{
    Unit *unit;

    if (grok_bool(side)) {
	close_displays();
	printf("\nThe outcome remains undecided");
	if (numhumans == 1 && side->humanp) {
	    printf(", but you're probably the loser!\n\n", side->host);
	} else {
	    printf("...\n\n");
	}
	/* Need to kill off all units to finish up statistics */
	for_all_units(side,unit) if (alive(unit)) kill_unit(unit, ENDOFWORLD);
	print_statistics();
	exit(0);
    }
    if (Build) {
	Freeze = !Freeze;
	show_timemode(side,TRUE);
    }
}

do_exit(side, n)
Side *side;
int n;
{
    ask_bool(side, "Do you REALLY want to end the game for EVERYBODY?",
	     FALSE);
    request_input(side, (Unit *) NULL, x_exit);
}

/* Stuff game state into a file.  By default, it goes into the current */
/* directory.  If building a scenario, will ask about each section, values */
/* of globals, and dest file before actually writing anything. */
/* No capability to write out period at present... */

x_save_1(side)
Side *side;
{
    char *sects, *fname;
    int sdetail = 1, udetail = 1;

    if ((sects = grok_string(side)) != NULL) {
	fname = "random.scn";
	sprintf(spbuf, "------");
	if (iindex('v', sects) >= 0) spbuf[0] = '+';
	if (iindex('p', sects) >= 0) spbuf[1] = '+';
	if (iindex('m', sects) >= 0) spbuf[2] = '+';
	if (iindex('g', sects) >= 0) spbuf[3] = '+';
	if (iindex('s', sects) >= 0) spbuf[4] = '+';
	if (iindex('u', sects) >= 0) spbuf[5] = '+';
	if (iindex('s', sects) >= 0) {
	    sdetail = 1;
	    if (isdigit(sects[iindex('s', sects)+1]))
		sdetail = sects[iindex('s', sects)+1] - '0';
	}
	if (iindex('u', sects) >= 0) {
	    udetail = 1;
	    if (isdigit(sects[iindex('u', sects)+1]))
		udetail = sects[iindex('u', sects)+1] - '0';
	}
	notify(side, "Mapfile with sections %s will be saved to \"%s\" ...",
	       spbuf, fname);
	if (write_scenario(fname, spbuf, sdetail, udetail)) {
	    notify(side, "Done writing to \"%s\".", fname);
	} else {
	    cmd_error(side, "Can't open file \"%s\"!", fname);
	}
    } else {
	request_input(side, (Unit *) NULL, x_save_1);
    }
}

/* Make a header appropriate to a save file, write the file, and leave. */

x_save_2(side)
Side *side;
{
    if (grok_bool(side)) {
	notify_all("Game will be saved to \"%s\" ...", SAVEFILE);
	if (write_savefile(SAVEFILE)) {
	    close_displays();
	    exit(0);
	} else {
	    cmd_error(side, "Can't open file \"%s\"!", SAVEFILE);
	}
    }
}

/* The command proper just sets up different handlers, depending on */
/* whether we're building (and therefore saving a scenario/fragment), or */
/* saving as much game state as possible, for resumption later. */

do_save(side, n)
Side *side;
int n;
{
    if (Build) {
	ask_string(side, "Sections to write?", "ms1u1");
	request_input(side, (Unit *) NULL, x_save_1);
    } else {
	ask_bool(side, "You really want to save?", FALSE);
	request_input(side, (Unit *) NULL, x_save_2);
    }
}

/* Redraw everything using the same code as when windows need a redraw. */

do_redraw(side, n)
Side *side;
int n;
{
  int x, y, ts;

#ifdef PREVVIEW
    if (n > 0) {  /* remove the views of units likely to have moved. */
      for_all_hexes(x, y)
	  if (side_view_age(side, x, y) > n) {
	    int viewunit = vtype(side_view(side, x, y));

	    if (viewunit != EMPTY && viewunit != UNSEEN
		&& mobile(viewunit)) { 
	      ts = side_view_timestamp(side, x, y);
	      set_side_view(side, x, y, EMPTY);
	      side_view_timestamp(side, x, y) = ts;
	    }
	  }
    }
#endif
    redraw(side);
}

/* Flicker on the current position, in case it's not easily visible. */

do_flash(side, n)
Side *side;
int n;
{
    int sx, sy;

    xform(side, unwrap(side, side->curx, side->cury), side->cury, &sx, &sy);
    flash_position(side, sx, sy, 1000);
}

/* Name or rename the current unit or a given side.  We make a copy of the */
/* string after it's been successfully read, just in case. */

x_name(side)
Side *side;
{
    char *name;
    Side *side2;

    if ((name = grok_string(side)) == NULL) {
	request_input(side, side->requnit, x_name);
    } else if (strlen(name) == 0) {
	notify(side, "Name not changed.");
    } else if (side->requnit != NULL) {
	side->requnit->name = copy_string(name);
    } else if (side->reqoside != NULL) {
	side->reqoside->name = copy_string(name);
	for_all_sides(side2) show_all_sides(side2,TRUE);
    } else {
	cmd_error(side, "Nothing to name!");
    }
}

/* The command proper decides between unit and side naming. */

do_name(side, n)
Side *side;
int n;
{
    if (side->curunit != NULL) {
	ask_string(side, "New name for unit:", side->curunit->name);
    } else {
	ask_string(side, "New name for yourself:", side->name);
	side->reqoside = side;
    }
    request_input(side, side->curunit, x_name);
}

/* Designate the current location as the center of action and sort all */
/* of our own units relative to it. */

do_center(side, n)
Side *side;
int n;
{
    side->cx = side->curx;  side->cy = side->cury;
    sort_units(TRUE);
    set_side_movunit(side);
    notify(side, "Units reordered.");
}

/* Center the screen on the current location. */

do_recenter(side, n)
Side *side;
int n;
{

    recenter(side, side->curx,side->cury);
    if (Debug) notify(side, "Current location %d %d ",  side->curx,side->cury);
}

/* Hook command to set miscellaneous options.  Can't do from command line */
/* because each display may want different behavior.  This routine can */
/* change the display dramatically, but it should only redraw if a change */
/* has actually been made. */

/* Conversion to machine player is irreversible, so we confirm it first. */

x_options_2(side)
Side *side;
{
  Unit *unit;

    if (grok_bool(side)) {
	side->humanp = !side->humanp;
	for_all_side_units(side, unit) 
	  unit->area = area_index(unit->x, unit->y);
	numhumans--;
	init_sighandlers();
    }
}

x_options(side)
Side *side;
{
    int n = side->reqvalue2;
    char opt;

    if ((opt = grok_char(side)) != '\0') {
	switch (opt) {
	case '?':
	    notify(side, "Change (D)isplay Mode, (G)raph, Win (H)eight,");
	    notify(side, "(I)nverse Video, (2)-color, (N)otice Buffer");
	    notify(side, "(R)obot, Start (B)eep Time, Win (W)idth,");
	    notify(side, "World Map (M)agnification,");
	    break;
	case '2':
	    reconfig = TRUE;
	    side->monochrome = !side->monochrome;
	    side->bonw = FALSE;
	    break;
	case 'b':
	    side->startbeeptime = n;
	    break;
	case 'd':
	    remap = TRUE;
	    side->showmode = (side->showmode + 1) % 4;
	    break;
	case 'g':
	    reinfo = TRUE;
	    side->graphical = !side->graphical;
	    break;
	case 'h':
	    if (n < 5 || n > world.height) {
		cmd_error(side, "Bad height %d!", n);
	    } else {
		if (n != side->vh) reconfig = TRUE;
		side->vh = n;
	    }
	    break;
	case 'i':
	    if (side->monochrome) {
		reconfig = TRUE;
		side->bonw = !side->bonw;
	    } else {
		cmd_error(side, "Inverse video is only for monochrome!");
	    }
	    break;
	case 'm':
	    if (n<1 || n>5) {
	      cmd_error(side, "Bad magnification %d", n);
	    } else if (n!=side->mm) {
	      side->mm = n;
	      /* we don't reconfig=TRUE because map magnification
		 is normally a function of other things outside
		 easy control. */
	      if (active_display(side))
		resize_display2(side, side->mw, side->mh);
	      reconfigure_display(side,FALSE);
	    }
	    break;
	case 'n':
	    if (n < 1 || n > MAXNOTES) {
		cmd_error(side, "Bad number of notes %d!", n);
	    } else {
		if (n != side->nh) reconfig = TRUE;
		side->nh = n;
	    }
	    break;
	case 'r':	
	    if (side->mode == MOVE) {
		ask_bool(side,
			 "Do you really want to become a machine?", FALSE);
		request_input(side, (Unit *) NULL, x_options_2);
		return;
	    } else {
		cmd_error(side, "Must be in move mode!");
	    }
	    break;
	case 'w':
	    if (n < 5 || n > world.width || n > BUFSIZE) {
		cmd_error(side, "Bad width %d!", n);
	    } else {
		if (n != side->vw) reconfig = TRUE;
		side->vw = n;
	    }
	    break;
	default:
	    cmd_error(side, "unrecognized option '%c'", opt);
	    break;
	}
	if (remap) {
	    /* undraw_box(side); */
	    show_map(side);
	}
	if (reinfo) show_info(side);
	if (reconfig) {
	  if (active_display(side))
	    set_sizes(side);
	  reconfigure_display(side,TRUE);
	}
    } else {
	request_input(side, (Unit *) NULL, x_options);
    }
}

/* The command proper. */

do_options(side, n)
Side *side;
int n;
{
    reinfo = remap = reconfig = FALSE;
    ask_char(side, "Options:", "2bdghimnrw");
    request_input(side, (Unit *) NULL, x_options);
    side->reqvalue2 = n;
}

/* Set standing orders for a unit of a given type that enters a given city. */
/* Space for standing orders is dynamically allocated the first time we */
/* request some orders. */

x_standing_1(side)
Side *side;
{
    int type;

    if ((type = grok_unit_type(side)) >= 0) {
	if (type != NOTHING) get_standing_order(side, type);
    } else {
	request_input(side, side->requnit, x_standing_1);
    } 
}

/* The command proper starts the ball rolling by prompting for the type */
/* of unit that will get standing orders.  Of course, if a unit is not of */
/* a type that has occupants, standing orders are pretty useless.  Also, */
/* if only one type of occupant is possible, then no need to ask. */

do_standing(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    int u;

    
    if (Debug) printf("doing standing order\n");
    u = ask_unit_type(side, "Type of occupant to get standing orders",
		      utypes[unit->type].capacity);
    if (Debug) printf("u is %d\n", u);
    if (u < 0) {
	show_standing_orders(side, unit);
	if (Debug) printf("Input being requested\n", u);
	request_input(side, unit, x_standing_1);
	side->sounit = unit;
    } else if (u == NOTHING) {
	cmd_error(side, "This unit never has occupants to give orders to!");
    } else {
	show_standing_orders(side, unit);
	get_standing_order(side, u);
    }
}

/* A standing order is acquired by snarfing the next order and saving it */
/* rather than applying it to some unit. */

get_standing_order(side, type)
Side *side;
int type;
{
  int i;
  
    if (side->requnit->standing == NULL) {
	side->requnit->standing =
	    (StandingOrder *) malloc(sizeof(StandingOrder));
	for_all_unit_types(i)
	  side->requnit->standing->orders[i] = NULL;
    }
    side->teach = TRUE;
    side->soutype = type;
    side->tmporder = (Order *) malloc(sizeof(Order));
    side->tmporder->morder = FALSE;
    side->tmporder->flags = NORMAL;
    
    notify(side, "Next input order will become the standing order (DEL to delete).");
    show_timemode(side,TRUE);
    request_command(side);
}

/* this command does nothing normally, but in teach mode it deletes the
   standing order */

do_nothing(side, n)
     Side	*side;
     int	n;
{
  if (side->teach) {
    free(side->tmporder);
    side->tmporder=NULL;
    finish_teach(side);
  }
}

/* Survey mode allows player to look around (and change things) by moving */
/* cursor.  The same command toggles in and out, so need a case statement. */
/* Players waiting their turn will be in survey mode, but can't get out. */

do_survey_mode(side, n)
Side *side;
int n;
{
    flush_input(side); /* don't want commands from one mode in other */
    switch (side->mode) {
    case MOVE:
	survey_mode(side);
	break;
    case SURVEY:
	if (can_move_unit(side,side->curunit))
	  side->movunit = side->curunit;
	move_mode(side);
	break;
    default:
	case_panic("mode", side->mode);
    }
}

/* Change what a unit is producing. */

do_product(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    if (!global.setproduct) {
	cmd_error(side, "No construction changes allowed in this game!");
    } else {
	if (!can_produce(unit)) {
	    cmd_error(side, "This unit can't build anything!");
	} else if (utypes[unit->type].occproduce || (unit->transport == NULL)){
	    if (!utypes[unit->type].maker) 
		wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
	    request_new_product(unit);
	  }
          else cmd_error(side, "This unable to produce inside other units.");
    }
}
/* Setting what a unit will produce next. */

do_next_product(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    if (!global.setproduct) {
	cmd_error(side, "No construction changes allowed in this game!");
    } else {
	if (!can_produce(unit)) {
	    cmd_error(side, "This unit can't build anything!");
	} else if (utypes[unit->type].occproduce || (unit->transport == NULL)){
	    if (!utypes[unit->type].maker) 
		wake_unit(unit, FALSE, WAKEOWNER, (Unit *) NULL);
	    request_next_product(unit);
	  }
          else cmd_error(side, "This unable to produce inside other units.");
    }
}

/* Set a unit to not produce anything (yes, this really is useful). */

do_idle(side, unit, n)
Side *side;
Unit *unit;
int n;
{
    if (!global.setproduct) {
	cmd_error(side, "No production changes allowed in this scenario!");
    } else {
	set_product(unit, NOTHING);
	unit->schedule = n;
	show_info(side);
    }
}

/* Send a short (1 line) message to another player.  Some messages are */
/* recognized specially, causing various actions. */

x_message(side)
Side *side;
{
    char *msg;
    Side *side3;

    if ((msg = grok_string(side)) != NULL) {
	if (side->reqoside == NULL) {
	    if (msg != NULL && strlen(msg) > 0) {
		notify_all("The %s announce: %s",
			   plural_form(side->name), msg);
	    }
	} else if (strcmp(msg, "briefing") == 0) {
	    notify(side->reqoside, "Receiving a briefing from the %s...",
		   plural_form(side->name));
	    reveal_side(side, side->reqoside, 100);
	    notify(side, "You just briefed the %s on your position.",
		   plural_form(side->reqoside->name));
	} else if (strcmp(msg, "alliance") == 0) {
	    notify(side, "You propose a formal alliance with the %s.",
		   plural_form(side->reqoside->name));
	    side->attitude[side_number(side->reqoside)] = ALLY;
	    if (side->reqoside->attitude[side_number(side)] >= ALLY) {
		declare_alliance(side, side->reqoside);
		for_all_sides(side3) redraw(side3);
	    } else {
		notify(side->reqoside, "The %s propose a formal alliance.",
		       plural_form(side->name));
	    }
	} else if (strcmp(msg, "neutral") == 0) {
	    notify(side, "You propose neutrality with the %s.",
		   plural_form(side->reqoside->name));
	    side->attitude[side_number(side->reqoside)] = NEUTRAL;
	    if (side->reqoside->attitude[side_number(side)] == NEUTRAL) {
		declare_neutrality(side, side->reqoside);
		for_all_sides(side3) redraw(side3);
	    } else {
		notify(side->reqoside, "The %s propose neutrality.",
		       plural_form(side->name));
	    }
	} else if (strcmp(msg, "war") == 0) {
	    notify(side, "You declare war on the %s!",
		   plural_form(side->reqoside->name));
	    declare_war(side, side->reqoside);
	    for_all_sides(side3) redraw(side3);
	} else if (strlen(msg) > 0) {
	    notify(side->reqoside, "The %s say to you: %s",
		   plural_form(side->name), msg);
	} else {
	    notify(side, "You keep your mouth shut.");
	}
    } else {
	request_input(side, (Unit *) NULL, x_message);
    }
}

/* The command proper. */

do_message(side, n)
Side *side;
int n;
{
    char prompt[BUFSIZE];
    Side *side2;

    side2 = side_n(n);
    side->reqoside = side2;
    if (side != side2) {
	if (side2) {
	    sprintf(prompt, "Say to the %s: ", plural_form(side2->name));
	} else {
	    sprintf(prompt, "Broadcast: ");
	}
	ask_string(side, prompt, (char *) NULL);
	request_input(side, (Unit *) NULL, x_message);
    } else {
	cmd_error(side, "You mumble to yourself.");
    }
}


/* toggle followaction flag */

do_look(side, n)
Side *side;
int n;
{

  if (n == -1)
    side->followaction = !side->followaction;
  else side->followaction = n;
}

/* Add a new player to the game. */
/* Should use arg to decide whether to convert machine player (or just */
/* use if available?)  Also needs to decide if new player is human and */
/* which host to open, then go through side's startup seq and open disp. */
/* Issues of cached values (?) and war/alliance setup and uses of numsides. */

x_add_player(side)
     Side	*side;
{
  char	*dname;
  Side	*zombie;
  Unit	*unit;

  if ((dname = grok_string(side)) == NULL) {
    request_input(side, (Unit*) NULL, x_add_player);
    return;
  }

  zombie = side->reqoside;

  zombie->host = copy_string(dname);

  zombie->humanp = TRUE;
  zombie->lost = FALSE;
  zombie->markunit = NULL;
  init_display(zombie);
  unit = side->markunit;
  unit_changes_side(unit, zombie, CAPTURE, GIFT);
  zombie->curunit = zombie->movunit = zombie->last_unit = unit;
  zombie->cx = unit->x;
  zombie->cy = unit->y;
}

do_add_player(side, n)
Side *side;
int n;
{
  Side	*zombie;

  cmd_error(side, "Sorry, can't add new players yet!");
  for_all_sides(zombie) {
    if (!zombie->humanp && zombie->lost &&
	zombie->unithead->next == zombie->unithead)
      break;
  }
  if (zombie!=NULL) {
    notify(side, "side %d is candidate", side_number(zombie));
    side->reqoside = zombie;
    side->markunit = side->curunit;
    ask_string(side, "display: ", (char*) NULL);
    request_input(side, (Unit*) NULL, x_add_player);
  } else
    cmd_error(side, "no corpses yet, can't resurrect");
}

/* Command to display the program version.  Looks wired in, but of course */
/* this is not something that we want to be easily changeable! */
/* This will also show data about other sides. */

do_version(side, n)
Side *side;
int n;
{
  
   notify(side, " ");
   notify(side,
	  "XCONQ version %s", version);
   notify(side,
	  "(c) Copyright 1987, 1988, 1991  Stanley T. Shebs");
   notify(side, " ");
   if (Debug || Build)
     reveal_side((Side *) NULL, side, 100);

   /* Now check that a lot of numbers are ok for the units. */
   {
     Unit *unit;
     Side *loop_side;
     
     for_all_units(loop_side, unit) {
       if (unit->x < 0 || unit->x >= world.width ||
	   unit->y <= 0 || unit->y >= (world.height - 1) ||
	   unit->hp <= 0)
	 notify(side, "Unit off map %s id %d (%d, %d) hp %d",
		unit_handle((Side *) NULL, unit),
		unit->id, unit->x, unit->y, unit->hp);
     }
   }
}

/* Set the unit to automatic control.  */

do_auto(side, unit, n)
Side *side;
Unit *unit;
int n;
{
  enter_procedure("do_auto");
    if (side->teach) {
	cache_auto(side);
    } else {
        if (side->curunit != NULL)
	  unit->orders.morder = TRUE;
    }
  exit_procedure();
}


/* Create any unit anywhere.  It gets the usual initial supply, and its */
/* current side is also its true side (i.e. it will never revolt). */

x_unit(side)
Side *side;
{
    int u;
    Unit *unit;

    if ((u = grok_unit_type(side)) >= 0) {
	if (u != NOTHING) {
	    unit = create_unit(u, (char *) NULL);
	    occupy_hex(unit, side->curx, side->cury);
	    init_supply(unit);
	    unit_changes_side(unit, side->reqoside, -1, -1);
	    unit->trueside = unit->side;
	    make_current(side, unit);
	    all_see_hex(side->curx, side->cury);
	}
    } else {
	ask_unit_type(side, "Type of unit to create?", (short *) NULL);
	request_input(side, (Unit *) NULL, x_unit);
    }
}

/* The command function proper, which only works in Build mode. */

do_unit(side, n)
Side *side;
int n;
{
    if (Build) {
	if (unit_at(side->curx, side->cury) == NULL) {
	    ask_unit_type(side, "Type of unit to create?", (short *) NULL);
	    side->reqoside = side_n(n);
	    request_input(side, (Unit *) NULL, x_unit);
	} else {
	    cmd_error(side, "Unit already here!");
	}
    } else {
	cmd_error(side, "Not building a mapfile!");
    }
}

/* Terrain editing alters a hexagonal area of given radius.  If only one */
/* hex changed (the default), just update that alone; otherwise, go ahead */
/* and redraw everything. */

/* The command itself just sets up what will be drawn. */

do_terrain(side, terr, n)
Side *side;
int terr, n;
{
    tmpterr = terr;
    tmpdist = min(abs(n), world.width);
    tmparea = (n >= 0);
    notify(side, "Will now paint %d hex %s of %s.",
	   tmpdist, (tmparea ? "radius area" : "bars"), ttypes[tmpterr].name);
}

/* Function to change just one hex and to echo that change. */
/* Don't need to make it show instantly, can wait. */

set_one_hex(x, y)
int x, y;
{
    set_terrain_at(x, y, tmpterr);
    see_exact(tmpside, x, y);
    draw_hex(tmpside, x, y, FALSE);
}

/* Painting operation is activated by the "sit" command. */

paint_terrain(side)
Side *side;
{
    int i;

    tmpside = side;
    if (tmparea) {
	apply_to_area(side->curx, side->cury, tmpdist, set_one_hex);
    } else {
	for (i = 0; i < tmpdist; ++i)
	    set_one_hex(wrap(side->curx + i), side->cury);
    }
}

/* Generic command error routine - beeps display etc. */

/*VARARGS*/
cmd_error(side, control, a1, a2, a3, a4, a5, a6)
Side *side;
char *control, *a1, *a2, *a3, *a4, *a5, *a6;
{
    notify(side, control, a1, a2, a3, a4, a5, a6);
    if (active_display(side) && humanside(side) &&
	(side->curunit == NULL || !side->curunit->orders.morder))
      beep(side);
}
