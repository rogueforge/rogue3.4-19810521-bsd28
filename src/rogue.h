/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	X.XX (Berkeley) XX/XX/XX
 */

/*
 * Maximum number of different things
 */
#define MAXROOMS 9
#define MAXTHINGS 9
#define MAXOBJ 9
#define MAXPACK 22
#define MAXTRAPS 10
#define	NUMTHINGS 5	/* number of types of things (potion, scroll, food, weapon, armor) */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/*
 * All the fun defines
 */
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (rp)->r_pos.x <= (cp)->x && (cp)->x <= (rp)->r_pos.x + (rp)->r_max.x - 1 \
 && (rp)->r_pos.y <= (cp)->y && (cp)->y <= (rp)->r_pos.y + (rp)->r_max.y - 1)
#define winat(y, x) (mvwinch(mw,y,x)==' '?mvwinch(stdscr,y,x):winch(mw))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define pack player.t_pack
#define attach(a,b) _attach(&a,b)
#define detach(a,b) _detach(&a,b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define on(thing, flag) (((thing).t_flags & flag) != 0)
#define off(thing, flag) (((thing).t_flags & flag) == 0)
#define CTRL(ch) ('ch' & 037)
#define ALLOC(x) calloc(1, (unsigned int) x)
#define FREE(x) cfree((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISMULT(type) (type == POTION || type == SCROLL || type == FOOD)

/*
 * Things that appear on the screens
 */
#define PASSAGE '#'
#define DOOR '+'
#define FLOOR '.'
#define PLAYER '@'
#define TRAP '^'
#define TRAPDOOR '>'
#define ARROWTRAP '{'
#define SLEEPTRAP '$'
#define BEARTRAP '}'
#define TELTRAP '~'
#define DARTTRAP '`'
#define SECRETDOOR '&'
#define STAIRS '%'
#define GOLD '*'
#define POTION '!'
#define SCROLL '?'
#define MAGIC '$'
#define FOOD ':'
#define WEAPON ')'
#define ARMOR ']'
#define AMULET ','
#define CALLABLE -1

/*
 * Various constants
 */
#define	PASSWD "mTXoY2LoFzRmY"
#define BEARTIME 3
#define SLEEPTIME 5
#define HEALTIME 30
#define HOLDTIME 2
#define STPOS 0
#define WANDERTIME 70
#define BEFORE 1
#define AFTER 2
#define HUHDURATION 20
#define SEEDURATION 150
#define HUNGERTIME 1000
#define MORETIME 150
#define ESCAPE 27

/*
 * Save against things
 */
#define VS_POISON 00
#define VS_PARALYZATION 00
#define VS_DEATH 00
#define VS_PETRIFICATION 01
#define VS_BREATH 02
#define VS_MAGIC  03

/*
 * Various flag bits
 */
#define ISDARK	0000001
#define ISCURSED 000001
#define ISBLIND 0000001
#define ISGONE	0000002
#define ISKNOW  0000002
#define ISRUN	0000004
#define ISFOUND 0000010
#define ISINVIS 0000020
#define ISMEAN  0000040
#define ISGREED 0000100
#define ISBLOCK 0000200
#define ISHELD  0000400
#define ISHUH   0001000
#define ISREGEN 0002000
#define CANHUH  0004000
#define CANSEE  0010000
#define ISMISL  0020000
#define ISCANC	0020000
#define ISMANY  0040000
#define ISSLOW	0040000
#define ISHASTE 0100000

/*
 * Potion types
 */
#define P_CONFUSE 0
#define P_PARALYZE 1
#define P_POISON 2
#define P_STRENGTH 3
#define P_SEEINVIS 4
#define P_HEALING 5
#define P_MFIND 6
#define P_TFIND 7
#define P_RAISE 8
#define P_XHEAL 9
#define P_HASTE 10
#define P_RESTORE 11
#define MAXPOTIONS 12

/*
 * Scroll types
 */
#define S_CONFUSE 0
#define S_MAP 1
#define S_LIGHT 2
#define S_HOLD 3
#define S_SLEEP 4
#define S_ARMOR 5
#define S_IDENT 6
#define S_SCARE 7
#define S_GFIND 8
#define S_TELEP 9
#define S_ENCH 10
#define S_CREATE 11
#define S_REMOVE 12
#define S_AGGR 13
#define MAXSCROLLS 14

/*
 * Weapon types
 */
#define MACE 0
#define SWORD 1
#define BOW 2
#define ARROW 3
#define DAGGER 4
#define ROCK 5
#define TWOSWORD 6
#define SLING 7
#define DART 8
#define CROSSBOW 9
#define BOLT 10
#define SPEAR 11
#define MAXWEAPONS 12

/*
 * Armor types
 */
#define LEATHER 0
#define RING_MAIL 1
#define STUDDED_LEATHER 2
#define SCALE_MAIL 3
#define CHAIN_MAIL 4
#define SPLINT_MAIL 5
#define BANDED_MAIL 6
#define PLATE_MAIL 7
#define MAXARMORS 8

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */

struct h_list {
    char h_ch;
    char *h_desc;
} helpstr[];

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

