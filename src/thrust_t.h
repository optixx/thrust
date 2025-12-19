
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef THRUST_TYPES_H
#define THRUST_TYPES_H

#define INCR_WRAP(x, max_val, reset_val) \
  do { \
    (x)++; \
    if((x) >= (max_val)) \
      (x) = (reset_val); \
  } while(0)

#define DECR_WRAP(x, min_val, reset_val) \
  do { \
    (x)--; \
    if((x) < (min_val)) \
      (x) = (reset_val) - 1; \
  } while(0)

#include <stdint.h>

#ifndef MIN
#define MIN(a,b) (( (a) < (b) ) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (( (a) > (b) ) ? (a) : (b))
#endif

#endif /* THRUST_TYPES_H */
