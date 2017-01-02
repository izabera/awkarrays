// awk arrays

#pragma once
#include "s/s.h"
#include <stdint.h>
typedef union {
  s str;
  double d; // not NaN or -0, inf and -inf are fine but they probably won't be used
  char dbuf[sizeof(double)];
  struct arr *a;
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
} elem;

typedef struct arr {
  elem *v;
  elem *k;
  size_t size, capacity;
} arr;


#define an0 __attribute__((nonnull))
#define an0si an0 static inline

an0si int elem_is_empty(elem *e) { return e->is_empty; }
an0si int elem_is_tombstone(elem *e) { return e->is_tombstone; }
an0si int elem_is_array(elem *e) { return !elem_is_empty(e) && e->is_array; }
an0si int elem_is_num(elem *e) { return !elem_is_empty(e) && !elem_is_array(e) && e->is_num; }
an0si int elem_is_s(elem *e) { return !elem_is_empty(e) && !elem_is_array(e) && !elem_is_num(e); }

an0 void arr_free(arr *);
an0 void elem_free(elem *);
an0 elem *elem_set_num(elem *, double);
an0 elem *elem_set_s(elem *, s *);
an0 arr *arr_set_elem(arr *, elem *, elem *);

an0 uint32_t elem_hash(elem *);
#undef an0si
#undef an0
