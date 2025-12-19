
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPTC "vhdec:j"
#define OPTS \
      { "version",        no_argument,       0, 'v' }, \
      { "help",           no_argument,       0, 'h' }, \
      { "nodemo",         no_argument,       0, 'd' }, \
      { "nosoundeffects", no_argument,       0, 'e' }, \
      { "gamma",          required_argument, 0, 'c' }, \
      { "step",           no_argument,       0, 'j' }

#define SDL_OPTC "2"
#define SDL_OPTS \
      { "double",         no_argument,       0, '2' }

#endif /* OPTIONS_H */
