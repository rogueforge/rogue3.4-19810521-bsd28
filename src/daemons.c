/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include "rogue.h"

Decho(arg)
int arg;
{
    msg("Decho %d", arg);
}

/*
 * doctor:
 *	A healing daemon that restors hit points after rest
 */
doctor()
{
    register int lv, ohp;

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    quiet++;
    if (lv < 8)
    {
	if (quiet > 20 - lv*2)
	    pstats.s_hpt++;
    }
    else
	if (quiet >= 3)
	    pstats.s_hpt += rnd(lv - 7)+1;
    if (ohp != pstats.s_hpt)
    {
	if (pstats.s_hpt > max_hp)
	    pstats.s_hpt = max_hp;
	quiet = 0;
    }
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
swander()
{
    daemon(rollwand, 0, BEFORE);
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */
rollwand()
{
    static int between = 0;

    if (++between >= 4)
    {
	if (roll(1, 6) == 4)
	{
	    wanderer();
	    kill_daemon(rollwand);
	    fuse(swander, 0, WANDERTIME, BEFORE);
	}
	between = 0;
    }
}

/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
unconfuse()
{
    player.t_flags &= ~ISHUH;
    msg("You feel less confused now");
}


/*
 * unsee:
 *	He lost his see invisible power
 */
unsee()
{
    player.t_flags &= ~CANSEE;
}

/*
 * hungry:
 *	The hero is getting hungry — first stage of the hunger chain.
 *	Announces hunger and sets up the next fuse (weak).
 */
hungry()
{
    if (!terse)
	msg("You are starting to get hungry");
    else
	msg("Getting hungry");
    fuse(weak, 0, MORETIME, AFTER);
}

/*
 * weak:
 *	The hero is getting weak from hunger — second stage.
 *	Announces weakness and sets up the next fuse (starve).
 */
weak()
{
    if (!terse)
	msg("You are so hungry, you are starting to feel weak");
    else
	msg("You are weak from hunger");
    fuse(starve, 0, MORETIME, AFTER);
}

/*
 * starve:
 *	The hero is about to start fainting — third stage.
 *	Starts the faint daemon.
 */
starve()
{
    daemon(faint, 0, AFTER);
}

/*
 * faint:
 *	The hero is fainting — runs each turn and randomly
 *	causes the hero to pass out.
 */
faint()
{
    if (no_command == 0 && rnd(100) <= 20)
    {
	no_command = rnd(8) + 4;
	if (!terse)
	    msg("You feel so weak from lack of food.  You faint.");
	else
	    msg("You faint");
	running = FALSE;
	count = 0;
    }
}

/*
 * nohaste:
 *	End the hasting
 */
nohaste()
{
    player.t_flags &= ~ISHASTE;
    msg("You feel yourself slowing down.");
}
