/*
 * Function(s) for dealing with potions
 *
 * @(#)potions.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

quaff()
{
    register struct object *obj;
    register struct linked_list *item, *titem;
    char buf[80];

    item = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != POTION)
    {
	if (!terse)
	    msg("Yuk! Why would you want to drink that?");
	else
	    msg("That's undrinkable");
	return;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch(obj->o_which)
    {
	when P_CONFUSE:
	    if (off(player, ISHUH))
	    {
		msg("Wait, what's going on here. Huh? What? Who?");
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    p_know[P_CONFUSE] = TRUE;
	when P_POISON:
	    chg_str(-(rnd(3)+1));
	    msg("You feel very sick now.");
	    p_know[P_POISON] = TRUE;
	when P_HEALING:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel better.");
	    p_know[P_HEALING] = TRUE;
	when P_STRENGTH:
	    msg("You feel stronger, now.  What bulging muscles!");
	    chg_str(1);
	    p_know[P_STRENGTH] = TRUE;
	when P_MFIND:
	    /*
	     * Potion of monster detection, if there are monters, detect them
	     */
	    if (mlist != NULL)
	    {
		wclear(hw);
		overwrite(mw, hw);
		show_win(hw,
		    "You begin to sense the presence of monsters.--More--");
		p_know[P_MFIND] = TRUE;
	    }
	    else
		msg("You have a strange feeling for a moment, then it passes.");
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL)
	    {
		struct linked_list *mobj;
		struct object *tp;
		bool show;

		show = FALSE;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj))
		{
		    tp = (struct object *) ldata(mobj);
		    if (is_magic(tp))
		    {
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
		    }
		    p_know[P_TFIND] = TRUE;
		}
		if (show)
		{
		    show_win(hw,
			"You sense the presence of magic on this level.--More--");
		    break;
		}
	    }
	    msg("You have a strange feeling for a moment, then it passes.");
	when P_PARALYZE:
	    msg("You can't move.");
	    no_command = HOLDTIME;
	    p_know[P_PARALYZE] = TRUE;
	when P_SEEINVIS:
	    msg("This potion tastes like %s juice.", fruit);
	    if (off(player, CANSEE))
	    {
		player.t_flags |= CANSEE;
		fuse(unsee, 0, SEEDURATION, AFTER);
		light(&hero);
	    }
	when P_RAISE:
	    msg("You suddenly feel much more skillful");
	    p_know[P_RAISE] = TRUE;
	    raise_level();
	when P_XHEAL:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel much better.");
	    p_know[P_XHEAL] = TRUE;
	when P_HASTE:
	    if (on(player, ISHASTE))
	    {
		msg("You faint from exhaustion.");
		no_command += rnd(8);
		extinguish(nohaste);
		player.t_flags &= ~ISHASTE;
	    }
	    else
	    {
		msg("You feel yourself moving much faster.");
		player.t_flags |= ISHASTE;
		fuse(nohaste, 0, rnd(4)+4, AFTER);
	    }
	    p_know[P_HASTE] = TRUE;
	when P_RESTORE:
	    msg("Hey, this tastes great.  It make you feel warm all over.");
	    pstats.s_str = max_stats.s_str;
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }
    status();
    if (p_know[obj->o_which] && p_guess[obj->o_which])
    {
	cfree(p_guess[obj->o_which]);
	p_guess[obj->o_which] = NULL;
    }
    else if (!p_know[obj->o_which] && askme && p_guess[obj->o_which] == NULL)
    {
	msg(terse ? "Call it: " : "What do you want to call it? ");
	mpos = 0;
	if (get_str(buf, cw) == NORM)
	{
	    p_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
	    strcpy(p_guess[obj->o_which], buf);
	}
	msg("");
    }
    /*
     * Throw the item away
     */
    inpack--;
    detach(pack, item);
    discard(item);
}
read_scroll()
{
    register struct object *obj;
    register struct linked_list *item;
    register struct room *rp;
    register int i,j;
    register char ch, nch;
    char buf[80];

    item = get_item("read", SCROLL);
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != SCROLL)
    {
	if (!terse)
	    msg("There is nothing on it to read");
	else
	    msg("Nothing to read");
	return;
    }
    msg("As you read the scroll, it vanishes.");
    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
	cur_weapon = NULL;
    switch(obj->o_which)
    {
	when S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    msg("Your hands begin to glow red");
	    player.t_flags |= CANHUH;
	when S_LIGHT:
	    s_know[S_LIGHT] = TRUE;
	    if ((rp = roomin(&hero)) == NULL)
		msg("The corridor glows and then fades");
	    else
	    {
		msg("The room is lit%s", terse ? "" :
		    " by a shimmering blue light.");
		rp->r_flags &= ~ISDARK;
		/*
		 * Light the room and put the player back up
		 */
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when S_ARMOR:
	    if (cur_armor != NULL)
	    {
		msg("Your armor glows faintly for a moment");
		cur_armor->o_ac--;
		cur_armor->o_flags &= ~ISCURSED;
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */
	    {
		register int x,y;
		register struct linked_list *mon;

		for (x = hero.x-2; x <= hero.x+2; x++)
		    for (y = hero.y-2; y <= hero.y+2; y++)
			if (y > 0 && x > 0 && isupper(mvwinch(mw, y, x)))
			    if ((mon = find_mons(y, x)) != NULL)
			    {
				register struct thing *th;

				th = (struct thing *) ldata(mon);
				th->t_flags &= ~ISRUN;
				th->t_flags |= ISHELD;
			    }
	    }
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    msg("You fall asleep.");
	    no_command += 4 + rnd(SLEEPTIME);
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    {
		register int x, y;
		struct linked_list *titem;
		register bool appear = 0;
		coord mp;

		/*
		 * Search for an open place
		 */
		for (y = hero.y; y <= hero.y+1; y++)
		    for (x = hero.x; x <= hero.x+1; x++)
		    {
			/*
			 * Don't put a monster in top of the player.
			 */
			if (y == hero.y && x == hero.x)
			    continue;
			/*
			 * Or anything else nasty
			 */
			if (step_ok(winat(y, x)))
			{
			    if (rnd(++appear) == 0)
			    {
				mp.y = y;
				mp.x = x;
			    }
			}
		    }
		if (appear)
		{
		    titem = new_item(sizeof (struct thing));
		    new_monster(titem, randmonster(), &mp);
		}
		else
		    msg("You hear a faint cry of anguish in the distance.");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    msg("This scroll is an identify scroll");
	    s_know[S_IDENT] = TRUE;
	    whatis();
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    s_know[S_MAP] = TRUE;
	    msg("Oh, now this scroll has a map on it.");
	    overwrite(stdscr, hw);
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++)
		{
		    switch (nch = ch = mvwinch(hw, i, j))
		    {
			case SECRETDOOR:
			    mvaddch(i, j, nch = DOOR);
			case '-':
			case '|':
			case DOOR:
			case PASSAGE:
			case ' ':
			case STAIRS:
			    break;
			default:
			    nch = ' ';
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    /*
	     * Copy in what he has discovered
	     */
	    overlay(cw, hw);
	    /*
	     * And set up for display
	     */
	    overwrite(hw, cw);
	when S_GFIND:
	    /*
	     * Potion of gold detection
	     */
	    {
		int gtotal = 0;

		wclear(hw);
		for (i = 0; i < MAXROOMS; i++)
		{
		    gtotal += rooms[i].r_goldval;
		    if (rooms[i].r_goldval != 0 &&
			mvwinch(stdscr, rooms[i].r_gold.y, rooms[i].r_gold.x)
			== GOLD)
			mvwaddch(hw,rooms[i].r_gold.y,rooms[i].r_gold.x,GOLD);
		}
		if (gtotal)
		{
		    s_know[S_GFIND] = TRUE;
		    show_win(hw,
			"You begin to feel greedy and you sense gold.--More--");
		}
		else msg("You begin to feel a pull downward");
	    }
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		int rm;
		struct room *cur_room;

		cur_room = roomin(&hero);
		rm = teleport();
		if (cur_room != &rooms[rm])
		    s_know[S_TELEP] = TRUE;
	    }
	when S_ENCH:
	    if (cur_weapon == NULL)
		msg("You feel a strange sense of loss.");
	    else
	    {
		cur_weapon->o_flags &= ~ISCURSED;
		if (rnd(100) > 50)
		    cur_weapon->o_hplus++;
		else
		    cur_weapon->o_dplus++;
		msg("Your %s glows blue for a moment.", w_names[cur_weapon->o_which]);
	    }
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
	    msg("You hear maniacal laughter in the distance.");
	when S_REMOVE:
	    if (cur_armor != NULL)
		cur_armor->o_flags &= ~ISCURSED;
	    if (cur_weapon != NULL)
		cur_weapon->o_flags &= ~ISCURSED;
	    msg("You feel as if somebody is watching over you.");
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    msg("You hear a high pitched humming noise.");
	    {
		register struct linked_list *mi;

		for (mi = mlist; mi != NULL; mi = next(mi))
		{
		    register struct thing *mtp = (struct thing *) ldata(mi);
		    mtp->t_flags |= ISRUN;
		}
	    }
	otherwise:
	    msg("What a puzzling scroll!");
	    return;
    }
    look();	/* put the result of the scroll on the screen */
    status();
    if (s_know[obj->o_which] && s_guess[obj->o_which])
    {
	cfree(s_guess[obj->o_which]);
	s_guess[obj->o_which] = NULL;
    }
    else if (!s_know[obj->o_which] && askme && s_guess[obj->o_which] == NULL)
    {
	msg(terse ? "Call it: " : "What do you want to call it? ");
	mpos = 0;
	if (get_str(buf, cw) == NORM)
	{
	    s_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
	    strcpy(s_guess[obj->o_which], buf);
	}
	msg("");
    }
    /*
     * Get rid of the thing
     */
    inpack--;
    detach(pack, item);
    discard(item);
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */
char *
num(n1, n2)
int n1, n2;
{
    static char numbuf[80];

    if (n1 == 0)
	return "+0";
    if (n2 == 0)
	return sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    return sprintf(numbuf, "%s%d,%s%d",
			    n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2);
}
