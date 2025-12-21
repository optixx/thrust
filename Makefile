# Simplified build for SDL-based Thrust on macOS.

SHELL        = /bin/sh
CC ?= gcc
SDLCONFIG ?= sdl2-config
PYTHON ?= python3
PYTHON3_14 := $(shell command -v python3.14 2>/dev/null)
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
VERSION      = 0.90
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

DEFINES      = $(strip -DHIGHSCOREFILE=\"$(HISCORE)\" -DVERSION=\"$(VERSION)\")
WARNFLAGS    = -Wall -Wstrict-prototypes -Wmissing-prototypes
STDFLAGS     = -std=c11
OPTIMIZE     = -fomit-frame-pointer -O3
CFLAGS ?= $(WARNFLAGS) $(STDFLAGS) $(OPTIMIZE)
ALL_CFLAGS   = $(strip $(DEFINES) $(CFLAGS) $(SDL_CFLAGS))
HELP_CFLAGS  = $(CFLAGS)
LDFLAGS      =
LIBS         = -lm

EMCC ?= emcc
EMFLAGS ?= -O3 -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sEXIT_RUNTIME=1 -sWASM=1 -sASYNCIFY $(DEFINES)
EMSDK_PYTHON ?= $(if $(PYTHON3_14),$(PYTHON3_14),python3)
EM_PYTHON ?= $(EMSDK_PYTHON)
WASM_EMCC := EMSDK_PYTHON=$(EMSDK_PYTHON) $(EMCC)
WASM_OUTPUT_DIR := wasm-dist
WASM_JS := $(WASM_OUTPUT_DIR)/sdlthrust.js
WASM_HTML := $(WASM_OUTPUT_DIR)/index.html
WASM_SRC := src/thrust.c src/main_loop.c src/game.c src/state.c src/menu.c src/menu_state.c src/demo_state.c \
            src/screen_state.c src/pref.c src/hud.c src/hiscore.c src/conf.c src/things.c \
            src/init.c src/level.c src/font5x5.c src/graphics.c src/assets.c src/world.c \
            src/input.c src/soundIt.c src/SDL.c assets/font.c assets/boom.c assets/boom2.c \
            assets/harp.c assets/thrust.c assets/zero.c
WASM_RC := $(WASM_OUTPUT_DIR)/thrust.rc
WASM_DEPS := $(WASM_SRC) $(ASSET_CS)

SOURCEOBJS   = $(addprefix $(SRC_OBJ_DIR)/, \
                 thrust.o game.o state.o menu.o menu_state.o demo_state.o screen_state.o main_loop.o pref.o hud.o hiscore.o conf.o things.o init.o \
                 level.o font5x5.o graphics.o assets.o world.o )
DATASEC      = $(addprefix assets/, \
                 blks.c ship.c shld.c colors.c bullet.c title.c demomove.c \
                 level1.c level2.c level3.c level4.c level5.c level6.c )
DATAOBJS     = $(addprefix $(ASSET_OBJ_DIR)/, font.o) $(patsubst %.c,%.o,$(addprefix $(ASSET_OBJ_DIR)/,$(notdir $(DATASEC))))
SOUNDITOBJS  = $(addprefix $(SRC_OBJ_DIR)/, soundIt.o)
SOUNDOBJS    = $(addprefix $(ASSET_OBJ_DIR)/, \
                 boom.o boom2.o harp.o thrust.o zero.o )
OBJS         = $(SOURCEOBJS) $(DATAOBJS) $(SOUNDITOBJS) $(SOUNDOBJS)
SDL_OBJS     = $(addprefix $(SRC_OBJ_DIR)/, input.o SDL.o )
ASSET_CS     = $(DATASEC)

all: sdlthrust

clean:
	rm -rf $(strip *~ core sdlthrust $(OBJS) $(SDL_OBJS) assets/*.bin .depend build) $(WASM_OUTPUT_DIR)
	rm -f build-stamp

format:
	./scripts/format.sh

lint:
	./scripts/lint.sh

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

PORT ?= 8000

.PHONY: all clean assets wasm serve-wasm wasm-build
wasm: wasm-build
wasm-build: $(WASM_JS) $(WASM_HTML) $(WASM_RC)

$(WASM_OUTPUT_DIR):
	mkdir -p $(WASM_OUTPUT_DIR)

$(WASM_JS): $(WASM_DEPS) | $(WASM_OUTPUT_DIR)
	$(WASM_EMCC) $(EMFLAGS) $(WASM_SRC) $(ASSET_CS) -o $@

serve-wasm:
	$(PYTHON) helpers/serve_wasm.py --directory $(WASM_OUTPUT_DIR) --port $(PORT)

$(WASM_HTML): wasm/index.html | $(WASM_OUTPUT_DIR)
	cp $< $@

$(WASM_RC): thrust.rc | $(WASM_OUTPUT_DIR)
	cp $< $@

ifeq (.depend,$(wildcard .depend))
include .depend
endif
