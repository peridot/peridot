#include "object.h"
#include "gc.h"

pd_object* pd_alloc_object(pvm_t* vm, size_t size, pd_object_type type) {
  pd_object* obj = pd_gc_malloc(vm, size);
  obj->type = type;
  // mark_bit is the current true, use ! to flip for the false meaning because objects start out unmarked
  // see details in pvm.h
  obj->mark = !vm->mark_bit;
  obj->next = vm->objects;
  vm->objects = obj;
#ifdef DEBUG_TRACE_GC
  printf("(%p) allocate %ld for %d\n", obj, (unsigned long)size, type);
#endif
  return obj;
}
