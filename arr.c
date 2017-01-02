#include "arr.h"
#include <stdint.h>
#include <string.h>

#define iterate(var, firstelem, ...)                              \
  for (int __i = 0; __i < 1; )                                    \
  for (__auto_type __first = firstelem; __i < 1; __i++)           \
  for (__typeof(*&__first) __tmp[] = { firstelem, __VA_ARGS__ },  \
       *var = __tmp;                                              \
       var < __tmp + sizeof __tmp/sizeof *__tmp;                  \
       var++)

arr *arr_new(arr *a) {
  // a single array to minimize calls to calloc and free
  elem *tmp = calloc(32, sizeof(elem));
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
    elem_free(&a->k[i]);
    elem_free(&a->v[i]);
  }
  free(a->k);
  //free(a->v);  it's a single array so free it only once
}

void elem_free(elem *e) {
  if (elem_is_array(e)) arr_free(e->a);
  else if (elem_is_s(e)) s_free(&e->str);
}

elem *elem_set_num(elem *e, double d) {
  *e = (elem) { 0 };
  e->d = d;
  e->is_num = 1;
  return e;
}

elem *elem_set_s(elem *e, s *str) {
  e->str = *str;
  return e;
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

uint32_t elem_hash(elem *e) {
  return elem_is_s(e) ? jenkins(s_data(&e->str), s_size(&e->str))
                      : jenkins(e->dbuf, sizeof(double));
}

int elem_cmp(elem *e1, elem *e2) {
  if (elem_is_empty(e1)) return !elem_is_empty(e2);
  if (elem_is_empty(e2)) return !elem_is_empty(e1);

  if (elem_is_num(e1)) return elem_is_num(e2) ? e1->d == e2->d ? 0 : e1->d < e2->d : 1;
  if (elem_is_s(e1)) return elem_is_s(e2) ? s_cmp(&e1->str, &e2->str) : 1;

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

arr *arr_set_elem(arr *a, elem *key, elem *val) {
  uint32_t idx = elem_hash(key) % a->size;
  elem *keys = a->k, *values = a->v;
  for ( ; ; idx = (idx == a->size-1) ? 0 : idx++) {
    if (elem_is_empty(&keys[idx])) {
      keys[idx] = *key;
      goto val;
    }
    if (!elem_cmp(key, &keys[idx])) {
 val: values[idx] = *val;
      break;
    }
  }
  return a;
}

elem *arr_get_elem(arr *a, elem *key) {
  uint32_t idx = elem_hash(key) % a->size;
  elem *keys = a->k, *values = a->v;
  for ( ; ; idx = (idx == a->size-1) ? 0 : idx++) {
    // #tombstones * 8 < #entries
    // #tombstones + #empty >= #entries/2
    // so this loop always terminates
    if (elem_is_empty(&keys[idx])) {
      if (!elem_is_tombstone(&keys[idx])) break; // todo: return null?
    }
    else if (!elem_cmp(key, &keys[idx])) break;
  }
  return &values[idx];
}


#include <stdio.h>
int main() {
  arr a;
  arr_new(&a);
  elem key, value;
  s str;
  elem_set_s(&key, s_new(&str, "foobar"));
  elem_set_num(&value, 1234.5678);
  arr_set_elem(&a, &key, &value);
  arr_free(&a);

  /*awk_eval("myarray[0] = \"hi mom\"");*/
  /*awk_eval("myarray[1][0] = 123");*/
  /*awk_eval("myarray[1][1][0] = 4.56");*/
  /*awk_dump_variables();*/
}
