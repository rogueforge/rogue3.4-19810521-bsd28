#
# Makefile for rogue
# @(#)Makefile	X.XX (Berkeley) XX/XX/XX
#
HDRS=	rogue.h mach_dep.h
OBJS=	vers.o main.o rooms.o passages.o command.o move.o fight.o chase.o \
	init.o things.o potions.o wizard.o list.o io.o rip.o \
	daemon.o daemons.o weapons.o monsters.o save.o options.o armor.o
POBJS=	vers.po main.po rooms.po passages.po command.po move.po fight.po chase.po \
	init.po things.po potions.po wizard.po list.po io.po rip.po \
	daemon.po daemons.po weapons.po monsters.po save.po options.po armor.po
CFILES=	vers.c main.c rooms.c passages.c command.c move.c fight.c chase.c \
	init.c things.c potions.c wizard.c list.c io.c rip.c \
	daemon.c daemons.c weapons.c monsters.c save.c options.c armor.c
MISC=	Makefile

#DEFS=
CFLAGS=		-O $(DEFS)
PROFLAGS=	-p -O
LDFLAGS=-i	# For PDP-11's
#LDFLAGS=	# For VAXes
CRLIB=	-lcurses
PCRLIB= -lcurses

VGRIND=	/usr/ucb/vgrind
# for sites without sccs front end, GET= get
#GET=	sccs get
GET=	get

.SUFFIXES: .po

.DEFAULT:
	$(GET) $@

.c.po:
	@echo $(CC) -c $(PROFLAGS) $?
	@rm -f x.c
	@ln $? x.c
	@$(CC) -c $(PROFLAGS) x.c
	@mv x.o $*.po

a.out: $(HDRS) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(CRLIB) -ltermlib
	size a.out
	@echo -n 'You still have to remove '		# 11/70's
	@size a.out | sed 's/+.*/ 1024 63 * - p/' | dc	# 11/70's

p.out:	$(HDRS) $(POBJS)
	@rm -f x.c
	$(CC) $(PROFLAGS) $(LDFLAGS) $(POBJS) $(PCRLIB) -ltermlib -o p.out
	size p.out

main.o rip.o: mach_dep.h

rogue: newvers a.out
	cp a.out rogue
	strip rogue

install: rogue
	cp rogue         $(DESTDIR)/usr/games/rogue


newvers:
	$(GET) -e vers.c


tags: $(HDRS) $(CFILES)
	ctags -u $?
	ed - tags < :ctfix
	sort tags -o tags

lint:
	lint -hxbc $(CFILES) -lcurses > linterrs

clean:
	rm -f $(POBJS) $(OBJS) core a.out p.out rogue rogue.tar vgrind.* linterrs

xtar: $(HDRS) $(CFILES) $(MISC)
	rm -f rogue.tar
	tar cf rogue.tar $? :ctfix
	touch xtar

vgrind:
	@csh $(VGRIND) -t -h "Rogue Version 3.4" $(HDRS) *.c > vgrind.out
	@ctags -v $(HDRS) *.c > index
	@csh $(VGRIND) -t -x index > vgrind.out.tbl

wc:
	@echo "  bytes  words  lines  pages file"
	@wc -cwlp $(HDRS) $(CFILES)

cfiles: $(CFILES)
