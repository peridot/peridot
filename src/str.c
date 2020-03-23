#include "str.h"
#include <stdlib.h>
#include "gc.h"
#include "value.h"
#include <string.h>
#include "peridot.h"

// FNV-1a algorithm for hashing.
// TODO: try out a few more hash algorithms and see if they perform better than this
static uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;
  
  for(int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }
  return hash;
}

pd_value pd_str_new(pvm_t* vm, char* cstr, size_t len) {
  uint32_t hash = hashString(cstr, len);
  pd_str* interned = pd_table_find_string(&vm->strings, cstr, len, hash);
  if(interned != NULL) return PD_FROM(interned);
  pd_str* str = (pd_str*) pd_alloc_object(vm, sizeof(pd_str) + len + 1, PD_OBJ_STRING);
  str->hash = hash;
  pvm_push(vm, PD_FROM(str));
  pd_table_set(vm, &vm->strings, str, NULL_VALUE);
  pvm_pop(vm);
  memcpy(str->bytes, cstr, len);
  str->bytes[len] = '\0';
  str->len = len;
  return PD_FROM(str);
}
