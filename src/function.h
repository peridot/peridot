#ifndef _PERIDOT_FUNCTION_H
#define _PERIDOT_FUNCTION_H

#include "object.h"
#include "chunk.h"
#include "str.h"

typedef struct {
  pd_object obj;
  int arity;
  int upvalue_count;
  pvm_chunk chunk;
  pd_str* name;
  int scope; // Scope depth of this function.
} pd_function;

typedef pd_value (*pd_native)(pvm_t* vm, int argc, pd_value* args);

typedef struct {
  pd_object obj;
  pd_native function;
} pd_native_function;

typedef struct pd_upvalue {
  pd_object obj;
  pd_value* location;
  pd_value closed;
  struct pd_upvalue* next;
} pd_upvalue;

typedef struct {
  pd_object obj;
  pd_function* function;
  pd_upvalue** upvalues;
  int upvalue_count;
} pd_closure;

pd_function* pd_function_new(pvm_t* vm);
pd_closure* pd_closure_new(pvm_t* vm, pd_function* fn);
pd_native_function* pd_native_function_new(pvm_t* vm, pd_native function);
pd_upvalue* pd_upvalue_new(pvm_t* vm, pd_value* slot);

#define PD_IS_FUNCTION(fn) (OBJECT_TYPE(AS_OBJECT(fn)) == PD_OBJ_FUNCTION)
#define PD_AS_FUNCTION(fn) ((pd_function*)AS_OBJECT(fn))

#define PD_IS_NATIVE(fn) (OBJECT_TYPE(AS_OBJECT(fn)) == PD_OBJ_NATIVE)
#define PD_AS_NATIVE(fn) (((pd_native_function*)AS_OBJECT(fn))->function)

#define PD_IS_CLOSURE(fn) (OBJECT_TYPE(AS_OBJECT(fn)) == PD_OBJ_CLOSURE)
#define PD_AS_CLOSURE(fn) ((pd_closure*)AS_OBJECT(fn))

#endif // _PERIDOT_FUNCTION_H
