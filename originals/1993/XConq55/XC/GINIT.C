/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Initialization and random routines for the display part of xconq. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "map.h"
#include "global.h"

/* Always use four lines for the unit info display, even on small screens. */

#define INFOLINES 5

int maxnamelen = 0;		/* length of longest side name+host */

/* Find a unit and put it at center (or close to it) of first view.  I guess */
/* can't use put_on_screen because not enough stuff is inited yet. */

init_curxy(side)
Side *side;
{
    Unit *unit;
    Side *loop_side;

    for_all_units(loop_side, unit) {
	if (unit->side == side) {
	    side->cx = unit->x;  side->cy = unit->y;
	    side->vcx = unit->x;
	    side->vcy = max(unit->y, side->vw2+1);
	    side->vcy = min(side->vcy, (world.height-1) - side->vh2);
	    return;
	}
    }
    side->vcx = side->vw2;  side->vcy = side->vh2;
    side->cx = side->vcx;  side->cy = side->vcy;
}

/* open displays early so that the first redraw will find the displays already up. */

init_displays1()
{
    int len;
    Side *side;

    for_all_sides(side) {
	len = 8 + strlen(side->name) + 1;
	maxnamelen = max(maxnamelen,
			 len + (side->host ? hostlen(side->host) + 3 : 0));
    }
    for_all_sides(side) {
	if (side->host != NULL) {
	    init_display(side);
	}
    }
}

/* The very first step in using X is to open up all the desired displays. */
/* In our case, there are many displays each with many windows.  If the */
/* display opens successfully, do an initial "redraw" to set the screen up. */

init_displays2()
{
    Side *side;

    for_all_sides(side) {
	if (side->host != NULL) {
	    if (active_display(side)) {
		init_curxy(side);
#if 1 /* -ml */
		redraw(side);
#endif
	    }
	}
    }
}

/* The length of the printed host name is the length till the first */
 /* period or the total length */

int hostlen(str)
char str[];
{
  int len = 0;

  while ((str[len] != '.') && (str[len] != '\0')) len++;
  return len;
}

/* Open display, create all the windows we'll need, do misc setup things, */
/* and initialize some globals to out-of-range values for recognition later. */

init_display(side)
Side *side;
{
    int i;

    if (Debug) printf("Will try to open display \"%s\" ...\n", side->host);

    if (!open_display(side)) {
	fprintf(stderr, "Display \"%s\" could not be opened!\n", side->host);
	exit_xconq();
    }
    active_display(side);   /* done for side effect */
    init_colors(side);
    init_misc(side);
    init_sizes(side);
    create_main_window(side);
    side->msg =
	create_window(side, 0, 0,
		      side->lw, side->nh * side->fh);
    side->info =
	create_window(side, 0, side->nh * side->fh + side->bd,
		      side->lw, INFOLINES * side->fh);
    side->prompt =
	create_window(side, 0, (side->nh + INFOLINES) * side->fh + 2*side->bd,
		      side->lw, side->fh);
    side->map =
	create_window(side, 0, side->tlh,
		      side->vw * side->hw,
		      side->vh * side->hch + (side->hh - side->hch));
    set_retain(side, side->map);
    side->sides =
	create_window(side, side->lw + side->bd + 1, 0,
		      side->sw * side->fw, numsides * side->fh);
    side->timemode =
	create_window(side, side->lw + side->fw, side->trh - 4 * side->fh,
		      2 * side->margin + 8 * side->fw, 2 * side->fh);
    side->clock =
	create_window(side, side->lw + 13 * side->fw, side->trh - 2 * side->fh,
		      8 * side->fw, side->fh);
    side->state =
	create_window(side, side->lw + 1, side->trh,
		      22 * side->fw, period.numutypes*max(side->hh, side->fh));
    if (world_display(side)) {
	side->world =
	    create_window(side,
			  side->lw+side->bd, side->mh-side->mm*world.height,
			  world.width * side->mm, world.height * side->mm);
	set_retain(side, side->world);
    }
    create_help_window(side);
    fixup_windows(side);
    enable_input(side);
    for (i = 0; i < MAXNOTES; ++i) side->noticebuf[i] = " ";
    for (i = 0; i < MAXUTYPES; ++i) side->bvec[i] = 0;
    /* Flag some values as uninitialized */
    side->vcx = side->vcy = -1;
    side->lastvcx = side->lastvcy = -1;
    side->lastx = side->lasty = -1;
    if (Debug) printf("Successfully opened \"%s\"!\n", side->host);
}

