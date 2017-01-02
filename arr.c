#include "arr.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define iterate(var, firstelem, ...)                              \
  for (int __i = 0; __i < 1; )                                    \
  for (__auto_type __first = firstelem; __i < 1; __i++)           \
  for (__typeof(*&__first) __tmp[] = { firstelem, __VA_ARGS__ },  \
       *var = __tmp;                                              \
       var < __tmp + sizeof __tmp/sizeof *__tmp;                  \
       var++)

arr *arr_new(arr *a) {
  // a single array to minimize calls to calloc and free
  var *tmp = calloc(32, sizeof(var));
  *a = (arr) {
    .v = tmp,
    .k = tmp + 16,
    .capacity = 16
  };
  return a;
}


void arr_free(arr *a) {
  // recursively free the whole thing
  for (size_t i = 0; i < a->size; i++) {
    var_free(&a->k[i]);
    var_free(&a->v[i]);
  }
  free(a->k);
  //free(a->v);  it's a single array so free it only once
}

void var_free(var *v) {
  if (var_is_array(v)) arr_free(v->a);
  else if (var_is_s(v)) s_free(&v->str);
}

var *var_set_num(var *v, double d) {
  *v = (var) { 0 };
  v->d = d;
  v->is_num = 1;
  return v;
}

var *var_set_s(var *v, s *str) {
  v->str = *str;
  return v;
}

// idea:
// it's possible to craft several malicious inputs that will collide
// because jenkins(input, len) will return the same value, *before*
// doing % size, and resizing the table won't change it
//
// possible solution: generate a random starting hash for every resize
// todo: change this function if it proves worthy
static inline uint32_t get_starting_hash() { return 0; }

static uint32_t jenkins(const uint8_t *key, size_t length) {
  size_t i = 0;
  uint32_t hash = get_starting_hash();
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

uint32_t var_hash(var *v) {
  assert(!var_is_arr(v));
  return var_is_s(v) ? jenkins(s_data(&v->str), s_size(&v->str))
                     : jenkins(v->dbuf, sizeof(double));
}

int var_cmp(var *e1, var *e2) {
  if (var_is_empty(e1)) return !var_is_empty(e2);
  if (var_is_empty(e2)) return !var_is_empty(e1);

  if (var_is_num(e1)) return var_is_num(e2) ? e1->d == e2->d ? 0 : e1->d < e2->d : 1;
  if (var_is_s(e1)) return var_is_s(e2) ? s_cmp(&e1->str, &e2->str) : 1;

  if (e1->a->size != e2->a->size) return 1;
  // todo
#warning "finish this"
  return 0;
}

// idea:
// instead of just checking how full the table is, check how bad it is
//
// it's possible that even with a low fill factor, several keys end up in the
// same bucket or slot or whatever
// and by only checking the fill factor, we wouldn't resize the table
//
// it's also possible that with a high fill factor, keys are well distributed
// resizing the table in that case can actually harm
//
// possible solution: since this uses open addressing, check the "longest
// chain of collisions"
// problem: how to deal with tombstones?

arr *arr_set(arr *a, var *key, var *val) {
  uint32_t idx = var_hash(key) % a->size;
  var *keys = a->k, *values = a->v;
  for ( ; ; idx = (idx == a->size-1) ? 0 : idx++) {
    if (var_is_empty(&keys[idx])) {
      keys[idx] = *key;
      goto val;
    }
    if (!var_cmp(key, &keys[idx])) {
 val: values[idx] = *val;
      break;
    }
  }
  return a;
}

var *var_arr_get(var *a, var *key) {
  uint32_t mask = a->mask,
           idx = var_hash(key) & hash,
           cnt = 0,
           maxprobe = a->maxprobe;
  var *keys = a->key, *values = keys + var_arrcapacity(a);

  for ( ; cnt < maxprobe; idx = (idx + ++cnt) & mask) {
    if (var_is_empty(&keys[idx])) {
      if (!var_is_tombstone(&keys[idx])) break; // todo: return null?
    }
    else if (!var_cmp(key, &keys[idx])) break;
  }
  return &values[idx];
}


#include <stdio.h>
int main() {
  var arr;
  var_new_array(&arr);
  var_set(&arr, var_tmp("foobar"), var_tmp(1234.5678));
  var_set(&arr, var_tmp("meow"), var_tmp("cat"));
  var_set(&arr, var_tmp(1234), var_tmp(5678));
  var_free(&arr);

  /*awk_eval("myarray[0] = \"hi mom\"");*/
  /*awk_eval("myarray[1][0] = 123");*/
  /*awk_eval("myarray[1][1][0] = 4.56");*/
  /*awk_dump_variables();*/
}
