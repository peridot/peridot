#include "function.h"
#include "gc.h"

pd_function* pd_function_new(pvm_t* vm) {
  pd_function* fn = ALLOC_OBJECT(vm, pd_function, PD_OBJ_FUNCTION);
  fn->arity = 0;
  fn->upvalue_count = 0;
  fn->name = NULL;
  fn->scope = 0;
  pvm_chunk_init(&fn->chunk);
  return fn;
}

pd_native_function* pd_native_function_new(pvm_t* vm, pd_native function) {
  pd_native_function* native = ALLOC_OBJECT(vm, pd_native_function, PD_OBJ_NATIVE);
  native->function = function;
  return native;
}

pd_closure* pd_closure_new(pvm_t* vm, pd_function* fn) {
  pd_upvalue** upvalues = pd_gc_malloc(vm, sizeof(pd_upvalue*) * fn->upvalue_count);
  for(int i = 0; i < fn->upvalue_count; i++) {
    upvalues[i] = NULL;
  }
  pd_closure* closure = ALLOC_OBJECT(vm, pd_closure, PD_OBJ_CLOSURE);
  closure->function = fn;
  closure->upvalues = upvalues;
  closure->upvalue_count = fn->upvalue_count;
  return closure;
}

pd_upvalue* pd_upvalue_new(pvm_t* vm, pd_value* slot) {
  pd_upvalue* upvalue = ALLOC_OBJECT(vm, pd_upvalue, PD_OBJ_UPVALUE);
  upvalue->location = slot;
  upvalue->next = NULL;
  upvalue->closed = NULL_VALUE;
  return upvalue;
}
