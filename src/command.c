/*
 * Read and execute the user commands
 *
 * @(#)command.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include <signal.h>
#include "rogue.h"

/*
 * command:
 *	Process the user commands
 */
command()
{
    register char ch;
    register int ntimes = 1;			/* Number of player moves */
    static char countch, direction, newcount = FALSE;
    char *unctrl();

    if (on(player, ISHASTE)) ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    firstmove = FALSE;
    while (ntimes--)
    {
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (!no_command)
	{
	    if (running) ch = runch;
	    else if (count) ch = countch;
	    else
	    {
		ch = readchar();
		if (mpos != 0 && !running)	/* Erase message if its there */
		    msg("");
	    }
	}
	else ch = ' ';
	if (no_command)
	{
	    if (--no_command == 0)
		msg("You can move again.");
	}
	else
	{
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch))
	    {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch))
		{
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;
	    }
	    /*
	     */
	    switch (ch)
	    {
		when 'f':
		    door_stop = TRUE;
		    firstmove = TRUE;
		    if (count && !newcount)
			ch = direction;
		    else
			ch = readchar();
		    switch (ch)
		    {
			case 'h': case 'j': case 'k': case 'l':
			case 'y': case 'u': case 'b': case 'n':
			    ch = toupper(ch);
		    }
		    direction = ch;
		when 't':
		    throwing = TRUE;
		    if (count && !newcount)
			ch = direction;
		    else
			ch = readchar();
		    switch (ch)
		    {
			case 'H': case 'J': case 'K': case 'L':
			case 'Y': case 'U': case 'B': case 'N':
			    ch = tolower(ch);
		    }
		    direction = ch;
	    }
	    newcount = FALSE;
	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch)
	    {
		when '!' : shell();
		when 'h' :
		    if (!throwing)
			do_move(0, -1);
		    else
			missile(0, -1);
		when 'j' :
		    if (!throwing)
			do_move(1, 0);
		    else
			missile(1, 0);
		when 'k' :
		    if (!throwing)
			do_move(-1, 0);
		    else
			missile(-1, 0);
		when 'l' :
		    if (!throwing)
			do_move(0, 1);
		    else
			missile(0, 1);
		when 'y' :
		    if (!throwing)
			do_move(-1, -1);
		    else
			missile(-1, -1);
		when 'u' :
		    if (!throwing)
			do_move(-1, 1);
		    else
			missile(-1, 1);
		when 'b' :
		    if (!throwing)
			do_move(1, -1);
		    else
			missile(1, -1);
		when 'n' :
		    if (!throwing)
			do_move(1, 1);
		    else
			missile(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 'Q' : after = FALSE; quit();
		when 'i' : after = FALSE; inventory(pack, 0);
		when 'I' : after = FALSE; picky_inven();
		when 'd' : drop();
		when 'q' : quaff();
		when 'r' : read_scroll();
		when 'e' : eat();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'o' : option();
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when 's' : search();
		when 'v' : msg("Rogue version %s. (mctesq was here)", release);
		when 'M' : after = FALSE; msg("Items allocated : %d", total);
		when 'R' : after = FALSE; msg(huh);
		when CTRL(L) : after = FALSE; clearok(curscr,TRUE);draw(curscr);
		when 'S' :
		    after = FALSE;
		    if (save_game())
		    {
			wmove(cw, LINES-1, 0);
			wclrtoeol(cw);
			draw(cw);
			endwin();
			exit(0);
		    }
		when ' ' : ;			/* Rest command */
		when CTRL(P) :
		    after = FALSE;
		    if (wizard)
		    {
			wizard = FALSE;
			msg("Not wizard any more");
		    }
		    else
		    {
			if (wizard = passwd())
			{
			    msg("You are suddenly as smart as Ken Arnold in dungeon #%d", dnum);
			    waswizard = TRUE;
			}
			else
			    msg("Sorry");
		    }
		when ESCAPE :	/* Escape */
		    if (door_stop || throwing || count)
		    {
			throwing = door_stop = FALSE;
			count = 0;
			after = FALSE;
		    }
		    else goto def;
		otherwise :
		def:
		    after = FALSE;
		    if (wizard) switch (ch)
		    {
			when '@' : msg("@ %d,%d", hero.y, hero.x);
			when 'C' : create_obj();
			when CTRL(I) : inventory(lvl_obj, 0);
			when CTRL(W) : whatis();
			when CTRL(D) : after = TRUE; level++; new_level();
			when CTRL(F) : show_win(stdscr, "--More (level map)--");
			when CTRL(X) : show_win(mw, "--More (monsters)--");
			otherwise :
			    msg("Illegal command '%s'.", unctrl(ch));
			    count = 0;
		    }
		    else
		    {
			msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
		    }
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    throwing = FALSE;
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let him pick it up.
	 */
	if (take != 0)
	    pick_up(take);
	status();
	if (ntimes)
	{
	    look();
	    draw(cw);
	}
    }
    /*
     * Kick off the rest if the daemons and fuses
     */
    if (after)
    {
	do_daemons(AFTER);
	do_fuses(AFTER);
    }
}