typedef struct {
    short st_str;
    short st_add;
} str_t;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    int r_flags;			/* Info about the room */
    int r_nexits;			/* Number of exits */
    coord r_exit[4];			/* Where the exits are */
};

/*
 * Array of all traps on this level
 */
struct trap {
    coord tr_pos;			/* Where trap is */
    char tr_type;			/* What kind of trap */
    int tr_flags;			/* Info about trap (i.e. ISFOUND) */
} traps[MAXTRAPS];

/*
 * Structure describing a fighting being
 */
struct stats {
    str_t s_str;			/* Strength */
    long s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    coord t_pos;			/* Position */
    char t_type;			/* What it is */
    char t_disguise;			/* What mimic looks like */
    coord *t_dest;			/* Where it is running to */
    short t_flags;			/* State word */
    struct stats t_stats;		/* Physical description */
    struct linked_list *t_pack;		/* What the thing is carrying */
};

/*
 * Array containing information on all the various types of mosnters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    short m_flags;			/* Things about the monster */
    struct stats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char *o_text;			/* What it says if you read it */
    char o_launch;			/* What you need to launch it */
    char *o_damage;			/* Damage if used like sword */
    char *o_hurldmg;			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    int o_flags;			/* Information about objects */
};

/*
 * Now all the global variables
 */

struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct linked_list *mlist;		/* List of monsters on the level */
struct linked_list *f_list;		/* List of active fuses */
struct linked_list *d_list;	/* List of active daemons */
struct thing player;			/* The rogue */
struct stats max_stats;			/* The maximum for the player */
struct monster monsters[26];		/* The initial monster states */
struct linked_list *lvl_obj;		/* List of objects on this level */
struct object *cur_weapon;		/* Which weapon he is weilding */
struct object *cur_armor;		/* What a well dresssed rogue wears */
struct magic_item s_magic[MAXSCROLLS];	/* Names and chances for scrolls */
struct magic_item p_magic[MAXPOTIONS];	/* Names and chances for potions */

int level;				/* What level rogue is on */
int purse;				/* How much gold the rogue has */
int mpos;				/* Where cursor is on top line */
int ntraps;				/* Number of traps on this level */
int no_move;				/* Number of turns held in place */
int no_command;				/* Number of turns asleep */
int inpack;				/* Number of things in pack */
int max_hp;				/* Player's max hit points */
int a_chances[MAXARMORS];		/* Probabilities for armor */
int a_class[MAXARMORS];			/* Armor class for various armors */
int lastscore;				/* Score before this turn */
int total;				/* Total dynamic memory chunks */
int no_food;				/* Number of levels without food */
int seed;				/* Random number seed */
int dnum;				/* Dungeon number (display copy of seed) */
int count;				/* Number of times to repeat command */
int fung_hit;				/* Number of time fungi has hit */
int quiet;				/* Number of quiet turns */
int max_level;				/* Deepest player has gone */

char take;				/* Thing the rogue is taking */
char prbuf[80];				/* Buffer for sprintfs */
char outbuf[BUFSIZ];			/* Output buffer for stdout */
char runch;				/* Direction player is running */
char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char *w_names[MAXWEAPONS];		/* Names of the various weapons */
char *a_names[MAXARMORS];		/* Names of armor types */
char *release;				/* Release number of rogue */
char whoami[80];			/* Name of player */
char fruit[80];				/* Favorite fruit */
char huh[80];				/* The last message printed */
char *s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* Players guess at what potion is */
char file_name[80];			/* Save file name */
char home[80];				/* User's home directory */

WINDOW *cw;				/* Window that the player sees */
WINDOW *hw;				/* Used for the help command */
WINDOW *mw;				/* Used to store mosnters */

bool running;				/* True if player is running */
bool playing;				/* True until he quits */
bool wizard;				/* True if allows wizard commands */
bool after;				/* True if we want after daemons */
bool notify;				/* True if player wants to know */
bool fight_flush;			/* True if toilet input */
bool terse;				/* True if we should be short */
bool door_stop;				/* Stop running when we pass a door */
bool jump;				/* Show running as series of jumps */
bool slow_invent;			/* Inventory one line at a time */
bool throwing;				/* Throw direction dispatch flag */
bool firstmove;				/* First move after setting door_stop */
bool waswizard;				/* Was a wizard sometime */
bool askme;				/* Ask about unidentified things */
bool s_know[MAXSCROLLS];		/* Does he know what a scroll does */
bool p_know[MAXPOTIONS];		/* Does he know what a potion does */
bool amulet;				/* He found the amulet */
bool in_shell;				/* True if executing a shell */

coord oldpos;				/* Position before last look() call */

struct linked_list *find_mons(), *find_obj(), *get_item(), *new_item();

char *malloc(), *getenv(), *unctrl(), *tr_name(), *new(), *sprintf();
char *inv_name(), *strcpy(), *strcat(), *sbrk();
char *ctime(), *num();

struct room *roomin();

coord *rndmove();

int auto_save(), endit(), nohaste(), doctor(), runners(), swander();
int unconfuse(), unsee(), rollwand(), hungry(), weak(), starve(), faint();

#ifdef CHECKTIME
int checkout();
#endif

long lseek();

int trap_at();
