/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include "rogue.h"

struct daemon_action {
    int (*d_func)();
    int d_arg;
    char d_type;
};

struct fuse_action {
    int (*d_func)();
    int d_arg;
    int d_time;
    char f_type;
};

/*
 * daemon:
 *	Start a daemon, takes a function.
 */
daemon(func, arg, type)
int (*func)(), arg, type;
{
    register struct linked_list *item;
    register struct daemon_action *dev;

    item = new_item(sizeof *dev);
    if (item == NULL)
	return;
    attach(d_list, item);
    dev = (struct daemon_action *) ldata(item);
    dev->d_func = func;
    dev->d_arg = arg;
    dev->d_type = type;
}

/*
 * kill_daemon:
 *	Remove a daemon from the list
 */
kill_daemon(func)
register int (*func)();
{
    register struct linked_list *item;
    register struct daemon_action *dev;

    for (item = d_list; item != NULL; item = next(item))
    {
	dev = (struct daemon_action *) ldata(item);
	if (dev->d_func == func)
	    break;
    }
    if (item != NULL)
    {
	detach(d_list, item);
	discard(item);
    }
}

/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */
do_daemons(flag)
register int flag;
{
    register struct linked_list *item;
    register struct daemon_action *dev;

    for (item = d_list; item != NULL; item = next(item))
    {
	dev = (struct daemon_action *) ldata(item);
	if (dev->d_type == flag)
	    (*dev->d_func)(dev->d_arg);
    }
}

/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */
fuse(func, arg, time, type)
int (*func)(), arg, time, type;
{
    register struct linked_list *item;
    register struct fuse_action *wire;

    item = new_item(sizeof *wire);
    if (item == NULL)
	return;
    attach(f_list, item);
    wire = (struct fuse_action *) ldata(item);
    wire->d_func = func;
    wire->d_arg = arg;
    wire->d_time = time;
    wire->f_type = type;
}

/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */
lengthen(func, xtime)
register int (*func)();
int xtime;
{
    register struct linked_list *item;
    register struct fuse_action *wire;

    for (item = f_list; item != NULL; item = next(item))
    {
	wire = (struct fuse_action *) ldata(item);
	if (func == wire->d_func)
	    break;
    }
    if (item != NULL)
	wire->d_time += xtime;
}

/*
 * extinguish:
 *	Put out a fuse
 */
extinguish(func)
register int (*func)();
{
    register struct linked_list *item;
    register struct fuse_action *wire;

    for (item = f_list; item != NULL; item = next(item))
    {
	wire = (struct fuse_action *) ldata(item);
	if (func == wire->d_func)
	    break;
    }
    if (item != NULL)
    {
	detach(f_list, item);
	discard(item);
    }
}

/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */
do_fuses(flag)
int flag;
{
    register struct linked_list *item, *nitem;
    register struct fuse_action *wire;

    for (item = f_list; item != NULL; item = nitem)
    {
	wire = (struct fuse_action *) ldata(item);
	nitem = next(item);
	if (flag == wire->f_type && --wire->d_time <= 0)
	{
	    int (*func)() = wire->d_func;
	    detach(f_list, item);
	    discard(item);
	    (*func)(wire->d_arg);
	}
    }
}

