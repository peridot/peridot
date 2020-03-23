#ifndef _PERIDOT_STR_H
#define _PERIDOT_STR_H

#include <stdint.h>
#include "object.h"
#include "value.h"

// String type, inherits from object
typedef struct {
  pd_object obj;
  size_t len;
  uint32_t hash; // Cache the hash.
  char bytes[];
} pd_str;

// Create a new string.
pd_value pd_str_new(pvm_t* vm, char* cstr, size_t len);

// Cast macros.
#define PD_AS_STRING(val) ((pd_str*)AS_OBJECT(val))
#define PD_AS_CSTRING(val) (PD_AS_STRING(val)->bytes)
#define PD_STRLEN(val) (PD_AS_STRING(val)->len)
#define PD_IS_STRING(val) (OBJECT_TYPE(AS_OBJECT(val)) == PD_OBJ_STRING)

#endif // _PERIDOT_STR_H
