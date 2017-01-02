// awk variables

#pragma once
#include "s/s.h"
#include <stdint.h>

typedef union var {
  s str;
  double d; // not NaN or -0, inf and -inf are fine but they probably won't be used

  struct { // associative array
    union var *keys;
    //union var *values;   keys and values are allocated in the same block

    uint32_t arrsize,
             // capacity is always a power of 2 -1
             arrcapacity: 6,
             // last 4 bits are important flags
             maxprobe: 22;
  };
  char dbuf[sizeof(double)]; // used to hash the double

  struct {
    unsigned char filler1[15];
    // now this is a bit messy
    // but the point is to overlay these fields over the same bits in s
    union {
      struct {
        unsigned char filler2:5,
                      is_array:1,
                      is_num:1,
                      is_empty:1;
      };
      struct {
        unsigned char filler3:6,
                      // this is checked only if is_empty is 1
                      is_tombstone:1,
                      filler4:1;
      };
    };
  };
} var;



#define an0 __attribute__((nonnull))
#define an0si an0 static inline

an0si int var_is_empty(var *v) { return v->is_empty; }
an0si int var_is_tombstone(var *v) { return v->is_tombstone; }
an0si int var_is_array(var *v) { return !var_is_empty(v) && v->is_array; }
an0si int var_is_num(var *v) { return !var_is_empty(v) && !var_is_array(v) && v->is_num; }
an0si int var_is_s(var *v) { return !var_is_empty(v) && !var_is_array(v) && !var_is_num(v); }

an0 void var_free(var *);

an0 var *var_set_num(var *, double);
an0 var *var_set_s(var *, s *);
an0 arr *var_set_keyvar(var *, var *, var *);
#define var_set(what, val, ...) \
  _Generic((val),               \
      double: var_set_num,      \
      int: var_set_num,         \
      s*: var_set_s,            \
      var*: var_set_keyvar)(what, val, ##__VA_ARGS__)

#define var_tmp(x)                              \
  _Generic((x+0),                               \
      double: &(var) { .d = (x), .is_num = 1 }, \
      int:    &(var) { .d = (x), .is_num = 1 }, \
      char *:       (var *)(s_tmp(x))           \
      const char *: (var *)(s_tmp(x)))

an0 uint32_t var_hash(var *);
#undef an0si
#undef an0
