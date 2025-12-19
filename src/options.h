
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPTC "vhdz:s"
#define OPTS                                                                                       \
    {"version", no_argument, 0, 'v'}, {"help", no_argument, 0, 'h'},                               \
        {"nodemo", no_argument, 0, 'd'}, {"zoom", required_argument, 0, 'z'},                      \
        {"smooth", no_argument, 0, 's'}

#endif /* OPTIONS_H */
