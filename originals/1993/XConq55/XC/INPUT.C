/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* To accommodate simultaneous movement by several players, our setup must */
/* be a little elaborate.  Basically, the program keeps the initiative; */
/* when a side's movement phase needs input, it posts a "request" and a */
/* handler for that request.  When X gets an event, data about it is */
/* preprocessed and then stuffed into the request structure.  At the end */
/* of a movement subphase, the requests are fulfilled by calling the handler */
/* and passing it the side and a pointer to the request. */

/* The bad part about all this is that *every* *single* interaction must */
/* be handled this way - no more calling a routine to get a value and */
/* waiting until it is complete! :-( */

/* This last restriction does not seem necessary if the request is being */
/* made by the current side.  In that case, additional requests being */
/* made means that requests must be handled immediately anyways. */

   

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "period.h"
#include "side.h"
#include "unit.h"
#include "global.h"
#include "map.h"

/* Magic values (unlikely to show up as player input) */

#define DFLTFLAG (-12345)       /* flags when default arg to be used */
#define NEGFLAG  (-12346)       /* flags when default arg to be used */
#define ITERFLAG (-12347)       /* flags args that are "itertime" */
#define NEXTPAGE (-101010)      /* Print these commands on the next page */

#define ESCAPE '\033'           /* standard abort character */
#define BACKSPACE '\010'        /* for fixing strings being entered */

/* The following structure is used only for the dispatch table. */

typedef struct func_tab {
    char fchar;                 /* character to match against */
    int (*code)();              /* pointer to command's function */
    int defaultarg;             /* default for argument */
    bool needunit;              /* true if cmd always applies to a unit */
    char *help;                 /* short documentation string */
} FuncTab;

extern x_help();
/* Predeclarations of all commands. */

int do_sentry(), do_delay(), do_wakeup(), do_wakemain(), do_embark(),
    do_fill(), do_exit(), do_resign(), do_message(), do_follow(),
    do_redraw(), do_flash(), do_disband(), do_return(), do_movetotransport(),
    do_sit(), do_product(), do_idle(), do_help(), do_version(),
    do_save(), do_standing(), do_ident(), do_moveto(), do_coast(),
    do_survey_mode(), do_unit_info(), do_printables(), do_occupant(),
    do_options(), do_name(), do_unit(), do_give_unit(), do_center(),
    do_mark_unit(), do_add_player(), do_patrol(), do_give(), do_take(),
    do_recenter(), do_look(), do_distance(), do_find_unit(), do_nothing(),
    do_find_next_unit(), do_next_product(), do_auto(), do_next_dead(),
    do_movetounit();

char dirchars[] = DIRCHARS;     /* typable characters for directions */

/* The command table itself.  Default iterations of ITERFLAG actually turn */
/* into the value of "itertime", which can be set by options cmd. */

