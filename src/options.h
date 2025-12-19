
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPTC "vhdec:jz:"
#define OPTS \
      { "version",        no_argument,       0, 'v' }, \
      { "help",           no_argument,       0, 'h' }, \
      { "nodemo",         no_argument,       0, 'd' }, \
      { "nosoundeffects", no_argument,       0, 'e' }, \
      { "gamma",          required_argument, 0, 'c' }, \
      { "step",           no_argument,       0, 'j' }, \
      { "zoom",           required_argument, 0, 'z' }

#define SDL_OPTC ""
#define SDL_OPTS
#endif /* OPTIONS_H */
