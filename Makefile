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

prefix ?= /usr/local
exec_prefix ?= $(prefix)
BINDIR ?= $(exec_prefix)/bin
MANDIR ?= $(prefix)/man
STATEDIR ?= $(prefix)/com
DEB ?=

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

 .PHONY: all clean TAGS dvi

all: sdlthrust

clean:
	rm -rf $(strip *~ core sdlthrust $(OBJS) $(SDL_OBJS) $(HELPPRG) datasrc/blks*.bin .depend )
	rm -f build-stamp

TAGS:
	etags src/*.c -o src/TAGS

dvi:
	@echo No documentation available.

sdlthrust: $(OBJS) $(SDL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SDL_LIBS) $(LIBS)

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

ifeq (.depend,$(wildcard .depend))
include .depend
endif
