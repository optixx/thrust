# Simplified build for SDL-based Thrust on macOS.

SHELL        = /bin/sh
INSTALL      = /usr/bin/install -c
DD           = dd
CC ?= gcc
SDLCONFIG ?= sdl-config
SDL_CFLAGS := $(shell $(SDLCONFIG) --cflags 2>/dev/null)
SDL_LIBS := $(shell $(SDLCONFIG) --libs 2>/dev/null)
ifeq ($(strip $(SDL_LIBS)),)
SDL_LIBS = -lSDL
endif
SDL_INCLUDE_FLAG := $(firstword $(filter -I%, $(SDL_CFLAGS)))
SDL_INC_DIR := $(patsubst -I%,%,$(SDL_INCLUDE_FLAG))
SDL_BASE_INCLUDE := $(patsubst %/SDL,%,$(SDL_INC_DIR))
ifneq ($(SDL_BASE_INCLUDE),$(SDL_INC_DIR))
SDL_CFLAGS := $(SDL_CFLAGS) -I$(SDL_BASE_INCLUDE)
endif
POD2MAN ?= /usr/bin/pod2man --section=6

prefix ?= /usr/local
exec_prefix ?= $(prefix)
BINDIR ?= $(exec_prefix)/bin
MANDIR ?= $(prefix)/man
STATEDIR ?= $(prefix)/com
DEB ?=

MAN          = thrust.man
HIGHSCORE    = thrust.highscore
FULLHISCORE  = $(STATEDIR)/$(HIGHSCORE)
VERSION_NR   = 0.89
VERSION      = 0.89c
HELPPRG      = $(addprefix helpers/, bin2c txt2c ppm2c reverse)
BIN8         = $(addprefix datasrc/, \
                 blks0.bin blks1.bin blks2.bin blks3.bin \
                 blks4.bin blks5.bin blks6.bin blks7.bin \
                 blks8.bin blks9.bin blksa.bin blksb.bin \
                 blksc.bin blksd.bin blkse.bin blksf.bin )

