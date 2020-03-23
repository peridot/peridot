#include <stdio.h>
#include "runtime.h"
#include "str.h"
#include "function.h"
#include <time.h>
#include "pvm.h"
#include "class.h"

// These two print functions are mainly here due to dependency cycles i couldn't think where to put them other than here.
static PD_INLINE void printObj(pd_value value) {
  switch(OBJECT_TYPE(AS_OBJECT(value))) {
    case PD_OBJ_CLASS:
      printf("%s", AS_CLASS(value)->name->bytes);
    case PD_OBJ_CLOSURE:
      printObj(PD_FROM(PD_AS_CLOSURE(value)->function));
      break;
    case PD_OBJ_STRING:
      printf("%s", PD_AS_CSTRING(value));
      break;
    case PD_OBJ_FUNCTION:
      if(PD_AS_FUNCTION(value)->name == NULL) {
        printf("<script>");
        break;
      }
      printf("<function %s at %p>", PD_AS_FUNCTION(value)->name->bytes, PD_AS_FUNCTION(value));
      break;
    case PD_OBJ_NATIVE:
      printf("<native function at %p>", PD_AS_NATIVE(value));
      break;
    case PD_OBJ_UPVALUE:
      printf("upvalue");
      break;
    default:
      pd_unreachable();
  }
}

void pd_value_print(pd_value value) {
  switch(PD_TYPE(value)) {
    case PD_NUMBER:
      printf("%g", AS_DOUBLE(value));
      break;
    case PD_TRUE:
      printf("true");
      break;
    case PD_FALSE:
      printf("false");
      break;
    case PD_NULL:
      printf("null");
      break;
    // Shouldn't ever need to print undefined but we can never know.
    case PD_UNDEFINED:
      printf("<undefined>");
      break;
    case PD_OBJECT:
      return printObj(value);
    default:
      pd_unreachable();
  }
}

// Wraps print_value in a native function to be added in the VM.
pd_value pd_println(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  if(argc < 1) return NULL_VALUE;
  pd_value_print(args[0]);
  printf("\n"); // pd_value_print doesn't print a newline but println builtin should.
  return NULL_VALUE;
}

// Print doesn't put a new line.
pd_value pd_print(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  if(argc > 0) pd_value_print(args[0]);
  return NULL_VALUE;
}

pd_value pd_clock(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  (void)argc;
  (void)args;
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

pd_value pd_gc_heap_size(pvm_t* vm, int argc, pd_value* args) {
  (void)argc;
  (void)args;
  return NUMBER_VAL((double)vm->bytes_allocated);
}
