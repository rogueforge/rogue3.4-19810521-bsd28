/*
 * Hero movement commands
 *
 * @(#)move.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * Used to hold the new hero position
 */

coord nh;

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */
do_move(dy, dx)
register int dy, dx;
{
    register char ch;

    if (no_move)
    {
	no_move--;
	msg("You are still stuck in the bear trap");
	return;
    }
    /*
     * Do a confused move (maybe)
     */
    if (rnd(100) < 80 && on(player, ISHUH))
	nh = *rndmove(&player);
    else
    {
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;
    }

    /*
     */
    if (nh.x < 0)
	nh.x = 0;
    else if (nh.x > COLS - 1)
	nh.x = COLS - 1;
    if (nh.y < 0)
	nh.y = 0;
    else if (nh.y > LINES - 1)
	nh.y = LINES - 1;
    diag_ok(&hero, &nh);
    if (running && ce(hero, nh))
	after = running = FALSE;
    ch = winat(nh.y, nh.x);
    if (on(player, ISHELD) && ch != 'F')
    {
	msg("You are being held");
	return;
    }
    switch(ch)
    {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    after = running = FALSE;
	    return;
	case TRAP:
	    ch = be_trapped(&nh);
	    if (ch == TRAPDOOR || ch == TELTRAP)
		return;
	    goto move_stuff;
	case GOLD:
	case POTION:
	case SCROLL:
	case FOOD:
	case WEAPON:
	case ARMOR:
	case AMULET:
	    running = FALSE;
	    take = ch;
	default:
move_stuff:
	    if (ch == PASSAGE && winat(hero.y, hero.x) == DOOR)
		light(&hero);
	    else if (ch == DOOR)
	    {
		running = FALSE;
		if (winat(hero.y, hero.x) == PASSAGE)
		    light(&nh);
	    }
	    else if (ch == STAIRS)
		running = FALSE;
	    else if (isupper(ch))
	    {
		running = FALSE;
		fight(&nh, ch, cur_weapon, FALSE);
		return;
	    }
	    ch = winat(hero.y, hero.x);
	    wmove(cw, unc(hero));
	    waddch(cw, ch);
	    hero = nh;
	    wmove(cw, unc(hero));
	    waddch(cw, PLAYER);
    }
}

/*
 * Called to illuminate a room.
 * If it is dark, remove anything that might move.
 */
light(cp)
coord *cp;
{
    register struct room *rp;
    register int j, k;
    register char ch, rch;

    fflush(stdout);
    if ((rp = roomin(cp)) == NULL)
	return;
    for (j = 0; j < rp->r_max.y; j++)
    {
	for (k = 0; k < rp->r_max.x; k++)
	{
	    ch = show(rp->r_pos.y + j, rp->r_pos.x + k);
	    wmove(cw, rp->r_pos.y + j, rp->r_pos.x + k);
	    /*
	     * Figure out how to display a secret door
	     */
	    if (ch == SECRETDOOR)
	    {
		if (j == 0 || j == rp->r_max.y - 1)
		    ch = '-';
		else
		    ch = '|';
	    }
	    /*
	     * If the room is a dark room, we might want to remove
	     * monsters and the like from it (since they might
	     * move)
	     */
	    if (rp->r_flags & ISDARK)
	    {
		rch = mvwinch(cw, rp->r_pos.y+j, rp->r_pos.x+k);
		switch (rch)
		{
		    when ' ':
		    case STAIRS:
		    case DOOR:
		    case '-':
		    case FLOOR:
		    case TRAP:
		    case '|':
			ch = rch;
		    otherwise:
			ch = ' ';
		}
	    }
	    waddch(cw, ch);
	}
    }
}

/*
 * show:
 *	returns what a certain thing will display as to the un-initiated
 */
