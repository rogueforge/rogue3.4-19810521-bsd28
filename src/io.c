/*
 * Various input/output functions
 *
 * @(#)io.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * msg:
 *	Display a message at the top of the screen.
 */
/*VARARGS1*/
msg(fmt, args)
char *fmt;
int args;
{
    FILE junk;
    char msgbuf[512];
    register char *ep;

    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    junk._flag = _IOWRT + _IOSTRG;
    junk._ptr = msgbuf;
    junk._cnt = 32767;
    _doprnt(fmt, &args, &junk);
    putc('\0', &junk);
    ep = msgbuf;
    while (*ep)
    {
	if (*ep == '\n')
	{
	    *ep = '\0';
	    ep--;
	}
	ep++;
    }
    strcpy(huh, msgbuf);
    if (mpos)
    {
	wmove(cw, 0, mpos);
	waddstr(cw, "--More--");
	draw(cw);
	wait_for(' ');
    }
    mvwaddstr(cw, 0, 0, msgbuf);
    wclrtoeol(cw);
    mpos = strlen(msgbuf);
    draw(cw);
}

/*
 * step_ok:
 *	returns true if it is ok to step on ch
 */
step_ok(ch)
{
    switch (ch)
    {
	case FLOOR:
	case PASSAGE:
	case DOOR:
	case STAIRS:
	case TRAP:
	case GOLD:
	case POTION:
	case SCROLL:
	case FOOD:
	case WEAPON:
	case ARMOR:
	case AMULET:
	    return TRUE;
	default:
	    return FALSE;
    }
}

/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */
readchar()
{
    char c;

    fflush(stdout);
    while (read(0, &c, 1) < 0)
	continue;
    return c;
}

/*
 * unctrl:
 *	Print a readable version of a certain character
 */
char *
unctrl(ch)
char ch;
{
    extern char *_unctrl[];		/* Defined in curses library */

    return _unctrl[ch&0177];
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */
status()
{
    register int oy, ox, temp;
    char buf[512];
    static int hpwidth = 0;
    static int s_lvl = -1, s_pur, s_hp = -1, s_str, s_add, s_ac;
    static int s_exp;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
	&& s_str == pstats.s_str.st_str && s_add == pstats.s_str.st_add
	&& s_lvl == level)
	    return;

    getyx(cw, oy, ox);
    if (s_hp != max_hp)
    {
	temp = s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    if (pstats.s_str.st_add == 0)
    {
	sprintf(buf,
	    "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %-2d  Ac: %-2d  Exp: %d/%ld",
	    level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp,
	    pstats.s_str.st_str,
	    cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
	    pstats.s_exp);
    }
    else
    {
	sprintf(buf,
	    "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %-2d/%-2d  Ac: %-2d  Exp: %d/%ld",
	    level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp,
	    pstats.s_str.st_str, pstats.s_str.st_add,
	    cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
	    pstats.s_exp);
    }
    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str.st_str;
    s_add = pstats.s_str.st_add;
    s_exp = pstats.s_exp;
    s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
    wmove(cw, LINES - 1, 0);
    wclrtoeol(cw);
    mvwaddstr(cw, LINES - 1, 0, buf);
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */
wait_for(ch)
register char ch;
{
    register char c;

    if (ch == '\n')
	while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
	while (readchar() != ch)
	    continue;
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */
show_win(scr, message)
register WINDOW *scr;
char *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    draw(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}