FuncTab commands[] = {
    's', do_sentry, ITERFLAG, TRUE, "put a unit on sentry duty",
    'w', do_wakemain, 0, FALSE, "wake units up from whatever they were doing",
    ' ', do_sit, 1, FALSE, "(Space Bar) make unit be on sentry this turn only",
    'r', do_return, ITERFLAG, TRUE, "return unit to nearest city/transport",
    'd', do_delay, 1, TRUE, "delay unit's move until later in turn",
    'f', do_follow, ITERFLAG, TRUE, "follow a designated leader",
    'm', do_moveto, 1, TRUE, "move to given location",
    '\015', do_movetounit, ITERFLAG, TRUE, "move to the marked unit",
    'z', do_survey_mode, 1, FALSE, "toggle between survey and move modes",
    'i', do_occupant, 1, TRUE, "look at the occupants",
    'e', do_embark, 0, TRUE, "embark units onto transport occupying same unit",
    '\006', do_fill, ITERFLAG, TRUE, "order transport to sleep until full",
    'x', do_mark_unit, 1, TRUE, "mark unit for later reference",
    'g', do_give, -1, TRUE, "give supplies to the transport",
    't', do_take, -1, TRUE, "take supplies from the transport",
    'c', do_center, 1, FALSE, "designate current location as center of action",
    'o', do_options, 6, FALSE, "set various options",
    ' ', NULL, 0, 0, "Change (D)isplay Mode, (G)raph, Win (H)eight,",
    ' ', NULL, 0, 0, "(I)nverse Video, (M)onochrome, (N)otice Buffer",
    ' ', NULL, 0, 0, "(R)obot, Start (B)eep Time, Win (W)idth",
    '+', do_printables, 1, FALSE, "put various data into files for printing",
    'v', do_flash, 1, FALSE, "highlight current position",
    ' ', NULL, 0, 0, "",
    'P', do_product, 1, TRUE, "set/change unit production",
    'I', do_idle, ITERFLAG, TRUE, "set unit to not produce anything",
    'W', do_wakeup, 0, FALSE, "wake ALL units in area, including occupants",
    'D', do_disband, 1, TRUE, "disband a unit and send it home",
    'E', do_movetotransport, ITERFLAG, TRUE, "move to filling transport",
    'F', do_coast, ITERFLAG, TRUE, "follow a coastline",
    'p', do_patrol, ITERFLAG, TRUE, "go on a patrol",
    'O', do_standing, 1, TRUE, "set standing orders for occupants",
    'G', do_give_unit, -2, TRUE, "turn unit over to another side",
    'M', do_message, -2, FALSE, "send a message to other sides",
    'X', do_resign, -2, FALSE, "resign from the game",
    'Q', do_exit, 1, FALSE, "kill game for all players",
    'S', do_save, 1, FALSE, "save the game into a file",
    'A', do_add_player, 1, TRUE, "add a new player to the game",
    ' ', NULL, NEXTPAGE, 0, "",
    'C', do_name, 1, FALSE, "call a unit or side by a name",
    'T', do_find_unit, -1, FALSE, "find a unit type, (arg gives unit number)",
    '\024', do_find_next_unit, -1, FALSE,
           "find next unit, (arg gives unit number)",
    '\004', do_next_dead, -1, FALSE, "Show next dead unit.",
    '?', do_help, 1, FALSE, "display help info",
    '/', do_ident, 1, FALSE, "identify things on screen",
    '=', do_unit_info, 1, FALSE, "display details about a type of unit",
    'V', do_version, 1, FALSE, "display program version",
    '\\', do_unit, 1, FALSE, "build a new unit (Build mode only)",
    '\001', do_auto, 1, TRUE, "Turn unit over to machine control",
    '\020', do_next_product, 1, TRUE, "set next product",
    '\022', do_redraw, 10, FALSE, "redraw screen erase very old views",
    '\014', do_redraw, 0, FALSE, "redraw screen",
           /* this is really hardwired into X11.c */
    '\027', do_look, -1, FALSE, "watch the action point when not your turn",
    '.', do_recenter, 1, FALSE, "center the screen around current location",
    '#', do_distance, 1, FALSE, "measure the distance from the current point",
    '\177', do_nothing, 1, FALSE, "delete a standing order",
    '\0', NULL, 0, FALSE, NULL   /* end of table marker */
};

/* Just to get everything off on the right foot. */

init_requests(side)
Side *side;
{
    side->reqvalue = DFLTFLAG;
}

/* Post a request - need only know the handler that will be called to */
/* fulfill the request later, and sometimes a relevant unit. */

request_input(side, unit, handler)
Side *side;
Unit *unit;
int (*handler)();
{
    if (Debug) printf("Making %s request ", side->name);
    if (!humanside(side))
      fprintf(stderr, "Gack!\n");
    if (!side->reqactive) {
	if (Debug) printf("(accepted)\n");
	side->reqactive = TRUE;
	side->reqhandler = handler;
	side->reqtype = GARBAGE;
	side->requnit = unit;
	side->reqchange = TRUE;
	/* any other data slots are setup by callers of this routine */
    } else {
	if (Debug) printf("(ignored)\n");
    }
}

/* Make a request go away.  Careful about using this! */

cancel_request(side)
Side *side;
{
    if (side->reqactive) {
	if (Debug) printf("Canceling %s request\n", side->name);
	side->reqactive = FALSE;
	side->reqchange = TRUE;
	if (side->reqhandler == x_help) {
	  conceal_help(side);
	  redraw(side);
	} else {
	  erase_cursor(side);
	  clear_info(side);
	  flush_output(side);
	}
    }
    flush_input(side);
}



/* Handle all requests at once.  This routine *will* wait forever for */
/* input (polling is evil).  This is also the right place to draw the */
/* unit cursor, since it is unwanted unless we are waiting on input. */

