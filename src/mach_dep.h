/*
 * machine dependicies
 *
 * @(#)mach_dep.h	X.XX (Berkeley) XX/XX/XX
 */
/*
 * where scorefile should live
 */
#define SCOREFILE	"/usr/public/.rogue_roll"

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#define	MAXUSERS	25	/* max number of users for this game */
#define	MAXLOAD		55	/* 10 * max 15 minute load average */

#if MAXUSERS|MAXLOAD
#define	CHECKTIME	15	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
#endif

