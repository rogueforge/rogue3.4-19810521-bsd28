/*
 * Draw the connecting passages
 *
 * @(#)passages.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include "rogue.h"

/*
 */

#define NCONN 15

static char *conn_data[NCONN] = {
    "0r1r1d2d3d3r4d5d6r7r",
    "0r0d2d3r4r4d5d6r",
    "0r1r2d3d3r4r5d7r",
    "0r1d1r2d3d4d6r7r",
    "0r1d1r2d3r5d6r7r",
    "0d1d2d3d3r4d4r5d",
    "0r1d2d3d3r4r6r7r",
    "0d0r1r2d3d4d4r5d",
    "0d0r1r2d3r4d4r5d6r",
    "0r1r2d3d4d5d6r7r",
    "0d0r1d1r2d3d3r4d4r5d6r7r",
    "0r1d1r2d3d3r4d4r7r",
    "0r1d2d3d4d4r5d6r",
    "0d0r1d1r2d4d5d6r",
    "0d0r1d1r4r5d6r7r",
};

/*
 * do_passages:
 *	Draw all the passages on a level.
 */
do_passages()
{
    register char *cp;

    cp = conn_data[rnd(NCONN)];
    while (*cp)
    {
	conn(*cp, cp[1]);
	cp += 2;
    }
}

/*
 * conn:
 *	Draw a corridor from a room in a certain direction.
 */
conn(rm_ch, direc)
char rm_ch;
char direc;
{
    register struct room *rpf, *rpt;
    register char rmt;
    register int distance, turn_spot, turn_distance;
    coord delta, curr, turn_delta, spos, epos;

    rm_ch -= '0';
    rpf = &rooms[rm_ch];
    /*
     * Set up the movement variables, in two cases:
     * first drawing one down.
     */
    if (direc == 'd')
    {
	rmt = rm_ch + 3;				/* room # of dest */
	rpt = &rooms[rmt];			/* room pointer of dest */
	delta.x = 0;				/* direction of move */
	delta.y = 1;
	spos.x = rpf->r_pos.x;			/* start of move */
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;			/* end of move */
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))		/* if not gone pick door pos */
	{
	    spos.x += rnd(rpf->r_max.x-2)+1;
	    spos.y += rpf->r_max.y-1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.x += rnd(rpt->r_max.x-2)+1;
	distance = abs(spos.y - epos.y) - 1;	/* distance to move */
	turn_delta.y = 0;			/* direction to turn */
	turn_delta.x = (spos.x < epos.x ? 1 : -1);
	turn_distance = abs(spos.x - epos.x);	/* how far to turn */
	turn_spot = rnd(distance-1) + 1;		/* where turn starts */
    }
    else if (direc == 'r')			/* setup for moving right */
    {
	rmt = rm_ch + 1;
	rpt = &rooms[rmt];
	delta.x = 1;
	delta.y = 0;
	spos.x = rpf->r_pos.x;
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))
	{
	    spos.x += rpf->r_max.x-1;
	    spos.y += rnd(rpf->r_max.y-2)+1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.y += rnd(rpt->r_max.y-2)+1;
	distance = abs(spos.x - epos.x) - 1;
	turn_delta.y = (spos.y < epos.y ? 1 : -1);
	turn_delta.x = 0;
	turn_distance = abs(spos.y - epos.y);
	turn_spot = rnd(distance-1) + 1;
    }
    else
	debug("error in connection tables");
    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE)) door(rpf, &spos);
    else
    {
	cmov(spos);
	addch('#');
    }
    if (!(rpt->r_flags & ISGONE)) door(rpt, &epos);
    else
    {
	cmov(epos);
	addch('#');
    }
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while(distance)
    {
	/*
	 * Move to new position
	 */
	curr.x += delta.x;
	curr.y += delta.y;
	/*
	 * Check if we are at the turn place, if so do the turn
	 */
	if (distance == turn_spot && turn_distance > 0)
	    while(turn_distance--)
	    {
		cmov(curr);
		addch(PASSAGE);
		curr.x += turn_delta.x;
		curr.y += turn_delta.y;
	    }
	/*
	 * Continue digging along
	 */
	cmov(curr);
	addch(PASSAGE);
	distance--;
    }
    curr.x += delta.x;
    curr.y += delta.y;
    if (!ce(curr, epos))
	msg("Warning, connectivity problem on this level.");
}

/*
 * Add a door or possibly a secret door
 * also enters the door in the exits array of the room.
 */
door(rm, cp)
register struct room *rm;
register coord *cp;
{
    cmov(*cp);
    addch(rnd(10) < level - 1 && rnd(100) < 20 ? SECRETDOOR : DOOR);
    rm->r_exit[rm->r_nexits++] = *cp;
}