handle_requests()
{
    bool waiters = FALSE;
    Side *side;

/* if (Debug) printf("@handle_requests(): \n"); */

    for_all_sides(side) {
	if (side->reqactive) {
            if (Debug) printf("Handling request for %s\n", side->name);
	    waiters = TRUE;
	    /* wasteful to call this for everybody... */
	    if (side->reqchange) { 
	      show_info(side);
	      draw_cursor(side);
	      flush_output(side);
	      side->reqchange = FALSE;
	    } 
	}
    }
    if (waiters) {
	get_input();
	for_all_sides(side) {
	    if (side->reqactive) {
		if (side->reqtype != GARBAGE) {
		    if (Debug) printf("Answering %s request with inptype %d\n",
				      side->name, side->reqtype);
		    side->reqactive = FALSE;
		    (*(side->reqhandler))(side);
		    side->reqchange = TRUE;
		    erase_cursor(side);
		    clear_info(side);
		    flush_output(side);
		    side->reqtype = GARBAGE;
		  }
	      }
	    if (!humanside(side) && side->reqtype == KEYBOARD &&
		side->reqch == ESCAPE) {
		side->humanp = TRUE;
		numhumans++;
		init_sighandlers();
	      }

	}
    }
}

/* Acquire a command from the player.  Command may be prefixed with a */
/* nonnegative number which is given as an argument to the command function */
/* (otherwise arg defaults to value in third column of command table). */
/* This routine takes in both keyboard and mouse input, other kinds of */
/* devices should be handled here also (rriiiight...).  Don't do any of */
/* this if the clock ran out, just supply an innocuous command. */

x_command(side)
Side *side;
{
    int dir, terr, sign = 1;

    switch (side->reqtype) {
    case KEYBOARD:
	if (Debug) printf("%s keyboard input: %c (%d)\n",
			  side->name, side->reqch, side->reqvalue);
	if (isdigit(side->reqch)) {
	    if (side->reqvalue == DFLTFLAG) {
		side->reqvalue = 0;
	    } else if (side->reqvalue == NEGFLAG) {
		side->reqvalue = 0;
		sign = -1;
	    } else {
		side->reqvalue *= 10;
	    }
	    side->reqvalue +=
		(side->reqvalue >= 0 ? 1 : -1) * (side->reqch - '0');
	    side->reqvalue *= sign;
	    sprintf(side->promptbuf, "Arg: %d", side->reqvalue);
	    show_prompt(side);
	    request_input(side, side->curunit, x_command);
	    return;
	} else {
	    clear_prompt(side);
	    if (Build && ((terr = find_terr(side->reqch)) >= 0)) {
		if (side->reqvalue == DFLTFLAG) side->reqvalue = 0;
		do_terrain(side, terr, side->reqvalue);
	    } else if (side->reqch == '-') {
		side->reqvalue = NEGFLAG;
		request_input(side, side->curunit, x_command);
		return;
	    } else if ((dir = iindex(side->reqch, dirchars)) >= 0) {
		if (side->reqvalue == DFLTFLAG) side->reqvalue = 1;
		do_dir(side, dir, side->reqvalue);
	    } else if ((dir = iindex(lowercase(side->reqch), dirchars)) >= 0) {
		if (side->reqvalue == DFLTFLAG)
		    side->reqvalue = side->itertime;
		if (side->mode == SURVEY) side->reqvalue = 10;
		do_dir(side, dir, side->reqvalue);
	    } else {
		execute_command(side, side->reqch, side->reqvalue);
	    }
	}
	break;
    case MAPPOS:
	if (Debug) printf("%s map input: %d,%d (%d)\n",
			  side->name, side->reqx, side->reqy, side->reqvalue);
	clear_prompt(side);
	if (side->reqx == side->curx && side->reqy == side->cury) {
	    if (side->curunit) do_sit(side, 1);
	    break;
	} else {
	    if (side->teach) {
		cache_moveto(side, side->reqx, side->reqy);
	    } else {
		switch (side->mode) {
		case MOVE:
		    if (side->curunit) {
		      Unit	*dest;
		      if (side->reqch &&
			  NULL!=(dest=unit_at(side->reqx, side->reqy))) {
			order_movetounit(side->curunit, dest, side->itertime);
		      } else {
			order_moveto(side->curunit, side->reqx, side->reqy);
			side->curunit->orders.flags &= ~SHORTESTPATH;
		      }
		    }
		    break;
		case SURVEY:
		    move_survey(side, side->reqx, side->reqy);
		    break;
		default:
		    case_panic("mode", side->mode);
		}
	    }
	}
	break;
    case SCROLLUP: 
	if (side->bottom_note + side->nh >= MAXNOTES) beep(side);
	else side->bottom_note = min(side->bottom_note + side->nh,
				     MAXNOTES - side->nh);
	show_note(side,TRUE);
	break;
    case SCROLLDOWN:
	if (side->bottom_note == 0) beep(side);
	else side->bottom_note = max(0, side->bottom_note - side->nh);
	show_note(side,TRUE);
	break;
    default:
	break;
    }
    if (global.giventime && !side->timedout && side->timeleft <= 0) {
	side->timedout = TRUE;
	notify(side, "You ran out of time!!");
	beep(side);
    }
    /* Reset the arg so we don't get confused next time around */
    side->reqvalue = DFLTFLAG;
}