/*
 * quit:
 *	Have player make certain, then exit.
 */
quit()
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (readchar() == 'y')
    {
	clear();
	move(LINES-1, 0);
	draw(stdscr);
	score(purse, 1);
	endwin();
	exit(0);
    }
    else
    {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status();
	draw(cw);
	mpos = 0;
	count = 0;
    }
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */
search()
{
    register int x, y;
    char ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++)
	{
	    ch = winat(y, x);
	    switch (ch)
	    {
		case SECRETDOOR:
		    if (rnd(100) < 20) {
			mvaddch(y, x, DOOR);
			count = 0;
		    }
		    break;
		case TRAP:
		{
		    register int i;

		    if (mvwinch(cw, y, x) == TRAP)
			break;
		    if (rnd(100) > 50)
			break;
		    i = trap_at(y, x);
		    traps[i].tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, TRAP);
		    count = 0;
		    msg(tr_name(traps[i].tr_type));
		}
	    }
	}
}

/*
 * tr_name:
 *	print the name of a trap
 */
char *
tr_name(ch)
char ch;
{
    register char *s;

    switch (ch)
    {
	when TRAPDOOR:
	    s = terse ? "A trapdoor." : "You found a trapdoor.";
	when BEARTRAP:
	    s = terse ? "A beartrap." : "You found a beartrap.";
	when SLEEPTRAP:
	    s = terse ? "A sleeping gas trap.":"You found a sleeping gas trap.";
	when ARROWTRAP:
	    s = terse ? "An arrow trap." : "You found an arrow trap.";
	when TELTRAP:
	    s = terse ? "A teleport trap." : "You found a teleport trap.";
	when DARTTRAP:
	    s = terse ? "A dart trap." : "You found a posion dart trap.";
    }
    return s;
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */
help()
{
    register struct h_list *strp = helpstr;
    register char helpch;
    register int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar();
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*')
    {
	wmove(cw, 0, 0);
	while (strp->h_ch)
	{
	    if (strp->h_ch == helpch)
	    {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'", unctrl(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch)
    {
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	cnt++;
	strp++;
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw, "--Press space to continue--");
    draw(hw);
    wait_for(' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status();
    touchwin(cw);
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */
identify()
{
    register char ch, *str;

    msg("What do you want identified? ");
    draw(cw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status();
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE)
    {
	msg("");
	return;
    }
    if (isalpha(ch) && isupper(ch))
	str = monsters[ch-'A'].m_name;
    else switch(ch)
    {
	case '|':
	case '-':
	    str = "wall of a room";
	when GOLD: str = "gold";
	when STAIRS : str = "passage leading down";
	when DOOR: str = "door";
	when FLOOR: str = "room floor";
	when PLAYER: str = "you";
	when PASSAGE: str = "passage";
	when TRAP: str = "trap";
	when POTION: str = "potion";
	when SCROLL: str = "scroll";
	when FOOD: str = "food";
	when WEAPON: str = "weapon";
	when ' ' : str = "solid rock";
	when ARMOR: str = "armor";
	when AMULET: str = "The Amulet of Yendor";
	otherwise: str = "unknown character";
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * Look:
 *	A quick glance all around the player
 */
look()
{
    register int x, y;
    register char ch;
    register int oldx, oldy;
    register bool inpass;
    register int passcount = 0;
    register struct room *rp;

    getyx(cw, oldy, oldx);
    if ((rp = roomin(&oldpos)) != NULL && (rp->r_flags & ISDARK))
    {
	for (x = oldpos.x - 1; x <= oldpos.x + 1; x++)
	    for (y = oldpos.y - 1; y <= oldpos.y + 1; y++)
		if (show(y, x) == FLOOR && (y != hero.y || x != hero.x))
		    mvwaddch(cw, y, x, ' ');
    }
    inpass = (roomin(&hero) == NULL);
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= hero.y + 1; y++)
	{
	    if (y <= 0 || y >= LINES - 1)
		continue;
	    /*
	     * Secret doors show as walls
	     */
	    ch = show(y, x);
	    if (ch == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage
	     */
	    if (y == hero.y && x == hero.x
		|| (inpass && (ch == '-' || ch == '|')))
		    continue;
	    wmove(cw, y, x);
	    waddch(cw, ch);
	    if (door_stop && !firstmove && running)
	    {
		if (ch != PASSAGE)
		switch (runch)
		{
		    when 'h':
			if (x == hero.x + 1)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == hero.y + 1)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch)
		{
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount++;
			break;
		    case FLOOR:
		    case '|':
		    case '-':
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 2)
	running = FALSE;
    wmove(cw, oldy, oldx);
    oldpos = hero;
}

/*
 * d_level:
 *	He wants to go down a level
 */
d_level()
{
    if (winat(hero.y, hero.x) != STAIRS)
	msg("I see no way down.");
    else
    {
	level++;
	new_level();
    }
}

/*
 * u_level:
 *	He wants to go up a level
 */
u_level()
{
    register struct linked_list *item;
    register struct object *obj;

    if (winat(hero.y, hero.x) == STAIRS)
    {
	for (item = pack; item != NULL; item = next(item))
	{
	    obj = (struct object *) ldata(item);
	    if (obj->o_type == AMULET)
		break;
	}
	if (item != NULL)
	{
	    msg("You feel a wrenching sensation in your gut.");
	    level--;
	    if (level == 0)
		total_winner();
	    new_level();
	    return;
	}
    }
    msg("I see no way up.");
}

/*
 * secretdoor:
 *	Figure out what a wall looks like at this position.
 */
secretdoor(y, x)
register int y, x;
{
    register int i;
    register struct room *rp;
    register coord *cpp;
    coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++)
	if (!(rp->r_flags & ISGONE) && inroom(rp, cpp))
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return('-');
	    else
		return('|');

    return('p');
}

/*
 * Let him escape for a while
 */
shell()
{
    register int pid;
    register char *sh;
    int ret_status;

    /*
     * Set the terminal back to original mode
     */
    sh = getenv("SHELL");
    wclear(hw);
    wmove(hw, LINES-1, 0);
    draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    while((pid = fork()) < 0)
	sleep(1);
    if (pid == 0)
    {
	/*
	 * Set back to original user, just in case
	 */
	setuid(getuid());
	setgid(getgid());
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	exit(-1);
    }
    else
    {
	int endit();

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait(&ret_status) != pid)
	    continue;
	signal(SIGINT, endit);
	signal(SIGQUIT, endit);
	printf("\n[Press return to continue]");
	noecho();
	crmode();
	in_shell = FALSE;
	wait_for('\n');
	clearok(cw, TRUE);
	touchwin(cw);
    }
}

/*
 * passwd:
 *	see if user knows password
 */
passwd()
{
    register char *sp, c;
    char buf[80], *crypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = getchar()) != '\n' && c != '\r' && c != '\033')
	if (c == _tty.sg_kill)
	    sp = buf;
	else if (c == _tty.sg_erase && sp > buf)
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, crypt(buf, "mT")) == 0);
}

/*
 * do_run:
 *	Set the running flag and direction character.
 *	Called from command() with a movement key (h,j,k,l,y,u,b,n).
 */
do_run(ch)
char ch;
{
    running = TRUE;
    after = FALSE;
    runch = ch;
}
