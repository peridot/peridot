#include "class.h"

pd_class* pd_class_new(pvm_t* vm, pd_str* name) {
  pd_class* klass = ALLOC_OBJECT(vm, pd_class, PD_OBJ_CLASS);
  klass->name = name;

  return klass;
}