/* The requester proper, which just sets up the hook to call the main */
/* command interpreter when some input comes by. */

request_command(side)
Side *side;
{
    request_input(side, side->curunit, x_command);
}

/* Search in command table and execute function if found, complaining if */
/* the command is not recognized.  Many commands operate on the "current */
/* unit", and all uniformly error out if there is no current unit, so put */
/* that test here.  Also fix up the arg if the value passed is one of the */
/* specially recognized ones. */

execute_command(side, ch, n)
Side *side;
char ch;
int n;
{
    struct func_tab *cmd;
    
    for (cmd = commands; cmd->fchar != '\0'; ++cmd) {
	if (ch == cmd->fchar) {
	    if (n == DFLTFLAG) n = cmd->defaultarg;
	    if (n == NEGFLAG)  n = 0 - cmd->defaultarg;
	    if (n == ITERFLAG) n = side->itertime;
	    if (Debug) printf("... actual arg is %d\n", n);
	    if (cmd->needunit) {
		if (side->curunit != NULL) {
		    (*(cmd->code))(side, side->curunit, n);
		} else {
		    cmd_error(side, "No unit to operate on here!");
		}
	    } else {
		(*(cmd->code))(side, n);
	    }
	    return;
	}
    }
    cmd_error(side, "Unknown command '%c'", ch);
}

/* Help for the main command mode just dumps part of the table, and a little */
/* extra info about what's not in the table.  This may go onto a screen or */
/* into a file, depending on where this was called from. */

command_help(side, n)
Side *side;
int n;
{
    FuncTab *cmd;

    wprintf(side, "To move a unit, use the mouse or [hlyubn]");
    wprintf(side, "[HLYUBN] moves unit repeatedly in that direction");
    wprintf(side, "Mousing unit makes it sit, mousing enemy unit attacks");
    if (Build) wprintf(side, "Terrain characters set what will be painted.");
    wprintf(side, "  ");
    for (cmd = commands; cmd->fchar != '\0'; ++cmd) {
      if (n == 0) {
	if (cmd->fchar < ' ' || cmd->fchar > '~') 
	  wprintf(side, "^%c %s", (cmd->fchar^0x40), cmd->help);
	else
	  wprintf(side, "%c  %s", cmd->fchar, cmd->help);
      }
      if (cmd->defaultarg == NEXTPAGE)
	n--;
      if (n < 0) {
	wprintf(side, "  SEE NEXT PAGE FOR MORE COMMANDS");
	return;
      }
    }
}

/* The handler for unit production needs to make sure valid input has */
/* been received, will put in another request if not. */

x_product_type(side)
Side *side;
{
    int u;

    if ((u = grok_unit_type(side)) >= 0) {
      if (u != side->requnit->product) {
	set_product(side->requnit, u);
	set_schedule(side->requnit);
      }
    } else {
	request_input(side, side->requnit, x_product_type);
    }
}
/* The handler for unit production needs to make sure valid input has */
/* been received, will put in another request if not. */

x_next_product_type(side)
Side *side;
{
    int u;

    if ((u = grok_unit_type(side)) >= 0) {
      if (u != NOTHING) {
	side->requnit->next_product = u;
      }
    } else {
	request_input(side, side->requnit, x_next_product_type);
    }
}

/* This is called when production is to be set or changed.  Note that since */
/* the user has a pretty dull choice if there is only one possible type of */
/* unit to build, in such cases we can bypass requests altogether. */

