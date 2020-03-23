#ifndef _PERIDOT_OBJECT_H
#define _PERIDOT_OBJECT_H

#include <stdint.h>
#include <stdbool.h>

// Some dependency cycles.
typedef struct pvm_t pvm_t;

// Types that objects can be, this includes builtin types
// User defined objects (classes) are represented in a single type.
// TODO: More types.
typedef enum {
  PD_OBJ_STRING, // "Strings"
  PD_OBJ_FUNCTION, // Functions are first class in Peridot.
  PD_OBJ_NATIVE, // Native C functions.
  PD_OBJ_CLASS,
  PD_OBJ_CLOSURE, // Closure
  PD_OBJ_UPVALUE // Captured variable.
} pd_object_type;

// The object struct
// Objects are tracked by the garbage collector.
// This is the base class, that is boxed as a value and passed around
// The actual object will be a "sub-struct" of this type that has to be cast to in order to use it.
// The type field represents the type that the object can be safely cast to.
// TODO: When we have classes add a class field here, because every builtin should have a class usable by the user.
typedef struct pd_object {
  pd_object_type type; // The type of the object.

  // Wether this object is marked but instead of true meaning marked the true value to compare
  // is the "mark_bit" in the VM that represents the current true, see the comments in pvm.h for details.
  bool mark;
  struct pd_object* next; // The next object in this linked list.
} pd_object;

// Get type of an object, casts to (pd_object*) because this can also be used on subclasses of object.
// the type is stored on the base.
#define OBJECT_TYPE(v) (((pd_object*)(v))->type)

// Allocate an object
// Because we are doing this in a "struct-inheritance" way, the actual size to allocate is unknown
// So we ask for the size too.
pd_object* pd_alloc_object(pvm_t* vm, size_t size, pd_object_type type);

// Wrapper macro that allocates and casts.
#define ALLOC_OBJECT(vm, type, objty) ((type*) pd_alloc_object(vm, sizeof(type), objty))

#endif // _PERIDOT_OBJECT_H
