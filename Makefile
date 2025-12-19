# Generated automatically from Makefile.in by configure.
# ./configure 

# Written by Peter Ekberg, peda@lysator.liu.se

SHELL        = /bin/sh
CC           = gcc
DD           = dd
INSTALL      = /usr/bin/install -c

NO_PBM       = yes
NO_SVGA      = yes
NO_X         = yes
NO_SDL       = @no_sdl@
SOUND        = 
POD2MAN      = /usr/bin/pod2man --section=6

prefix       = /usr/local
exec_prefix  = ${prefix}
BINDIR       = ${exec_prefix}/bin
MANDIR       = ${prefix}/man
STATEDIR     = ${prefix}/com

DEFINES      = $(strip \
                 -DHIGHSCOREFILE=\"$(FULLHISCORE)\" \
                 -DVERSION=\"$(VERSION)\" \
                 -DHAVE_CONFIG_H)
OPTIMIZE     =  -fomit-frame-pointer -O9 -s
COMPILE      =  -Wall -Wstrict-prototypes -Wmissing-prototypes -std=gnu89 -I/opt/homebrew/include
ALL_CFLAGS   = $(strip \
                 $(DEFINES) $(OPTIMIZE) $(COMPILE) \
                  )
SVGA_LIBS    =  -lvgagl -lvga
X_LIBS       =    -lXext -lX11
SDL_LIBS     = -lSDL
LDFLAGS      = -L/opt/homebrew/lib

PBM_FLAGS    =   -lppm -lpgm -lpbm -DVERSION=\"0.6\"

MAN          = thrust.man
HIGHSCORE    = thrust.highscore
FULLHISCORE  = $(STATEDIR)/$(HIGHSCORE)
VERSION_NR   = 0.89
VERSION      = 0.89c

LIBS         = -lm

HELPPRG      = $(addprefix helpers/, bin2c txt2c ppm2c reverse)

BIN8         = $(addprefix datasrc/, \
                 blks0.bin blks1.bin blks2.bin blks3.bin \
                 blks4.bin blks5.bin blks6.bin blks7.bin \
                 blks8.bin blks9.bin blksa.bin blksb.bin \
                 blksc.bin blksd.bin blkse.bin blksf.bin )

SOURCEOBJS   = $(addprefix src/, \
                 thrust.o fast_gr.o hiscore.o conf.o things.o init.o \
		 level.o font5x5.o graphics.o )
DATASEC      = $(addprefix datasrc/, \
                 blks.c ship.c shld.c colors.c bullet.c title.c demomove.c \
                 level1.c level2.c level3.c level4.c level5.c level6.c)
DATAOBJS     = $(addprefix datasrc/, font.o) $(patsubst %.c,%.o,$(DATASEC))
SOUNDITOBJS  = $(addprefix src/, soundIt.o)
SOUNDOBJS    = $(addprefix datasrc/, \
                 boom.o boom2.o harp.o thrust.o zero.o)
ifeq ($(SOUND),yes)
OBJS         = $(SOURCEOBJS) $(DATAOBJS) $(SOUNDITOBJS) $(SOUNDOBJS)
else
OBJS         = $(SOURCEOBJS) $(DATAOBJS)
endif
X_OBJS       = $(addprefix src/, X11key.o X11.o)
X_DATAOBJS   = $(addprefix datasrc/, icon48.o)
SVGA_OBJS    = $(addprefix src/, svgakey.o ksyms.o svga.o)
SDL_OBJS     = $(addprefix src/, SDLkey.o SDL.o)

TARFILE      = thrust-$(VERSION).src.tar
BINTARFILE   = thrust-$(VERSION).bin.tar
DISTFILES    = COPYING README INSTALL TODO CHANGES thrustrc thrust.lsm \
               thrust-$(VERSION).lsm $(MAN) $(HIGHSCORE)
SRCDISTFILES = $(addsuffix .pod.in, $(basename $(MAN))) Makefile*.in \
               configure.in configure install-sh config.sub config.guess
BINDISTFILES = Makefile.bindist


#######################
#
# Targets.
#
#######################

.PHONY: all install uninstall install-man uninstall-man \
        clean distclean mostlyclean realclean \
        dep TAGS info dvi man dist bindist zip

.SECONDARY: $(DATASEC) $(patsubst %.o,%.c,$(X_DATAOBJS) $(SOUNDOBJS)) \
            $(HELPPRG)

.INTERMEDIATE: $(BIN8) datasrc/blks.bin

# is there an easier way than doing all 8 combinations?
ifeq ($(NO_X),yes)
 ifeq ($(NO_SVGA),yes)
  ifeq ($(NO_SDL),yes)
all:
	@echo Unable to find any one of X and SVGAlib.
	@echo Try reconfiguring.

install:
  else
all: sdlthrust

install: sdlthrust
  endif
 else
  ifeq ($(NO_SDL),yes)
all: thrust

install: thrust
  else
all: thrust sdlthrust

install: thrust sdlthrust
  endif
 endif
else
 ifeq ($(NO_SVGA),yes)
  ifeq ($(NO_SDL),yes)
all: xthrust

install: xthrust
  else
all: xthrust sdlthrust

install: xthrust sdlthrust
  endif
 else
  ifeq ($(NO_SDL),yes)
all: thrust xthrust

install: thrust xthrust
  else
all: thrust xthrust sdlthrust

install: thrust xthrust sdlthrust
  endif
 endif
endif
	@if test ! -d $(DEB)/$(BINDIR) ; then \
	  echo $(INSTALL) -d $(DEB)/$(BINDIR); \
	  $(INSTALL) -d $(DEB)/$(BINDIR); \
	fi
ifneq ($(NO_SVGA),yes)
	$(INSTALL) thrust $(DEB)/$(BINDIR)/thrust
endif
ifneq ($(NO_X),yes)
	$(INSTALL) xthrust $(DEB)/$(BINDIR)/xthrust
endif
ifneq ($(NO_SDL),yes)
	$(INSTALL) sdlthrust $(DEB)/$(BINDIR)/sdlthrust
endif
#	@if test ! -d $(STATEDIR) ; then \
#	  echo $(INSTALL) -d $(STATEDIR); \
#	  $(INSTALL) -d $(STATEDIR); \
#	fi
#	@if test ! -f $(FULLHISCORE) ; then \
#	  echo $(INSTALL) -m 660 $(HIGHSCORE) $(FULLHISCORE);\
#	  $(INSTALL) -m 660 $(HIGHSCORE) $(FULLHISCORE); \
#	fi

uninstall:
	rm -i $(BINDIR)/thrust
	rm -i $(BINDIR)/xthrust
	rm -i $(BINDIR)/sdlthrust
	rm -i $(FULLHISCORE)

install-man: man
	$(INSTALL) -m 0644 $(MAN) $(DEB)/$(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

uninstall-man:
	rm $(MANDIR)/man6/$(addsuffix .6,$(basename $(MAN)))

clean:
	rm -rf $(strip *~ core thrust xthrust sdlthrust $(OBJS) $(X_OBJS) $(X_DATAOBJS) $(SVGA_OBJS) $(SDL_OBJS) $(HELPPRG) datasrc/blks*.bin .depend )

distclean: clean
	rm -f src/TAGS $(TARFILE).gz \
              Makefile Makefile.bindist \
              $(addsuffix .pod, $(basename $(MAN))) src/config.h

mostlyclean:
	rm -rf *~ core thrust xthrust sdlthrust $(SOURCEOBJS) $(X_OBJS) $(SVGA_OBJS) $(SDL_OBJS)

realclean: distclean
	rm -f $(patsubst %.o,%.c,$(X_DATAOBJS) $(SOUNDOBJS)) $(DATASEC) \
              $(MAN) $(BINTARFILE).gz thrust-$(VERSION).lsm \
              config.cache config.log config.status configure

dep:
	$(CC) -M $(patsubst %.o,%.c,$(SOURCEOBJS)) > .depend
ifneq ($(NO_SVGA),yes)
	$(CC) -M $(patsubst %.o,%.c,$(SVGA_OBJS)) >> .depend
endif
ifneq ($(NO_X),yes)
	$(CC) -M $(patsubst %.o,%.c,$(X_OBJS) $(X_DATAOBJS)) >> .depend
endif
ifneq ($(NO_SDL),yes)
	$(CC) -M $(patsubst %.o,%.c,$(SDL_OBJS)) >> .depend
endif

TAGS:
	etags src/*.c -o src/TAGS

info:
	@echo No documentation available.

dvi:
	@echo No documentation available.

man: $(MAN)

dist: man $(patsubst %.o,%.c,$(DATAOBJS) $(X_DATAOBJS) $(SOUNDOBJS))
	mkdir thrust-$(VERSION)
	mkdir thrust-$(VERSION)/src
	mkdir thrust-$(VERSION)/datasrc
	mkdir thrust-$(VERSION)/helpers
	mkdir thrust-$(VERSION)/DirectX
	cp -dpf $(DISTFILES) thrust-$(VERSION)
	cp -dpf $(SRCDISTFILES) thrust-$(VERSION)
	cp -dpf src/*.h src/*.c src/*.in \
	        thrust-$(VERSION)/src
	cp -dpf datasrc/*.c datasrc/*.bmp datasrc/*.snd \
	        datasrc/*.pal datasrc/*.def datasrc/*.ppm \
	        datasrc/demomove.bin \
	        thrust-$(VERSION)/datasrc
	cp -dpf helpers/*.c thrust-$(VERSION)/helpers
	cp -dpf DirectX/* thrust-$(VERSION)/DirectX
	rm -f thrust-$(VERSION)/src/config.h
	chmod -R g-w thrust-$(VERSION)/*
	tar -cf $(TARFILE) thrust-$(VERSION)/*
	gzip -f9 $(TARFILE)
	rm -rf thrust-$(VERSION)

ifneq ($(NO_SVGA),yes)
ifneq ($(NO_X),yes)
ifneq ($(NO_SDL),yes)
bindist: thrust xthrust sdlthrust man
	mkdir thrust-$(VERSION)
	cp -dpf $(DISTFILES) $(BINDISTFILES) sdlthrust xthrust thrust thrust-$(VERSION)
	mv thrust-$(VERSION)/Makefile.bindist thrust-$(VERSION)/Makefile
	chmod -R g-w thrust-$(VERSION)/*
	tar -cf $(BINTARFILE) thrust-$(VERSION)/*
	gzip -f9 $(BINTARFILE)
	rm -rf thrust-$(VERSION)
else
bindist:
	@echo Only for Linux with X, SVGAlib and SDL.
endif
else
bindist:
	@echo Only for Linux with X, SVGAlib and SDL.
endif
else
bindist:
	@echo Only for Linux with X, SVGAlib and SDL.
endif


#######################
#
# Implicit rules.
#
#######################

# Compile C files to object files.
%.o: %.c
	$(CC) $(ALL_CFLAGS) -c -o $(addprefix $(dir $<), $(notdir $@)) $<


# Extract palette information.
%.bin: %.pal
	$(DD) of=$@ if=$< bs=1 skip=790
# Extract picture information.
%.rev: %.bmp
	$(DD) of=$@ if=$< bs=1 skip=1078


# The bmp format stores rows backwards (last row first).
# These rules are used to "reverse" the order in which the rows are stored.
# There is one rule for each of the three used widths of the line.
%.bin: %-4.rev helpers/reverse
	helpers/reverse 4 < $< > $@
%.bin: %-8.rev helpers/reverse
	helpers/reverse 8 < $< > $@
%.bin: %-16.rev helpers/reverse
	helpers/reverse 16 < $< > $@


# Make a C file out of a text file. The info is stored in an array of strings.
# One line in the original text file will be stored in one cell of the array.
%.c: %.def helpers/txt2c
	helpers/txt2c $(notdir $(basename $<)) < $< > $@

# Make a C file out of a binary file. The info is stored in a byte array.
%.c: %.bin helpers/bin2c
	helpers/bin2c bin_$(notdir $(basename $<)) < $< > $@

# Make a C file out of a sound file.
%.c: %.snd helpers/bin2c
	/bin/echo > $@
	/bin/echo unsigned int sound_$(notdir $(basename $<))_len = \
        `wc -c < $<`\; >> $@
	helpers/bin2c sound_$(notdir $(basename $<)) < $< >> $@

# Make a C file out of a ppm picture file.
ifeq ($(NO_PBM),yes)
%.c: %.ppm
	@echo Must configure with pbm to build ppm2c.
	@echo Warning: Unable to rebuild $@ from $<.
else
%.c: %.ppm helpers/ppm2c
	helpers/ppm2c -n $(notdir $(basename $<)) -m 32 < $< > $@
endif


# Make a man page from a pod source
%.man: %.pod
ifeq ($(POD2MAN),)
	@echo Must have pod2man to rebuild man page.
	@echo Warning: Unable to rebuild $@ from $<.
else
	$(POD2MAN) $< > $@
endif

# Make html man page from a pod source
%.html: %.pod
ifeq ($(POD2MAN),)
	@echo Must have pod2html to rebuild man page.
	@echo Warning: Unable to rebuild $@ from $<.
else
	$(subst pod2man,pod2html,$(POD2MAN)) $< > $@
	rm pod2htmd.x~~ pod2htmi.x~~
endif



#######################
#
# Secondary targets.
#
#######################

ifneq ($(NO_SVGA),yes)
thrust: $(OBJS) $(SVGA_OBJS)
	$(CC) $(LDFLAGS) -o thrust $(OBJS) $(SVGA_OBJS) $(SVGA_LIBS) $(LIBS)
endif

ifneq ($(NO_X),yes)
xthrust: $(OBJS) $(X_OBJS) $(X_DATAOBJS)
	$(CC) $(LDFLAGS) -o xthrust $(OBJS) $(X_OBJS) $(X_DATAOBJS) $(X_LIBS) $(LIBS)
endif

ifneq ($(NO_SDL),yes)
sdlthrust: $(OBJS) $(SDL_OBJS)
	$(CC) $(LDFLAGS) -o sdlthrust $(OBJS) $(SDL_OBJS) $(SDL_LIBS) $(LIBS)
endif

datasrc/blks.bin: $(BIN8)
	cat $^ > $@

helpers/reverse: helpers/reverse.c
	$(CC) -DHAVE_CONFIG_H $(COMPILE) $< -o $@

helpers/bin2c: helpers/bin2c.c
	$(CC) -DHAVE_CONFIG_H $(COMPILE) $< -o $@

helpers/txt2c: helpers/txt2c.c
	$(CC) -DHAVE_CONFIG_H $(COMPILE) $< -o $@

ifneq ($(NO_PBM),yes)
helpers/ppm2c: helpers/ppm2c.c $(addprefix src/, )
	$(CC) -DHAVE_CONFIG_H $< $(PBM_FLAGS) -o $@
endif


#######################
#
# Dependencies.
#
#######################

ifeq (.depend,$(wildcard .depend))
include .depend
endif