request_new_product(unit)
Unit *unit;
{
    int u;
    Side *us = unit->side;

    if (humanside(us)) {
	sprintf(spbuf, "%s will build: ", unit_handle(us, unit));
	u = ask_unit_type(us, spbuf, utypes[unit->type].make);
	if (u < 0) {
	    make_current(us, unit);
	    request_input(us, unit, x_product_type);
	} else {
	  if (u != unit->product) {
	    set_product(unit, u);
	    set_schedule(unit);
	  }
	}
    } else {
	set_product(unit, machine_product(unit));
	set_schedule(unit);
    }
}

/* This is called when production is to be set or changed.  This
   routine allows the user to specify what will be produced after the
   current production is finished. */

request_next_product(unit)
Unit *unit;
{
    int u;
    Side *us = unit->side;

    if (humanside(us)) {
	sprintf(spbuf, "%s's next product will be: ", unit_handle(us, unit));
	u = ask_unit_type(us, spbuf, utypes[unit->type].make);
	if (u < 0) {
	    make_current(us, unit);
	    request_input(us, unit, x_next_product_type);
	} else {
	  if (u != unit->product  && u != NOTHING) {
	    set_product(unit, u);
	    set_schedule(unit);
	  }
	}
    } else {
       /* the machine should never do this.  Just ignore it.  */
    }
}

/* Prompt for a type of a unit from player, maybe only allowing some types */
/* to be accepted.  Also allow specification of no unit type.  We do this */
/* by scanning the vector, building a string of chars and a vector of */
/* unit types, so as to be able to map back when done. */

ask_unit_type(side, prompt, possibles)
Side *side;
char *prompt;
short *possibles;
{
    int numtypes = 0, u, type;

    for_all_unit_types(u) {
	side->bvec[u] = 0;
	if (possibles == NULL || possibles[u]) {
	    side->bvec[u] = 1;
	    side->ustr[numtypes] = utypes[u].uchar;
	    side->uvec[numtypes] = u;
	    numtypes++;
	}
    }
    if (numtypes == 0) {
	type = NOTHING;
    } else if (numtypes == 1) {
	type = side->uvec[0];
	side->bvec[type] = 0;
    } else {
	side->ustr[numtypes] = '\0';
	sprintf(side->promptbuf, "%s [%s] ", prompt, side->ustr);
	show_prompt(side);
	draw_unit_list(side, side->bvec);
	type = -1;
    }
    return type;
}

/* Do something with the char or unit type that the player entered. */

grok_unit_type(side)
Side *side;
{
    int i, type = -1;

    switch (side->reqtype) {
    case KEYBOARD:
	echo_at_prompt(side, side->reqch);
	if (side->reqch == '?') {
	    help_unit_type(side);
	} else if (side->reqch == ESCAPE) {
	    type = NOTHING;
	} else if (iindex(side->reqch, side->ustr) == -1) {
	    notify(side, "Unit type '%c' not in \"%s\"!",
		   side->reqch, side->ustr);
	} else {
	    type = find_unit_char(side->reqch);
	}
	break;
    case UNITTYPE:
	if (between(0, side->requtype, period.numutypes-1) &&
	    side->bvec[side->requtype]) {
	    type = side->requtype;
	} else {
	    notify(side, "Not a valid unit type!");
	}
	break;
    default:
	break;
    }
    if (type >= 0) {
	clear_prompt(side);
	for_all_unit_types(i) side->bvec[i] = 0;
	draw_unit_list(side, side->bvec);
    }
    return type;
}

/* User is asked to pick a position on map.  This will iterate until the */
/* space bar designates the final position. */

ask_position(side, prompt)
Side *side;
char *prompt;
{
    sprintf(side->promptbuf, "%s [mouse or keys to move, space bar to set]",
	    prompt);
    show_prompt(side);
    side->tmpcurx = side->curx;  side->tmpcury = side->cury;
    side->tmpcurunit = side->curunit;
}

/* Restore the saved "cur" slots. */

restore_cur(side)
Side *side;
{
    side->curx = side->tmpcurx;  side->cury = side->tmpcury;
    side->curunit = side->tmpcurunit;
}

/* Interpret the user's input in response to a position request.  All we */
/* have to comprehend is direction keys and mouse hits.  Space bars and */
/* unmoving mice both mean that a position has been decided on. */

