/*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 * @(#)main.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <signal.h>
#include <pwd.h>
#include "mach_dep.h"
#include "rogue.h"

#ifdef CHECKTIME
static int num_checks = 0;	/* times we've gone over in checkout() */
#endif

main(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    register struct passwd *pw;
    register struct linked_list *item;
    register struct object *obj;
    struct passwd *getpwuid();
    char *getpass(), *crypt();
    int quit();

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, -1);
	exit(0);
    }
    /*
     */
    if ((env = getenv("UCBHASH")) != NULL)
	if (strcmp(env, "/etc/pwtable") == 0)
	    printf("Cheaters never prosper...\n");
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), "mT")) == 0)
	{
	    wizard = TRUE;
	    argv++;
	    argc--;
	}

    /*
     * get home and options from environment
     */
    setbuf(stdout, outbuf);

    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((pw = getpwuid(getuid())) != NULL)
	strcpy(home, pw->pw_dir);
    else
	home[0] = '\0';
    strcat(home, "/");

    strcpy(file_name, home);
    strcpy(file_name, "rogue.save");

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL)
	{
	    printf("Say, who the hell are you?\n");
	    exit(1);
	}
	else
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

#if MAXLOAD|MAXUSERS
    if (too_much() && !wizard && !author())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a");
	if (fruit[0]=='a' || fruit[0]=='e' || fruit[0]=='i'
	    || fruit[0]=='o' || fruit[0]=='u')
		putchar('n');
	printf(" %s?\n", fruit);
	exit(1);
    }
#endif
    if (argc == 2)
	if (!restore(argv[1], envp)) /* Note: restore will never return */
	    exit(1);
    if (wizard && getenv("SEED") != NULL)
	dnum = atoi(getenv("SEED"));
    else
	dnum = getpid();
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;

    initscr();				/* Start up cursor package */
    setup();
    init_player();			/* Roll up the rogue */
    init_names();			/* Set up names of scrolls */
    init_colors();			/* Set up colors of potions */
    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    cw->_scroll = FALSE;
    mw->_scroll = FALSE;
    stdscr->_scroll = FALSE;
    hw->_scroll = FALSE;
    waswizard = wizard;
    new_level();			/* Draw current level */
    lastscore = -1;
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    fuse(hungry, 0, HUNGERTIME, AFTER);
    daemon(runners, 0, AFTER);
    /*
     * Give the rogue his weaponry.  First a mace.
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    attach(pack, item);
    inpack++;
    obj->o_type = WEAPON;
    obj->o_which = SWORD;
    init_weapon(obj, SWORD);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    cur_weapon = obj;
    /*
     * And his suit of armor
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    attach(pack, item);
    inpack++;
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_ac = a_class[RING_MAIL] - 1;
    cur_armor = obj;
    /*
     * Give him some food too
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    attach(pack, item);
    inpack++;
    obj->o_type = FOOD;
    obj->o_which = 0;
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */
endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */
fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */
rnd(range)
register int range;
{
    return range == 0 ? 0 : abs(RN) % range;
}

/*
 * roll:
 *	roll a number of dice
 */
roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}

setup()
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifndef DUMP
    signal(SIGHUP, auto_save);
    signal(SIGILL, auto_save);
    signal(SIGTRAP, auto_save);
    signal(SIGIOT, auto_save);
    signal(SIGEMT, auto_save);
    signal(SIGFPE, auto_save);
    signal(SIGBUS, auto_save);
    signal(SIGSEGV, auto_save);
    signal(SIGSYS, auto_save);
    signal(SIGPIPE, auto_save);
    signal(SIGTERM, auto_save);
#endif

    signal(SIGINT, quit);
#ifndef DUMP
    signal(SIGQUIT, endit);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif
#ifdef CHECKTIME
    if (!author())
    {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
    }
#endif
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */
playit()
{
    register char *opts;
    register bool wasrunning;

    /*
     * set up defaults for slow terminals
     */

    if (_tty.sg_ospeed < B1200)
    {
	terse = TRUE;
	jump = TRUE;
    }

    /*
     * parse environment declaration of options
     */
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);

    oldpos = hero;
    while (playing)
    {
	wasrunning = running;
	look();
	if (!running && wasrunning)
	{
	    door_stop = FALSE;
	    wasrunning = FALSE;
	}
	status();
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump))
	    draw(cw);
	command();
	if (!running && wasrunning)
	    door_stop = FALSE;
    }
    endit();
}

#if MAXLOAD|MAXUSERS
/*
 * see if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
    double avec[3];
#else
    register int cnt;
#endif

#ifdef MAXLOAD
    loadav(avec);
    return (avec[2] > (MAXLOAD / 10.0));
#else
    return (ucount() > MAXUSERS);
#endif
}

/*
 * see if a user is an author of the program
 */
author()
{
    switch (getuid())
    {
	case 0:
	case 24601:
	case 25900:
	    return TRUE;
	default:
	    return FALSE;
    }
}
#endif

#ifdef CHECKTIME
checkout()
{
    signal(SIGALRM, checkout);
    if (too_much())
    {
	switch (num_checks++)
	{
	    when 0:
		if (terse)
		    chmsg("High load.  Please leave in %d minutes", 15);
		else
		    chmsg("The load is too high to be playing.  Please leave in %d minutes", 15);
	    when 1:
		chmsg("Please save your game.  You have %d minutes", 7);
	    when 2:
		chmsg("Last warning.  You have %d minutes to leave", 3);
	    when 3:
		fatal("Sorry.  You took to long.  You are dead\n");
	}
	alarm(CHECKTIME * 60 / (num_checks + 1));
    }
    else
    {
	if (num_checks)
	{
	    chmsg("The load has dropped back down.  You have a reprieve.");
	    num_checks = 0;
	}
	alarm(CHECKTIME * 60);
    }
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
    if (in_shell)
    {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    }
    else
	msg(fmt, arg);
}
#endif

#ifdef LOADAV

loadav(avg)
register double *avg;
{
    int buf[3];

    if (gldav(buf) < 0)
	return;
    avg[0] = (double) buf[0] / 256.0;
    avg[1] = (double) buf[1] / 256.0;
    avg[2] = (double) buf[2] / 256.0;
}
#endif

#ifdef UCOUNT

#include <utmp.h>

struct utmp buf;

ucount()
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif

# ifdef SIGTSTP
/*
 * handle stop and start signals
 */
tstp()
{
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    raw();	/* flush input */
    noraw();
}
# endif
