/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include "rogue.h"

/*
 * List of monsters in rough order of vorpalness
 */
static char *lvl_mons =  "JKBHSEAOZGLCRQNYTWFIXUMVDP";
static char *wand_mons = "JKBHS AOZG CRQ Y W IXU V  ";

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */
randmonster()
{
    register int d;

    d = rnd(10) + level - 5;
    if (d < 1)
	d = rnd(5) + 1;
    if (d > 26)
	d = rnd(5) + 22;
    return lvl_mons[d-1];
}

/*
 * randwand:
 *	Pick a random wandering monster
 */
randwand()
{
    register int d;

    do
    {
	d = rnd(10) + level - 5;
	if (d < 1)
	    d = rnd(5) + 1;
	if (d > 26)
	    d = rnd(5) + 22;
    } while (wand_mons[d-1] == ' ');
    return lvl_mons[d-1];
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */
new_monster(item, type, cp)
struct linked_list *item;
char type;
register coord *cp;
{
    register struct thing *tp;
    register struct monster *mp;

    attach(mlist, item);
    tp = (struct thing *) ldata(item);
    tp->t_type = type;
    tp->t_pos = *cp;
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_type-'A'];
    tp->t_stats.s_hpt = roll(mp->m_stats.s_lvl, 8);
    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    tp->t_stats.s_dmg = mp->m_stats.s_dmg;
    tp->t_stats.s_exp = mp->m_stats.s_exp;
    tp->t_stats.s_str.st_str = 10;
    tp->t_flags = mp->m_flags;
    tp->t_pack = NULL;
    if (type == 'M')
    {
	char mch;

	switch (rnd(6))
	{
	    when 0: mch = GOLD;
	    when 1: mch = POTION;
	    when 2: mch = SCROLL;
	    when 3: mch = STAIRS;
	    when 4: mch = WEAPON;
	    when 5: mch = ARMOR;
	}
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */
wanderer()
{
    register int i, ch;
    register struct room *rp, *hr = roomin(&hero);
    register struct linked_list *item;
    register struct thing *tp;
    coord cp;

    item = new_item(sizeof *tp);
    do
    {
	i = rnd_room();
	if ((rp = &rooms[i]) == hr)
	    continue;
	rnd_pos(rp, &cp);
	if ((ch = mvwinch(stdscr, cp.y, cp.x)) == ERR)
	{
	    debug("Routine wanderer: mvwinch failed to %d,%d", cp.y, cp.x);
	    wait_for('\n');
	    return;
	}
    } until(step_ok(ch));
    new_monster(item, randwand(), &cp);
    tp = (struct thing *) ldata(item);
    tp->t_flags |= ISRUN;
    tp->t_pos = cp;
    tp->t_dest = &hero;
    if (wizard)
	msg("Started a wandering %s", monsters[tp->t_type-'A'].m_name);
}
