/* Copyright (c) 1987, 1988  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Random definitions useful for nearly any C program. */

typedef unsigned char unchar;
typedef unsigned short unshort;

/* If some system tries to redefine bool, we're screwed. */

#define bool int

/* Some systems define TRUE and FALSE themselves. */

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef MSDOS
#define abs(x) (((x) < 0) ? (0 - (x)) : (x))
#endif /* MSDOS */

#define min(x,y) (((x) < (y)) ? (x) : (y))

#define max(x,y) (((x) > (y)) ? (x) : (y))

#define between(lo,n,hi) ((lo) <= (n) && (n) <= (hi))

#define flip_coin() (random(257) % 2)

#define avg(a,b) (((a) + (b)) / 2)

#define iswhite(c) ((c) == ' ' || (c) == '\n' || (c) == '\t')

#define lowercase(c) (isupper(c) ? tolower(c) : (c))

#define uppercase(c) (islower(c) ? toupper(c) : (c))

/* Miscellaneous declarations. */

extern bool Debug, Build, Freeze;
extern int Cheat;

extern char spbuf[], tmpbuf[], version[];
extern char *plural_form(), *copy_string(), *read_line();
extern char *xconqlib;

extern char *copy_string(), *read_line();
extern void init_random();
#ifdef hpux
#define RANDOM(m) (random()%(m))
#else
#define RANDOM(m) random(m)
#endif
void recenter();
#ifdef UNIX
#ifndef MSDOS
extern char *malloc();
extern char *getenv();
#endif /* MSDOS */
#endif /* UNIX */
extern int unwrap();

#ifdef STACK

#define PROCSTACKSIZE 30
extern char *procedure_executing[PROCSTACKSIZE], *routine_executing;
extern int procedure_stack_ptr;
#define enter_procedure(x) {procedure_stack_ptr =\
 (procedure_stack_ptr + 1) % PROCSTACKSIZE; \
   procedure_executing[procedure_stack_ptr] = x;}
#define exit_procedure() {procedure_stack_ptr =\
(procedure_stack_ptr == 0 ? PROCSTACKSIZE - 1 : (procedure_stack_ptr - 1));}
#define cur_procedure() procedure_executing[procedure_stack_ptr]
#define routine(x) routine_executing = x

#else

#define enter_procedure(x)
#define exit_procedure()
#define cur_procedure()
#define routine(x)

#endif
