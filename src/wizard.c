/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certin object is
 */
whatis()
{
    register struct object *obj;
    register struct linked_list *item;

    if ((item = get_item("identify", 0)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type == SCROLL)
    {
	msg("A scroll of %s.", s_magic[obj->o_which].mi_name);
	s_know[obj->o_which] = TRUE;
	if (s_guess[obj->o_which])
	{
	    cfree(s_guess[obj->o_which]);
	    s_guess[obj->o_which] = NULL;
	}
	return;
    }
    else if (obj->o_type == POTION)
    {
	msg("A potion of %s.", p_magic[obj->o_which].mi_name);
	p_know[obj->o_which] = TRUE;
	if (p_guess[obj->o_which])
	{
	    cfree(p_guess[obj->o_which]);
	    p_guess[obj->o_which] = NULL;
	}
	return;
    }
    else if (obj->o_type == WEAPON)
    {
	msg("A %s %s", num(obj->o_hplus, obj->o_dplus),
	    w_names[obj->o_which]);
	obj->o_flags |= ISKNOW;
    }
    else if (obj->o_type == ARMOR)
    {
	msg("%s %s", num(a_class[obj->o_which] - obj->o_ac, 0),
	    a_names[obj->o_which]);
	obj->o_flags |= ISKNOW;
    }
}

/*
 * eat:
 *	She wants to eat something, so let her try
 */
eat()
{
    register struct linked_list *item;
    register struct object *obj;

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != FOOD)
    {
	if (!terse)
	    msg("Ugh, you would get ill if you ate that.");
	else
	    msg("That's Inedible!");
	return;
    }
    inpack--;
    detach(pack, item);
    discard(item);
    if (obj->o_which == 1)
    {
	msg("Yuk, this food tastes awful");
	pstats.s_exp++;
    }
    else
	msg("Yum, that tasted good");
    /*
     * Kill off any pending hunger fuses/daemons and restart
     */
    kill_daemon(faint);
    extinguish(weak);
    extinguish(starve);
    extinguish(hungry);
    fuse(hungry, 0, HUNGERTIME, AFTER);
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * create_obj:
 *	Wizard command for getting anything he wants
 */
create_obj()
{
    register struct linked_list *item;
    register struct object *obj;
    register char ch, bless;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    msg("Type of item: ");
    obj->o_type = readchar();
    mpos = 0;
    msg("Which %c do you want? (0-f)", obj->o_type);
    obj->o_which = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
    mpos = 0;
    msg("Blessing? (+,-,n)");
    bless = readchar();
    if (bless == '-')
	obj->o_flags |= ISCURSED;
    if (obj->o_type == WEAPON)
    {
	init_weapon(obj, obj->o_which);
	if (bless == '-')
	    obj->o_hplus -= rnd(3)+1;
	if (bless == '+')
	    obj->o_hplus += rnd(3)+1;
    }
    if (obj->o_type == ARMOR)
    {
	obj->o_ac = a_class[obj->o_which];
	if (bless == '-')
	    obj->o_ac += rnd(3)+1;
	if (bless == '+')
	    obj->o_ac -= rnd(3)+1;
    }
    attach(pack, item);
}

/*
 * wield:
 *	Pull out a certain weapon
 */
wield()
{
    register struct linked_list *item;
    register struct object *obj;

    if (cur_weapon != NULL && cur_weapon->o_flags & ISCURSED)
    {
	msg("You can't change weapons, the thing is cursed.");
	return;
    }
    if ((item = get_item("wield", WEAPON)) == NULL)
	return;

    obj = (struct object *) ldata(item);
    if (terse)
	msg("Wielding %s\n", inv_name(obj, TRUE));
    else
	msg("You are now wielding %s\n", inv_name(obj, TRUE));
    cur_weapon = obj;
}

/*
 * chg_str:
 *	Used to modify the player's strength.  It keeps track
 *	of the highest it has been, just in case.
 */
chg_str(amt)
int amt;
{
    if (amt == 0)
	return;
    if (amt > 0)
    {
	while (amt--)
	{
	    if (pstats.s_str.st_str < 18)
		pstats.s_str.st_str++;
	    else if (pstats.s_str.st_add == 0)
		pstats.s_str.st_add = rnd(50) + 1;
	    else if (pstats.s_str.st_add <= 50)
		pstats.s_str.st_add = 51 + rnd(24);
	    else if (pstats.s_str.st_add <= 75)
		pstats.s_str.st_add = 76 + rnd(14);
	    else if (pstats.s_str.st_add <= 90)
		pstats.s_str.st_add = 91;
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add++;
	}
	if (pstats.s_str.st_str > max_stats.s_str.st_str ||
	    (pstats.s_str.st_str == 18 &&
	     pstats.s_str.st_add > max_stats.s_str.st_add))
		max_stats.s_str = pstats.s_str;
    }
    else
    {
	while (amt++)
	{
	    if (pstats.s_str.st_str < 18 || pstats.s_str.st_add == 0)
		pstats.s_str.st_str--;
	    else if (pstats.s_str.st_add < 51)
		pstats.s_str.st_add = 0;
	    else if (pstats.s_str.st_add < 76)
		pstats.s_str.st_add = 1 + rnd(50);
	    else if (pstats.s_str.st_add < 91)
		pstats.s_str.st_add = 51 + rnd(25);
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add = 76 + rnd(14);
	    else
		pstats.s_str.st_add = 91 + rnd(8);
	}
	if (pstats.s_str.st_str < 3)
	    pstats.s_str.st_str = 3;
    }
}

/*
 * telport:
 *	Bamf the hero someplace else
 */
teleport()
{
    register int rm;
    coord c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
    }
    return rm;
}

