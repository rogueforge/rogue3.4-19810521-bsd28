/*
 * Contains functions for dealing with things
 *
 * @(#)things.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * new_level:
 *	Dig and draw a new level
 */
new_level()
{
    register int rm, i;
    register char ch;
    coord stairs;

    if (level > max_level)
	max_level = level;
    wclear(cw);
    wclear(mw);
    clear();
    status();
    /*
     * Free up the monsters on the last level
     */
    free_list(mlist);
    do_rooms();				/* Draw rooms */
    do_passages();			/* Draw passages */
    no_food++;
    put_things();			/* Place objects (if any) */
    /*
     * Place the staircase down.
     */
    rm = rnd_room();
    do {
	rnd_pos(&rooms[rm], &stairs);
    } until (winat(stairs.y, stairs.x) == FLOOR);
    addch(STAIRS);
    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
	ntraps = rnd(level/4)+1;
	if (ntraps > MAXTRAPS)
	    ntraps = MAXTRAPS;
	i = ntraps;
	while (i--)
	{
	    do
	    {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &stairs);
	    } until (winat(stairs.y, stairs.x) == FLOOR);
	    switch(rnd(6))
	    {
		when 0: ch = TRAPDOOR;
		when 1: ch = BEARTRAP;
		when 2: ch = SLEEPTRAP;
		when 3: ch = ARROWTRAP;
		when 4: ch = TELTRAP;
		when 5: ch = DARTTRAP;
	    }
	    addch(TRAP);
	    traps[i].tr_type = ch;
	    traps[i].tr_flags = 0;
	    traps[i].tr_pos = stairs;
	}
    }
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    }
    until(winat(hero.y, hero.x) == FLOOR);
    light(&hero);
    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
}

/*
 * Pick a room that is really there
 */
