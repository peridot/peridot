#include <stdio.h>
#include <string.h>
#include "object.h"
#include "value.h"
#include "gc.h"

PERIDOT_DEF_DYN_ARR(pd_value_array, pd_value)

// TODO: kinda useless and doesn't follow the pd_ prefix rule.
bool valuesEqual(pd_value a, pd_value b) {
  return a == b;
/*
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
//> Strings strings-equal
    case VAL_OBJ:
//< Strings strings-equal
    {
      ObjString* aString = AS_STRING(a);
      ObjString* bString = AS_STRING(b);
      return aString->length == bString->length &&
          memcmp(aString->chars, bString->chars, aString->length) == 0;
    }
//> Hash Tables equal
      return AS_OBJECT(a) == AS_OBJECT(b);
//< Hash Tables equal
  }
//> Optimization not-yet
*/
}
//< Types of Values values-equal
