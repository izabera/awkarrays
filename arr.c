#include "arr.h"

#define iterate(var, firstelem, ...)                              \
  for (int __i = 0; __i < 1; )                                    \
  for (__auto_type __first = firstelem; __i < 1; __i++)           \
  for (__typeof(*&__first) __tmp[] = { firstelem, __VA_ARGS__ },  \
       *var = __tmp;                                              \
       var < __tmp + sizeof __tmp/sizeof *__tmp;                  \
       var++)

arr *arr_new(arr *a) {
  *a = (arr) {
    .v = calloc(16, sizeof(elem)),
    .k = calloc(16, sizeof(elem)),
    .capacity = 16
  };
  return a;
}


void arr_free(arr *a) {
  // recursively free the whole thing
  for (size_t i = 0; i < a->size; i++) {
    if (!elem_is_empty(&a->k[i])) {
      if (elem_is_array(&a->v[i])) {
        arr_free(a->v[i].a);
      }
      else if (elem_is_s(&a->v[i]))
        s_free(&a->v[i].str);

      // a key can only be either s or num
      if (elem_is_s(&a->k[i]))
        s_free(&a->k[i].str);
    }
  }
  free(a->k);
  free(a->v);
}

#include <stdio.h>
int main() {
  arr a;
  arr_new(&a);
  arr_free(&a);

  /*awk_eval("myarray[0] = \"hi mom\"");*/
  /*awk_eval("myarray[1][0] = 123");*/
  /*awk_eval("myarray[1][1][0] = 4.56");*/
  /*awk_dump_variables();*/
}