rnd_room()
{
    register int rm;

    do
    {
	rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * inv_name:
 *	return the name of something as it would appear in an
 *	inventory.
 */
char *
inv_name(obj, drop)
register struct object *obj;
register bool drop;
{
    register char *pb;

    switch(obj->o_type)
    {
	when SCROLL:
	    if (s_know[obj->o_which])
		sprintf(prbuf, "A scroll of %s", s_magic[obj->o_which].mi_name);
	    else if (s_guess[obj->o_which])
		sprintf(prbuf, "A scroll called %s", s_guess[obj->o_which]);
	    else
		sprintf(prbuf, "A scroll titled '%s'", s_names[obj->o_which]);
	when POTION:
	    if (p_know[obj->o_which])
		sprintf(prbuf, "A potion of %s(%s)",
		    p_magic[obj->o_which].mi_name,
		    p_colors[obj->o_which]);
	    else if (p_guess[obj->o_which])
		sprintf(prbuf, "A potion called %s",
		    p_guess[obj->o_which]);
	    else
		sprintf(prbuf, "A %s potion",
		    p_colors[obj->o_which]);
	when FOOD:
	    if (obj->o_which == 2)
		sprintf(prbuf, "A single %s", fruit);
	    else
		strcpy(prbuf, "Some food");
	when WEAPON:
	    if (obj->o_flags & ISKNOW)
		sprintf(prbuf, "A %s %s", num(obj->o_hplus, obj->o_dplus),
		    w_names[obj->o_which]);
	    else
		sprintf(prbuf, "A %s", w_names[obj->o_which]);
	when ARMOR:
	    if (obj->o_flags & ISKNOW)
		sprintf(prbuf, "%s %s",
		    num(a_class[obj->o_which] - obj->o_ac, 0),
		    a_names[obj->o_which]);
	    else
		sprintf(prbuf, "%s", a_names[obj->o_which]);
	when AMULET:
	    strcpy(prbuf, "The Amulet of Yendor");
	otherwise:
	    debug("Picked up something funny");
	    sprintf(prbuf, "Something bizarre %s", unctrl(obj->o_type));
    }
    if (obj == cur_armor)
	strcat(prbuf, " (being worn)");
    if (obj == cur_weapon)
	strcat(prbuf, " (weapon in hand)");
    if (drop && isupper(prbuf[0]))
	prbuf[0] = tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    if (!drop)
	strcat(prbuf, ".");
    return prbuf;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
    switch(ch)
    {
	case GOLD:
	    money();
	    break;
	default:
	    debug("Where did you pick that up???");
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:
	case AMULET:
	    add_pack();
	    break;
    }
}

/*
 * money:
 *	Add to characters purse
 */
money()
{
    register int i;

    for (i = 0; i < MAXROOMS; i++)
	if (ce(hero, rooms[i].r_gold))
	{
	    if (notify)
		msg("%s%d gold pieces.", terse ? "" : "You found ", rooms[i].r_goldval);
	    purse += rooms[i].r_goldval;
	    rooms[i].r_goldval = 0;
	    cmov(rooms[i].r_gold);
	    addch(FLOOR);
	    return;
	}
    msg("That gold must have been counterfeit");
}

/*
 * add_pack:
 *	Pick up an object from the floor and add it to the pack.
 */
add_pack()
{
    register struct linked_list *item, *ip, *lp;
    register struct object *obj, *op;
    register char ch;
    register struct thing *tp;

    /*
     * Check if there is room
     */
    if (inpack == MAXPACK)
    {
	msg("You can't carry anything else.");
	return;
    }
    if ((item = find_obj(hero.y, hero.x)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    inpack++;
    detach(lvl_obj, item);
    mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    /*
     * Search for an object of the same type to find insertion point
     */
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL)
    {
	/*
	 */
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else
    {
	/*
	 */
	while (ip != NULL && op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
		break;
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    /*
     */
    if (ip == NULL)
    {
	if (pack == NULL)
	    pack = item;
	else
	{
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else
    {
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
    /*
     * Notify the user
     */
    ch = 'a';
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	if (ip == item)
	    break;
	ch++;
    }
    obj = (struct object *) ldata(item);
    if (notify)
    {
	if (terse)
	    msg("%s (%c)", inv_name(obj, FALSE), ch);
	else
	    msg("You found %s (%c).", inv_name(obj, TRUE), ch);
    }
    if (obj->o_type == AMULET)
	amulet = FALSE;
}

/*
 * put_things:
 *	put things on this level
 */
put_things()
{
    register int i, j, k;
    register struct linked_list *item;
    register struct object *cur;
    register int rm;
    coord tp;

    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 35)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_item(sizeof *cur);
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    cur->o_hplus = cur->o_dplus = 0;
	    cur->o_damage = cur->o_hurldmg = "0d0";
	    cur->o_ac = 11;
	    /*
	     * Decide what kind of object it will be
	     * If we haven't had food for a while, let it be food.
	     */
	    switch (no_food > 3 ? 4 : rnd(10))
	    {
		when 0: case 1: case 5:
		    cur->o_type = POTION;
		    for (j = 0, k = rnd(100); j < MAXPOTIONS; j++)
			if (k < p_magic[j].mi_prob)
			    break;
		    if (j == MAXPOTIONS)
		    {
			j = 0;
			if (wizard)
			    msg("Picked bad potion %d", k);
		    }
		    cur->o_which = j;
		when 2: case 3: case 6:
		    cur->o_type = SCROLL;
		    for (j = 0, k = rnd(100); j < MAXSCROLLS; j++)
			if (k < s_magic[j].mi_prob)
			    break;
		    if (j == MAXSCROLLS)
		    {
			j = 0;
			if (wizard)
			    msg("Picked bad scroll %d", k);
		    }
		    cur->o_which = j;
		when 4: case 7:
		    no_food = 0;
		    cur->o_type = FOOD;
		    if (rnd(100) > 10)
			cur->o_which = 0;
		    else if (rnd(100) > 50)
			cur->o_which = 1;
		    else
			cur->o_which = 2;
		when 8:
		    cur->o_type = WEAPON;
		    cur->o_which = rnd(MAXWEAPONS);
		    init_weapon(cur, cur->o_which);
		    if ((k = rnd(100)) < 10)
		    {
			cur->o_flags |= ISCURSED;
			cur->o_hplus -= rnd(3)+1;
		    }
		    else if (k < 15)
			cur->o_hplus += rnd(3)+1;
		when 9:
		    cur->o_type = ARMOR;
		    for (j = 0, k = rnd(100); j < MAXARMORS; j++)
			if (k < a_chances[j])
			    break;
		    if (j == MAXARMORS)
		    {
			debug("Picked a bad armor %d", k);
			j = 0;
		    }
		    cur->o_which = j;
		    cur->o_ac = a_class[j];
		    if ((k = rnd(100)) < 20)
		    {
			cur->o_flags |= ISCURSED;
			cur->o_ac += rnd(3)+1;
		    }
		    else if (k < 28)
			cur->o_ac -= rnd(3)+1;
		otherwise:
		    debug("Picked a bad kind of object");
		    wait_for(' ');
	    }
	    /*
	     * Put it somewhere
	     */
	    rm = rnd_room();
	    do {
		rnd_pos(&rooms[rm], &tp);
	    } until (winat(tp.y, tp.x) == FLOOR);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground.
     */
    if (level > 25 && amulet)
    {
	item = new_item(sizeof *cur);
	attach(lvl_obj, item);
	cur = (struct object *) ldata(item);
	cur->o_hplus = cur->o_dplus = 0;
	cur->o_damage = cur->o_hurldmg = "0d0";
	cur->o_ac = 11;
	cur->o_type = AMULET;
	/*
	 * Put it somewhere
	 */
	rm = rnd_room();
	do {
	    rnd_pos(&rooms[rm], &tp);
	} until (winat(tp.y, tp.x) == FLOOR);
	mvaddch(tp.y, tp.x, cur->o_type);
	cur->o_pos = tp;
    }
}

/*
 * inventory:
 *	list what is in the pack
 */
inventory(list, type)
struct linked_list *list;
int type;
{
    register struct object *obj;
    register char ch;
    register int n_objs;
    char inv_temp[80];

    n_objs = 0;
    for (ch = 'a'; ch <= 'w' && list != NULL; list = next(list), ch++)
    {
	obj = (struct object *) ldata(list);
	if (type && type != obj->o_type)
		continue;
	switch (n_objs++)
	{
	    /*
	     * For the first thing in the inventory, just save the string
	     * in case there is only one.
	     */
	    case 0:
		sprintf(inv_temp, "%c) %s\n", ch, inv_name(obj, FALSE));
		break;
	    /*
	     * If there is more than one, clear the screen, print the
	     * saved message and fall through to ...
	     */
	    case 1:
		if (slow_invent)
		    msg(inv_temp);
		else
		{
		    wclear(hw);
		    waddstr(hw, inv_temp);
		}
	    /*
	     * Print the line for this object
	     */
	    default:
		if (slow_invent)
		    msg("%c) %s\n", ch, inv_name(obj, FALSE));
		else
		    wprintw(hw, "%c) %s\n", ch, inv_name(obj, FALSE));
	}
    }
    if (n_objs == 0)
    {
	if (terse)
	    if (type == 0)
		msg("Empty handed.");
	    else
		msg("Nothing appropriate");
	else if (type == 0)
	    msg("You are empty handed.");
	else
	    msg("You don't have anything appropriate");
	return;
    }
    if (n_objs == 1)
    {
	msg(inv_temp);
	return;
    }
    if (!slow_invent)
    {
	mvwaddstr(hw, 23, 0, "--Press space to continue--");
	draw(hw);
	wait_for(' ');
	clearok(cw, TRUE);
	touchwin(cw);
    }
}

/*
 * drop:
 *	put something down
 */
drop()
{
    register char ch;
    register struct linked_list *obj;
    register struct object *op;

    if ((obj = get_item("drop", 0)) == NULL)
	return;
    op = (struct object *) ldata(obj);
    if (op == cur_weapon)
    {
	if (op->o_flags & ISCURSED)
	{
	    msg("You can't drop it.  It appears to be cursed.");
	    return;
	}
	cur_weapon = NULL;
    }
    if (op == cur_armor)
    {
	if (op->o_flags & ISCURSED)
	{
	    msg("You can't drop it.  It appears to be cursed.");
	    return;
	}
	waste_time();
	cur_armor = NULL;
    }
    ch = mvwinch(stdscr, hero.y, hero.x);
    if (ch != FLOOR && ch != PASSAGE)
    {
	msg("There is something there already");
	return;
    }
    detach(pack, obj);
    inpack--;
    /*
     */
    if (op->o_type == SCROLL && op->o_which == S_SCARE)
    {
	if (op->o_flags & ISFOUND)
	{
	    msg("The scroll turns to dust as it hits the ground.");
	    return;
	}
	op->o_flags |= ISFOUND;
    }
    /*
     * Link it into the level object list
     */
    attach(lvl_obj, obj);
    mvaddch(hero.y, hero.x, op->o_type);
    op->o_pos = hero;
    msg("Dropped %s", inv_name(op, TRUE));
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */
struct linked_list *
find_obj(y, x)
int y;
int x;
{
    register struct linked_list *obj;
    register struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    debug(sprintf(prbuf, "Non-object %d,%d", y, x));
    return NULL;
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item(purpose, type)
char *purpose;
int type;
{
    register struct linked_list *obj;
    register char ch, och;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else
    {
	for (;;)
	{
	    if (!terse)
		msg("Which object do you want to %s? (* for a list): ", purpose);
	    else
		msg("%s what? (* for list): ", purpose);
	    ch = readchar();
	    mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == ESCAPE || ch == CTRL(G))
	    {
		after = FALSE;
		msg("");
		return NULL;
	    }
	    if (ch == '*')
	    {
		mpos = 0;
		inventory(pack, type);
		continue;
	    }
	    for (obj = pack, och = 'a'; obj != NULL; obj = next(obj), och++)
		if (ch == och)
		    break;
	    if (obj == NULL)
	    {
		msg("Please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else
		return obj;
	}
    }
    return NULL;
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
    register struct linked_list *item;
    register char ch, mch;

    if (pack == NULL)
	msg("You aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE));
    else
    {
	msg(terse ? "Item: " : "Which item do you wish to inventory: ");
	mpos = 0;
	mch = readchar();
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch)
	    {
		msg("%c) %s",ch,inv_name((struct object *) ldata(item), FALSE));
		return;
	    }
	msg("'%s' not in pack", unctrl(mch));
    }
}
