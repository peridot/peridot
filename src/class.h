#ifndef _PERIDOT_CLASS_H
#define _PERIDOT_CLASS_H

#include "object.h"
#include "str.h"

typedef struct {
  pd_object obj;
  pd_str* name;
} pd_class;

#define IS_CLASS(val) (OBJECT_TYPE(AS_OBJECT(val)) == PD_OBJ_CLASS)
#define AS_CLASS(val) ((pd_class*)AS_OBJECT(val))

pd_class* pd_class_new(pvm_t* vm, pd_str* name);

#endif // _PERIDOT_CLASS_H