grok_position(side)
Side *side;
{
    int dir;

    switch (side->reqtype) {
    case KEYBOARD:
	if (side->reqch == ' ') {
	    clear_prompt(side);
	    return TRUE;
	} else if ((dir = iindex(side->reqch, dirchars)) >= 0) {
	    side->reqposx = wrap(side->reqposx + dirx[dir]);
	    side->reqposy = side->reqposy + diry[dir];
	} else if ((dir = iindex(lowercase(side->reqch), dirchars)) >= 0) {
	    side->reqposx = wrap(side->reqposx + 10*dirx[dir]);
	    side->reqposy = side->reqposy + 10*diry[dir];
	}
	break;
    case MAPPOS:
	if (side->reqx == side->reqposx && side->reqy == side->reqposy) {
	    clear_prompt(side);
	    return TRUE;
	} else {
	    side->reqposx = side->reqx;  side->reqposy = side->reqy;
	}
	break;
    default:
	break;
    }
    side->curx = side->reqposx;  side->cury = side->reqposy;
    side->curunit = NULL;
    show_info(side);
    return FALSE;
}

/* Prompt for a yes/no answer with a settable default. */

ask_bool(side, question, dflt)
Side *side;
char *question;
bool dflt;
{
    sprintf(side->promptbuf, "%s [%s]", question, (dflt ? "yn" : "ny"));
    show_prompt(side);
    side->reqvalue2 = dflt;
}

/* Figure out what the answer actually is, keeping the default in mind. */

grok_bool(side)
Side *side;
{
    bool dflt = side->reqvalue2;
    char ch = side->reqch;

    if (side->reqtype == KEYBOARD) {
	if (dflt ? (lowercase(ch) == 'n') : (lowercase(ch) == 'y'))
	    dflt = !dflt;
    }
    clear_prompt(side);
    return dflt;
}

/* Prompt for a single character. */

ask_char(side, question, choices)
Side *side;
char *question, *choices;
{
    sprintf(side->promptbuf, "%s [%s]", question, choices);
    show_prompt(side);
}

/* The char has already been processed, so just pass it through. */

grok_char(side)
Side *side;
{
    if (side->reqtype == KEYBOARD) {
	clear_prompt(side);
	return side->reqch;
    } else {
	return '\0';
    }
}

/* Read a string from the prompt window.  Deletion is allowed, and a */
/* cursor is displayed (this should definitely be a toolkit call...) */
/* Some restrictions on what strings can be read - for instance can't */
/* read or default to a NULL string. */

ask_string(side, prompt, dflt)
Side *side;
char *prompt, *dflt;
{
    if (dflt == NULL) {
	sprintf(side->promptbuf, "%s ", prompt);
    } else {
	sprintf(side->promptbuf, "%s (default \"%s\") ", prompt, dflt);
    }
    show_prompt(side);
    side->reqstrbeg = strlen(side->promptbuf);
    side->reqcurstr = side->reqstrbeg;
    write_str_cursor(side);
    side->reqdeflt = dflt;
}

/* Dig a character from the filled-in request and add it into the string. */
/* Keep returning NULL until we get something. */

char *
grok_string(side)
Side *side;
{
    if (side->reqtype == KEYBOARD) {
	if (side->reqch == '\r' ||
	    side->reqch == '\n' ||
	    side->reqcurstr >= BUFSIZE-2) {
	    if (side->reqcurstr == side->reqstrbeg && side->reqdeflt != NULL) {
		strcpy(side->promptbuf+side->reqstrbeg, side->reqdeflt);
	    } else {
		(side->promptbuf)[side->reqcurstr] = '\0';
	    }
	    clear_prompt(side);
	    return copy_string(side->promptbuf + side->reqstrbeg);
	} else {
	    if (side->reqch == BACKSPACE) {
		if (side->reqcurstr > side->reqstrbeg) --side->reqcurstr;
	    } else {
		echo_at_prompt(side, side->reqch);
		(side->promptbuf)[side->reqcurstr++] = side->reqch;
	    }
	}
	write_str_cursor(side);
	(side->promptbuf)[side->reqcurstr+1] = '\0';
    }
    return NULL;
}

/* Return TRUE if someone is busy doing something. */

bool
someone_doing_something()
{
  Side *side;

    for_all_sides(side) {
	if (humanside(side) && side->reqactive &&
	    side->reqhandler != x_command &&
	    side->reqhandler != x_help && !side->lost) 
	    return TRUE;
    }
    return FALSE;
}