DEFINES      = $(strip -DHIGHSCOREFILE=\"$(FULLHISCORE)\" -DVERSION=\"$(VERSION)\" -DHAVE_CONFIG_H)
WARNFLAGS    = -Wall -Wstrict-prototypes -Wmissing-prototypes
STDFLAGS     = -std=gnu89
OPTIMIZE     = -fomit-frame-pointer -O3
CFLAGS ?= $(WARNFLAGS) $(STDFLAGS) $(OPTIMIZE)
ALL_CFLAGS   = $(strip $(DEFINES) $(CFLAGS) $(SDL_CFLAGS))
HELP_CFLAGS  = -DHAVE_CONFIG_H $(CFLAGS)
LDFLAGS      =
LIBS         = -lm

NO_PBM       = yes
PBM_FLAGS    =   -lppm -lpgm -lpbm -DVERSION="0.6"

SOURCEOBJS   = $(addprefix src/, \
                 thrust.o fast_gr.o hiscore.o conf.o things.o init.o \
                 level.o font5x5.o graphics.o )
DATASEC      = $(addprefix datasrc/, \
                 blks.c ship.c shld.c colors.c bullet.c title.c demomove.c \
                 level1.c level2.c level3.c level4.c level5.c level6.c )
DATAOBJS     = $(addprefix datasrc/, font.o) $(patsubst %.c,%.o,$(DATASEC))
SOUNDITOBJS  = $(addprefix src/, soundIt.o)
SOUNDOBJS    = $(addprefix datasrc/, \
                 boom.o boom2.o harp.o thrust.o zero.o )
ifeq ($(SOUND),yes)
OBJS         = $(SOURCEOBJS) $(DATAOBJS) $(SOUNDITOBJS) $(SOUNDOBJS)
else
OBJS         = $(SOURCEOBJS) $(DATAOBJS)
endif
SDL_OBJS     = $(addprefix src/, SDLkey.o SDL.o )

TARFILE      = thrust-$(VERSION).src.tar
BINTARFILE   = thrust-$(VERSION).bin.tar
DISTFILES    = COPYING README INSTALL TODO CHANGES thrustrc thrust.lsm \
               thrust-$(VERSION).lsm $(MAN) $(HIGHSCORE) Makefile
SRCDISTFILES = $(addsuffix .pod.in, $(basename $(MAN))) thrust.pod install-sh

.PHONY: all install uninstall install-man uninstall-man \
        clean distclean mostlyclean realclean dep TAGS info dvi man dist bindist

all: sdlthrust

install:
	@if test ! -d $(DEB)/$(BINDIR) ; then \
	  echo $(INSTALL) -d $(DEB)/$(BINDIR); \
	  $(INSTALL) -d $(DEB)/$(BINDIR); \
	fi
	$(INSTALL) sdlthrust $(DEB)/$(BINDIR)/sdlthrust
	@if test ! -d $(DEB)/$(STATEDIR) ; then \
	  $(INSTALL) -d $(DEB)/$(STATEDIR); \
	fi

uninstall:
	-rm -i $(BINDIR)/sdlthrust
	-rm -i $(FULLHISCORE)

install-man: man
	@mkdir -p $(DEB)/$(MANDIR)/man6
	$(INSTALL) -m 0644 $(MAN) $(DEB)/$(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

uninstall-man:
	-rm $(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

clean:
	rm -rf $(strip *~ core sdlthrust $(OBJS) $(SDL_OBJS) $(HELPPRG) datasrc/blks*.bin .depend )
	rm -f build-stamp

distclean: clean
	rm -f src/TAGS $(TARFILE).gz $(BINTARFILE).gz thrust-$(VERSION).lsm $(addsuffix .pod,$(basename $(MAN)))

mostlyclean:
	rm -rf *~ core sdlthrust $(OBJS) $(SDL_OBJS)

realclean: distclean
	rm -f $(patsubst %.o,%.c,$(SOUNDOBJS)) $(DATASEC) $(MAN)

dep:
	$(CC) -M $(patsubst %.o,%.c,$(SOURCEOBJS)) > .depend
	$(CC) -M $(patsubst %.o,%.c,$(SDL_OBJS)) >> .depend
	ifneq ($(SOUND),yes)
		: # no extra dependencies
	else
		$(CC) -M $(patsubst %.o,%.c,$(SOUNDOBJS)) >> .depend
	endif

TAGS:
	etags src/*.c -o src/TAGS

info:
	@echo No documentation available.

dvi:
	@echo No documentation available.

man: $(MAN)

sdlthrust: $(OBJS) $(SDL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SDL_LIBS) $(LIBS)

dist: man $(patsubst %.o,%.c,$(DATAOBJS) $(SOUNDOBJS))
	mkdir -p thrust-$(VERSION)
	mkdir -p thrust-$(VERSION)/src
	mkdir -p thrust-$(VERSION)/datasrc
	mkdir -p thrust-$(VERSION)/helpers
	cp -dpf $(DISTFILES) thrust-$(VERSION)
	cp -dpf $(SRCDISTFILES) thrust-$(VERSION)
	cp -dpf src/*.h src/*.c src/*.in thrust-$(VERSION)/src
	cp -dpf datasrc/*.c datasrc/*.bmp datasrc/*.snd \
	        datasrc/*.pal datasrc/*.def datasrc/*.ppm \
	        datasrc/demomove.bin thrust-$(VERSION)/datasrc
	cp -dpf helpers/*.c thrust-$(VERSION)/helpers
	rm -f thrust-$(VERSION)/src/config.h
	chmod -R g-w thrust-$(VERSION)/*
	tar -cf $(TARFILE) thrust-$(VERSION)/*
	gzip -f9 $(TARFILE)
	rm -rf thrust-$(VERSION)

bindist: sdlthrust man
	mkdir -p thrust-$(VERSION)
	cp -dpf $(DISTFILES) sdlthrust thrust-$(VERSION)
	cp -dpf src/*.h src/*.c src/*.in thrust-$(VERSION)/src
	cp -dpf datasrc/*.c datasrc/*.bmp datasrc/*.snd \
	        datasrc/*.pal datasrc/*.def datasrc/*.ppm \
	        datasrc/demomove.bin thrust-$(VERSION)/datasrc
	cp -dpf helpers/*.c thrust-$(VERSION)/helpers
	rm -f thrust-$(VERSION)/src/config.h
	chmod -R g-w thrust-$(VERSION)/*
	tar -cf $(BINTARFILE) thrust-$(VERSION)/*
	gzip -f9 $(BINTARFILE)
	rm -rf thrust-$(VERSION)

datasrc/blks.bin: $(BIN8)
	cat $^ > $@

helpers/reverse: helpers/reverse.c
	$(CC) $(HELP_CFLAGS) $< -o $@

helpers/bin2c: helpers/bin2c.c
	$(CC) $(HELP_CFLAGS) $< -o $@

helpers/txt2c: helpers/txt2c.c
	$(CC) $(HELP_CFLAGS) $< -o $@

helpers/ppm2c: helpers/ppm2c.c
	$(CC) $(HELP_CFLAGS) $< $(PBM_FLAGS) -o $@

%.o: %.c
	$(CC) $(ALL_CFLAGS) -c -o $(addprefix $(dir $<),$(notdir $@)) $<

%.bin: %.pal
	$(DD) of=$@ if=$< bs=1 skip=790

%.rev: %.bmp
	$(DD) of=$@ if=$< bs=1 skip=1078

%.bin: %-4.rev helpers/reverse
	helpers/reverse 4 < $< > $@

%.bin: %-8.rev helpers/reverse
	helpers/reverse 8 < $< > $@

%.bin: %-16.rev helpers/reverse
	helpers/reverse 16 < $< > $@

%.c: %.def helpers/txt2c
	helpers/txt2c $(notdir $(basename $<)) < $< > $@

%.c: %.bin helpers/bin2c
	helpers/bin2c bin_$(notdir $(basename $<)) < $< > $@

%.c: %.snd helpers/bin2c
	/bin/echo > $@
	/bin/echo unsigned int sound_$(notdir $(basename $<))_len = `wc -c < $<`\; >> $@
	helpers/bin2c sound_$(notdir $(basename $<)) < $< >> $@

ifeq ($(NO_PBM),yes)
%.c: %.ppm
	@echo Must configure with pbm to build ppm2c.
	@echo Warning: Unable to rebuild $@ from $<.
else
%.c: %.ppm helpers/ppm2c
	helpers/ppm2c -n $(notdir $(basename $<)) -m 32 < $< > $@
endif

%.man: %.pod
ifeq ($(POD2MAN),)
	@echo Must have pod2man to rebuild man page.
	@echo Warning: Unable to rebuild $@ from $<.
else
	$(POD2MAN) $< > $@
endif

%.html: %.pod
ifeq ($(POD2MAN),)
	@echo Must have pod2html to rebuild man page.
	@echo Warning: Unable to rebuild $@ from $<.
else
	$(subst pod2man,pod2html,$(POD2MAN)) $< > $@
	rm pod2htmd.x~~ pod2htmi.x~~
endif

ifeq (.depend,$(wildcard .depend))
include .depend
endif