/* Initial sizing tries to take as much screen as possible. */

init_sizes(side)
Side *side;
{
  if (!resize_display(side, display_width(side), display_height(side))) {
    fprintf(stderr, "Display \"%s\" is too small!\n", side->host);
    exit_xconq();
  }
}

/* Decide/compute all the sizes of things.  Our main problem here is that */
/* the display might be smaller than we really desire, so things have to */
/* be adjusted to fit. */

resize_display(side, mw, mh)
Side *side;
int mw, mh;
{
  /* if you adjust this procedure, please make a corresponding
     adjustment to resize_display2() */
    int alw, abh;

    alw = max(mw/4, world.width);
    side->sw = min(maxnamelen+5, alw / side->fw);

    alw = mw - max((side->sw * side->fw), world.width) - side->bd;
    side->vw = min(world.width, alw / side->hw);
    side->nw = min(BUFSIZE, alw / side->fw);

    abh = (2 * mh) / 3;
    side->vh = min(world.height, abh / side->hch);
    side->nh = max(1, min((abh/side->fh)/2 - 5, MAXNOTES));
    side->mm = min(5, (max(world.width, side->fw*side->sw) / world.width));
    set_sizes(side);
    side->mw = mw;
    side->mh = mh;
    return (side->vw >= MINWIDTH && side->vh >= MINHEIGHT);
}

/* this version doesn't adjust the map magnification. */
resize_display2(side, mw, mh)
Side *side;
int mw, mh;
{
  /* if you adjust this procedure, please make a corresponding
     adjustment to resize_display() */
    int alw, abh;

    alw = max(mw/4, side->mm*world.width);
    side->sw = min(maxnamelen+5, alw / side->fw);

    alw = mw - max((side->sw * side->fw), side->mm*world.width) - side->bd;
    side->vw = min(world.width, alw / side->hw);
    side->nw = min(BUFSIZE, alw / side->fw);

    abh = (2 * mh) / 3;
    side->vh = min(world.height, abh / side->hch);
    side->nh = max(1, min((abh/side->fh)/2 - 5, MAXNOTES));
    set_sizes(side);
    side->mw = mw;
    side->mh = mh;
    return (side->vw >= MINWIDTH && side->vh >= MINHEIGHT);
}

/* This fn is a "ten-line horror"; effectively a mini tiled window manager */
/* that keeps the subwindows from overlapping no matter what the display and */
/* requested sizes are.  Siemens eat your heart out... */

set_sizes(side)
Side *side;
{
    int ulhgt, llhgt, urhgt, lrhgt;

    /* Make sure map window dimensions are OK */
    side->vw = min(world.width, side->vw);
    side->vh = min(world.height, side->vh);
    side->vw2 = side->vw / 2;  side->vh2 = side->vh / 2;
    side->vw_odd = side->vw % 2;
    /* Compute subregion sizes (left/right upper/lower width/height) */
    side->lw = max(side->nw * side->fw, side->hw * side->vw);
    side->rw = max((world_display(side) ? world.width * side->mm : 0),
		   side->sw * side->fw);

    side->trh = (numsides + 4) * side->fh + side->bd;
    side->tlh = side->fh * (side->nh + INFOLINES + 1) + 3 * side->bd;

    side->brh = period.numutypes * max(side->hh, side->fh);
    if (world_display(side))
      side->brh += side->mm * world.height + 2*side->bd;
    side->blh = side->hch * side->vh + (side->hh - side->hch);

    if (side->brh+side->trh > side->blh+side->tlh)
      side->blh = side->brh+side->trh - side->tlh;

    if (side->trh > side->tlh) {
      side->brh = side->tlh+side->blh - side->trh;
    } else if (side->brh > side->blh) {
      side->trh = side->tlh+side->blh - side->brh;
    } else {
      side->brh = side->blh;
      side->trh = side->tlh;
    }

    side->mw = side->lw + side->bd + side->rw;
    side->mh = side->tlh + 3*side->bd + side->blh;
    /* Only vcy needs adjustment, since vcx can never be close to an edge */
    side->vcy = min(max(side->vh2, side->vcy), (world.height-1)-side->vh2);
}

