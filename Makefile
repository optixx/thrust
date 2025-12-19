# Simplified build for SDL-based Thrust on macOS.

SHELL        = /bin/sh
INSTALL      = /usr/bin/install -c
CC ?= gcc
SDLCONFIG ?= sdl-config
PYTHON ?= python3
ASSET_TOOL := $(PYTHON) helpers/asset_tool.py
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
BIN8         = $(addprefix assets/, \
                 blks0.bin blks1.bin blks2.bin blks3.bin \
                 blks4.bin blks5.bin blks6.bin blks7.bin \
                 blks8.bin blks9.bin blksa.bin blksb.bin \
                 blksc.bin blksd.bin blkse.bin blksf.bin )
BMP_FILES    = $(wildcard assets/*-*.bmp)
BMP_BASE     = $(word 1,$(subst -, ,$(basename $(notdir $1))))
BMP_BLOCK    = $(word 2,$(subst -, ,$(basename $(notdir $1))))
BMP_BINARIES = $(foreach bmp,$(BMP_FILES),assets/$(call BMP_BASE,$(bmp)).bin)
PAL_FILES    = $(wildcard assets/*.pal)
PAL_BINARIES = $(foreach pal,$(PAL_FILES),assets/$(basename $(notdir $(pal))).bin)
ASSET_BINARIES = $(sort $(BMP_BINARIES) $(PAL_BINARIES) assets/blks.bin)
OBJDIR       = build
SRC_OBJ_DIR  = $(OBJDIR)/src
ASSET_OBJ_DIR = $(OBJDIR)/assets

DEFINES      = $(strip -DHIGHSCOREFILE=\"$(FULLHISCORE)\" -DVERSION=\"$(VERSION)\" -DHAVE_CONFIG_H)
WARNFLAGS    = -Wall -Wstrict-prototypes -Wmissing-prototypes
STDFLAGS     = -std=gnu89
OPTIMIZE     = -fomit-frame-pointer -O3
CFLAGS ?= $(WARNFLAGS) $(STDFLAGS) $(OPTIMIZE)
ALL_CFLAGS   = $(strip $(DEFINES) $(CFLAGS) $(SDL_CFLAGS))
HELP_CFLAGS  = -DHAVE_CONFIG_H $(CFLAGS)
LDFLAGS      =
LIBS         = -lm

SOURCEOBJS   = $(addprefix $(SRC_OBJ_DIR)/, \
                 thrust.o fast_gr.o hiscore.o conf.o things.o init.o \
                 level.o font5x5.o graphics.o )
DATASEC      = $(addprefix assets/, \
                 blks.c ship.c shld.c colors.c bullet.c title.c demomove.c \
                 level1.c level2.c level3.c level4.c level5.c level6.c )
DATAOBJS     = $(addprefix $(ASSET_OBJ_DIR)/, font.o) $(patsubst %.c,%.o,$(addprefix $(ASSET_OBJ_DIR)/,$(notdir $(DATASEC))))
SOUNDITOBJS  = $(addprefix $(SRC_OBJ_DIR)/, soundIt.o)
SOUNDOBJS    = $(addprefix $(ASSET_OBJ_DIR)/, \
                 boom.o boom2.o harp.o thrust.o zero.o )
ifeq ($(SOUND),yes)
OBJS         = $(SOURCEOBJS) $(DATAOBJS) $(SOUNDITOBJS) $(SOUNDOBJS)
else
OBJS         = $(SOURCEOBJS) $(DATAOBJS)
endif
SDL_OBJS     = $(addprefix $(SRC_OBJ_DIR)/, SDLkey.o SDL.o )
ASSET_CS     = $(DATASEC)

.PHONY: all clean TAGS dvi assets

all: sdlthrust

clean:
	rm -rf $(strip *~ core sdlthrust $(OBJS) $(SDL_OBJS) assets/*.bin .depend build)
	rm -f build-stamp

TAGS:
	etags src/*.c -o src/TAGS

dvi:
	@echo No documentation available.

sdlthrust: $(OBJS) $(SDL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SDL_LIBS) $(LIBS)

assets: $(ASSET_BINARIES) $(ASSET_CS)

assets/blks.bin: $(BIN8)
	cat $^ > $@

$(SRC_OBJ_DIR):
	mkdir -p $@

$(ASSET_OBJ_DIR):
	mkdir -p $@

$(SRC_OBJ_DIR)/%.o: src/%.c | $(SRC_OBJ_DIR)
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

$(ASSET_OBJ_DIR)/%.o: assets/%.c | $(ASSET_OBJ_DIR)
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

define BMP_TO_BIN_RULE
assets/$(call BMP_BASE,$(1)).bin: $(1)
	$(ASSET_TOOL) bmp2bin --block-size $(call BMP_BLOCK,$(1)) --skip 1078 $(1) $$@
endef
$(foreach bmp,$(BMP_FILES),$(eval $(call BMP_TO_BIN_RULE,$(bmp))))

define PAL_TO_BIN_RULE
assets/$(basename $(notdir $(1))).bin: $(1)
	$(ASSET_TOOL) pal2bin --skip 790 $(1) $$@
endef
$(foreach pal,$(PAL_FILES),$(eval $(call PAL_TO_BIN_RULE,$(pal))))

%.c: %.def
	$(ASSET_TOOL) def2c $(notdir $(basename $<)) $< > $@

%.c: %.bin
	$(ASSET_TOOL) bin2c bin_$(notdir $(basename $<)) $< > $@

%.c: %.snd
	$(ASSET_TOOL) sound2c sound_$(notdir $(basename $<)) $< > $@

ifeq (.depend,$(wildcard .depend))
include .depend
endif