show(y, x)
int y, x;
{
    register char ch = winat(y, x);
    register struct linked_list *it;
    register struct thing *tp;
    struct room *rer;
    coord tpos;

    tpos.y = y;
    tpos.x = x;
    rer = roomin(&tpos);
    if (ch == TRAP)
	return (traps[trap_at(y, x)].tr_flags & ISFOUND) ? TRAP : FLOOR;
    else if (isupper(ch))
    {
	it = find_mons(y, x);
	if (it == NULL)
	    msg("Can't find monster in show");
	tp = (struct thing *) ldata(it);
	/*
	 */
	if (rnd(100) > 33 && on(*tp, ISMEAN) && off(*tp, ISHELD))
	{
	    tp->t_dest = &hero;
	    tp->t_flags |= ISRUN;
	}
	switch (ch)
	{
	    when 'M':
		ch = tp->t_disguise;
	    when 'U':
		if (rer != NULL && (!(rer->r_flags & ISDARK)
		    || DISTANCE(y, x, hero.y, hero.x) < 3))
		{
		    if (off(*tp, ISFOUND) && !save_throw(VS_MAGIC))
		    {
			msg("The umber hulk's gaze has confused you.");
			if (on(player, ISHUH))
			    lengthen(unconfuse, HUHDURATION + rnd(20));
			else
			    fuse(unconfuse, 0, HUHDURATION + rnd(20), AFTER);
			player.t_flags |= ISHUH;
		    }
		    tp->t_flags |= ISFOUND;
		}
	}
	/*
	 * Hide invisible monsters
	 */
	if (on(*tp, ISINVIS) && off(player, CANSEE))
	    ch = mvwinch(stdscr, y, x);
	/*
	 * Let greedy ones guard gold
	 */
	if (on(*tp, ISGREED) && off(*tp, ISRUN))
	{
	    if (rer != NULL && rer->r_goldval)
	    {
		tp->t_dest = &rer->r_gold;
		tp->t_flags |= ISRUN;
	    }
	}
    }
    return ch;
}

/*
 * be_trapped:
 *	The guy stepped on a trap.... Make him pay.
 */
be_trapped(tc)
register coord *tc;
{
    register int i;
    register char ch;

    i = trap_at(tc->y, tc->x);
    count = running = FALSE;
    mvaddch(traps[i].tr_pos.y, traps[i].tr_pos.x, FLOOR);
    switch (ch = traps[i].tr_type)
    {
	when TRAPDOOR:
	    level++;
	    new_level();
	    msg("You fell into a trap!");
	when BEARTRAP:
	    no_move += BEARTIME;
	    msg("You are caught in a bear trap");
	when SLEEPTRAP:
	    no_command += SLEEPTIME;
	    msg("A strange white mist envelops you and you fall asleep");
	when ARROWTRAP:
	    if (swing(pstats.s_lvl-1, pstats.s_arm, 1))
	    {
		msg("Oh no! An arrow shot you");
		if ((pstats.s_hpt -= roll(1, 6)) <= 0)
		{
		    msg("The arrow killed you.");
		    death('a');
		}
	    }
	    else
	    {
		register struct linked_list *item;
		register struct object *arrow;

		msg("An arrow shoots past you.");
		item = new_item(sizeof *arrow);
		arrow = (struct object *) ldata(item);
		arrow->o_type = WEAPON;
		arrow->o_which = ARROW;
		init_weapon(arrow, ARROW);
		arrow->o_pos = hero;
		fall(item, FALSE);
	    }
	when TELTRAP:
	    teleport();
	when DARTTRAP:
	    if (swing(pstats.s_lvl+1, pstats.s_arm, 1))
	    {
		msg("A small dart just hit you in the shoulder");
		if ((pstats.s_hpt -= roll(1, 4)) <= 0)
		{
		    msg("The dart killed you.");
		    death('d');
		}
		chg_str(-1);
	    }
	    else
		msg("A small dart whizzes by your ear and vanishes.");
    }
    return(ch);
}

/*
 * trap_at:
 *	find the trap at (y,x) on screen.
 */
trap_at(y, x)
register int y, x;
{
    register int i;

    for (i = 0; i < ntraps; i++)
	if (traps[i].tr_pos.y == y && traps[i].tr_pos.x == x)
	    break;
    if (i == ntraps)
	debug(sprintf(prbuf, "Trap at %d,%d not in array", y, x));
    return i;
}

/*
 * rndmove:
 *	move in a random direction if the monster/person is confused
 */
coord *
rndmove(who)
struct thing *who;
{
    register int x, y;
    register char ch;
    register int ex, ey, nopen = 0;
    register struct linked_list *item;
    register struct object *obj;
    static coord ret;  /* what we will be returning */

    ret = who->t_pos;
    /*
     * Now go through the spaces surrounding the player and
     * set that place in the array to true if the space can be
     * moved into
     */
    ey = ret.y + 1;
    ex = ret.x + 1;
    for (y = who->t_pos.y - 1; y <= ey; y++)
	if (y >= 0 && y < LINES)
	    for (x = who->t_pos.x - 1; x <= ex; x++)
	    {
		if (x < 0 || x >= COLS)
		    continue;
		ch = winat(y, x);
		if (step_ok(ch))
		{
		    if (ch == SCROLL)
		    {
			item = NULL;
			for (item = lvl_obj; item != NULL; item = next(item))
			{
			    obj = (struct object *) ldata(item);
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (item != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    if (rnd(++nopen) == 0)
		    {
			ret.y = y;
			ret.x = x;
		    }
		}
	    }
    return &ret;
}
