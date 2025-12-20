
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

static inline void
incr_wrap(int *value, int max_val, int reset_val)
{
    (*value)++;
    if (*value >= max_val)
        *value = reset_val;
}

static inline void
decr_wrap(int *value, int min_val, int reset_val)
{
    (*value)--;
    if (*value < min_val)
        *value = reset_val - 1;
}

#include <stdbool.h>

static inline int
min_int(int a, int b)
{
    return (a < b) ? a : b;
}

static inline int
max_int(int a, int b)
{
    return (a > b) ? a : b;
}

#endif /* THRUST_TYPES_H */
