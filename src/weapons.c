/*
 * Functions for dealing with problems brought about by weapons
 *
 * @(#)weapons.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

#define NONE 100

char *w_names[MAXWEAPONS] = {
    "mace",
    "long sword",
    "short bow",
    "single arrow",
    "dagger",
    "rock",
    "two handed sword",
    "sling",
    "dart",
    "crossbow",
    "crossbow bolt",
    "spear",
};

static struct init_weps {
    char *iw_dam;
    char *iw_hrl;
    char iw_launch;
    int iw_flags;
} init_dam[MAXWEAPONS] = {
    "2d4", "1d3", NONE, 0,		/* Mace */
    "1d10", "1d2", NONE,0,		/* Long sword */
    "1d1", "1d1", NONE,	0,		/* Bow */
    "1d1", "1d6", BOW,	ISMANY|ISMISL,	/* Arrow */
    "1d6", "1d4", NONE,	ISMISL,		/* Dagger */
    "1d2", "1d4", SLING,ISMANY|ISMISL,	/* Rock */
    "3d6", "1d2", NONE,	0,		/* 2h sword */
    "0d0", "0d0", NONE, 0,		/* Sling */
    "1d1", "1d3", NONE,	ISMANY|ISMISL,	/* Dart */
    "1d1", "1d1", NONE, 0,		/* Crossbow */
    "1d2", "1d10", CROSSBOW, ISMANY|ISMISL,/* Crossbow bolt */
    "1d8", "1d6", NONE, ISMISL,		/* Spear */
};

/*
 * missile:
 *	Fire a missile in a given direction
 */
missile(ydelta, xdelta)
int ydelta, xdelta;
{
    register struct linked_list *item;
    register struct object *obj;
    register int ch;

    /*
     * Get which thing we are hurling
     */
    if ((item = get_item("throw", WEAPON)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj == cur_weapon)
    {
	if (obj->o_flags & ISCURSED)
	{
	    msg("You can't throw it.%s", terse ? "" : "  It is cursed.");
	    return;
	}
    }
    if (obj == cur_armor)
    {
	if (obj->o_flags & ISCURSED)
	{
	    msg("You can't throw it.%s", terse ? "" : "  It is cursed.");
	    return;
	}
	waste_time();
	cur_armor = NULL;
    }
    if (on(player, ISHUH))
    {
	xdelta = rnd(3) - 1;
	ydelta = rnd(3) - 1;
    }
    detach(pack, item);
    inpack--;
    /*
     * Come fly with us ...
     */
    obj->o_pos = hero;
    for (;;)
    {
	/*
	 * Erase the old one
	 */
	if (!ce(obj->o_pos, hero) && cansee(unc(obj->o_pos)) &&
	    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
			    show(obj->o_pos.y, obj->o_pos.x));
	/*
	 * Get the new position
	 */
	obj->o_pos.y += ydelta;
	obj->o_pos.x += xdelta;
	if (step_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR)
	{
	    /*
	     * It hasn't hit anything yet, so display it
	     * If it alright.
	     */
	    if (cansee(unc(obj->o_pos)) &&
		mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
	    {
		mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type);
		draw(cw);
	    }
	    continue;
	}
	break;
    }
    if (isupper(ch) && hit_monster(obj->o_pos.y, obj->o_pos.x, obj))
	;
    else
	fall(item, TRUE);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    return;
}

/*
 * fall:
 *	Drop an item someplace around here.
 */
fall(item, pr)
struct linked_list *item;
bool pr;
{
    register struct object *obj;
    register int y, x;
    char ch;

    obj = (struct object *) ldata(item);
    /*
     * Try to find a place to put it.
     */
    for (y = obj->o_pos.y - 1; y <= obj->o_pos.y + 1; y++)
	for (x = obj->o_pos.x - 1; x <= obj->o_pos.x + 1; x++)
	{
	    if ((ch = winat(y, x)) == FLOOR || ch == PASSAGE)
	    {
		mvaddch(y, x, obj->o_type);
		obj->o_pos.y = y;
		obj->o_pos.x = x;
		light(&hero);
		attach(lvl_obj, item);
		return;
	    }
	}
    if (pr)
	msg("Your %s vanishes as it hits the ground.", w_names[obj->o_which]);
    discard(item);
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */
init_weapon(weap, type)
register struct object *weap;
char type;
{
    register struct init_weps *iwp;

    iwp = &init_dam[type];
    weap->o_damage = iwp->iw_dam;
    weap->o_hurldmg = iwp->iw_hrl;
    weap->o_launch = iwp->iw_launch;
    weap->o_flags = iwp->iw_flags;
    if (weap->o_flags & ISMANY)
    {
	weap->o_count = rnd(8) + 1;
    }
    else
	weap->o_count = 1;
}

/*
 * Does the missile hit the monster
 */
hit_monster(y, x, obj)
int y, x;
struct object *obj;
{
    coord mp;

    mp.y = y;
    mp.x = x;
    return fight(&mp, winat(y, x), obj, TRUE);
}