/* Acquire a set of colors.  There are too many to make it feasible to */
/* customize them via .Xdefaults, so we don't even try.  If there aren't */
/* enough colors, drop into monochrome mode.  This doesn't take the window */
/* manager into account - it may have grabbed some color space. */

init_colors(side)
Side *side;
{
    if (Debug) printf("%d colors available ...\n", display_colors(side));
    side->monochrome = (display_colors(side) <= 2);
    side->bonw = (side->monochrome ? BLACKONWHITE : FALSE);
    set_colors(side);
}

/* This will set up the correct set of colors at any point in the game. */
/* Note that terrain colors get NO value if in monochrome mode. */
/* If the colors requested are not available, it is up to the graphics */
/* interface to supply a substitute. */


long
get_map_color(side, name, class, fallback)
     Side	*side;
     char	*name, *class, *fallback;
{
  static char	rmName[200], rmClass[200];
  sprintf(rmName, "map.%s.%s", period.name, name);
  sprintf(rmClass, "map.Period.%s", class);
  return request_color(side, rmName, rmClass, fallback);
}

set_colors(side)
Side *side;
{
  int	t;
  long fg, bg;
  
  fg = (side->bonw ? black_color(side) : white_color(side));
  bg = (side->bonw ? white_color(side) : black_color(side));
  side->bgcolor = side->owncolor = side->altcolor = side->diffcolor = bg;
  side->fgcolor = side->bdcolor = side->graycolor = side->enemycolor = fg;
  side->neutcolor = side->goodcolor = side->badcolor = fg;
  if (!side->monochrome) {
    static char	*FG="Foreground", *BG="Background";
    for_all_terrain_types(t) {
      side->hexcolor[t] = request_color(side, NULL, NULL, ttypes[t].color);
    }
#if 1
    get_color_resources(side);
#else
    side->owncolor = get_map_color(side,"ownColor",BG,"black");
    side->altcolor = get_map_color(side,"alternateColor",BG,"blue");
    side->diffcolor = get_map_color(side,"differentColor",BG,"maroon");
    side->bdcolor = get_map_color(side,"borderColor",FG,"blue");
    side->graycolor = get_map_color(side,"grayColor",FG,"light gray");
    side->enemycolor = get_map_color(side,"enemyColor",FG,"red");
    side->neutcolor = get_map_color(side,"neutralColor",FG,"light gray");
    side->goodcolor = get_map_color(side,"goodColor",FG,"green");
    side->badcolor = get_map_color(side,"badColor",FG,"red");
#endif
  }
}

/* Move windows and change their sizes to correspond with the new sizes of */
/* viewports, etc. */

reconfigure_display(side, rzmain)
Side *side;
int rzmain;
{
  int sy, sdy;
  
  if (active_display(side)) {
    if (rzmain)
      set_colors(side);
    reset_misc(side);
    sy = 0;  sdy = side->nh * side->fh;
    change_window(side, side->msg, 0, sy, side->lw, sdy);
    sy += sdy + side->bd;  sdy = INFOLINES * side->fh;
    change_window(side, side->info, 0, sy, side->lw, sdy);
    sy += sdy + side->bd;  sdy = 1 * side->fh;
    change_window(side, side->prompt, 0, sy, side->lw, sdy);
    change_window(side, side->map,
		  0, side->tlh,
		  side->vw * side->hw,
		  side->vh * side->hch + (side->hh - side->hch));
    change_window(side, side->sides,
		  side->lw + side->bd, 0, -1, -1);
    change_window(side, side->timemode,
		  side->lw + side->fw, side->trh - 4 * side->fh, -1, -1);
    change_window(side, side->clock,
		  side->lw + 13 * side->fw, side->trh - side->fh, -1, -1);
    change_window(side, side->state,
		  side->lw + side->bd, side->trh + side->bd, -1, -1);
    if (world_display(side)) {
      change_window(side, side->world,
		    side->lw+side->bd, side->mh - world.height*side->mm,
		    world.width * side->mm, world.height * side->mm);
    }
    if (rzmain) {
      change_window(side, side->main, -1, -1, side->mw, side->mh);
      printf("main window configured to %dx%d\n", side->mw, side->mh);
    }
#if 0
    redraw(side);
#endif
  }
}

