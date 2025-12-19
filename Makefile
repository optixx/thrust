# Simplified build for SDL-based Thrust on macOS.

SHELL        = /bin/sh
CC ?= gcc
SDLCONFIG ?= sdl2-config
PYTHON ?= python3
ASSET_TOOL := $(PYTHON) helpers/asset_tool.py
SDL_CFLAGS := $(shell $(SDLCONFIG) --cflags 2>/dev/null)
SDL_LIBS := $(shell $(SDLCONFIG) --libs 2>/dev/null)
ifeq ($(strip $(SDL_LIBS)),)
SDL_LIBS = -L/opt/homebrew/lib -lSDL2
endif
ifeq ($(strip $(SDL_CFLAGS)),)
SDL_CFLAGS = -I/opt/homebrew/include/SDL2 -I/opt/homebrew/include -I/Library/Frameworks/SDL2.framework/Headers
endif

HIGHSCORE    = thrust.highscore
FULLHISCORE  = $(HIGHSCORE)
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

DEFINES      = $(strip -DHIGHSCOREFILE=\"$(FULLHISCORE)\" -DVERSION=\"$(VERSION)\")
WARNFLAGS    = -Wall -Wstrict-prototypes -Wmissing-prototypes
STDFLAGS     = -std=gnu89
OPTIMIZE     = -fomit-frame-pointer -O3
CFLAGS ?= $(WARNFLAGS) $(STDFLAGS) $(OPTIMIZE)
ALL_CFLAGS   = $(strip $(DEFINES) $(CFLAGS) $(SDL_CFLAGS))
HELP_CFLAGS  = $(CFLAGS)
LDFLAGS      =
LIBS         = -lm

SOURCEOBJS   = $(addprefix $(SRC_OBJ_DIR)/, \
                 thrust.o hiscore.o conf.o things.o init.o \
                 level.o font5x5.o graphics.o )
DATASEC      = $(addprefix assets/, \
                 blks.c ship.c shld.c colors.c bullet.c title.c demomove.c \
                 level1.c level2.c level3.c level4.c level5.c level6.c )
DATAOBJS     = $(addprefix $(ASSET_OBJ_DIR)/, font.o) $(patsubst %.c,%.o,$(addprefix $(ASSET_OBJ_DIR)/,$(notdir $(DATASEC))))
SOUNDITOBJS  = $(addprefix $(SRC_OBJ_DIR)/, soundIt.o)
SOUNDOBJS    = $(addprefix $(ASSET_OBJ_DIR)/, \
                 boom.o boom2.o harp.o thrust.o zero.o )
OBJS         = $(SOURCEOBJS) $(DATAOBJS) $(SOUNDITOBJS) $(SOUNDOBJS)
SDL_OBJS     = $(addprefix $(SRC_OBJ_DIR)/, SDLkey.o SDL.o )
ASSET_CS     = $(DATASEC)

.PHONY: all clean assets

all: sdlthrust

clean:
	rm -rf $(strip *~ core sdlthrust $(OBJS) $(SDL_OBJS) assets/*.bin .depend build)
	rm -f build-stamp

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
