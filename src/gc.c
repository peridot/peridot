#include "gc.h"
#include <stdlib.h>
#include "runtime.h"
#include "class.h"
#define GC_HEAP_GROW_FACTOR 2

#ifdef DEBUG_TRACE_GC
#include <stdio.h>
#include "debug.h"
#endif

void* pd_gc_realloc(pvm_t* vm, void* previous, size_t oldSize, size_t newSize) {
  vm->bytes_allocated += newSize - oldSize;
  if(newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
    // Stress test GC
    pd_gc_collect(vm);
#endif

    if(vm->bytes_allocated > vm->next_gc) {
      pd_gc_collect(vm);
    }
  }
  
  if(newSize == 0) {
    free(previous);
    return NULL;
  }
  
  return realloc(previous, newSize);
}

// Sugar for calling realloc in a more natural way.
void* pd_gc_malloc(pvm_t* vm, size_t size) {
  return pd_gc_realloc(vm, NULL, 0, size);
}

void pd_gc_gray_object(pvm_t* vm, pd_object* object) {
  if (object == NULL) return;

  // Don't get caught in cycle.
  if(object->mark == vm->mark_bit) return;
#ifdef DEBUG_TRACE_GC
  printf("(%p) gray ", object);
  pd_value_print(PD_FROM(object));
  printf("\n");
#endif
  object->mark = vm->mark_bit;
  if(vm->gray_capacity < vm->gray_count + 1) {
    vm->gray_capacity = PD_GROW_CAPACITY(vm->gray_capacity);
    // Not using pd_gc_realloc() here because we don't want to trigger the GC inside a GC!
    vm->gray_stack = realloc(vm->gray_stack, sizeof(pd_object*) * vm->gray_capacity);
  }
  vm->gray_stack[vm->gray_count++] = object;
}

void pd_gc_gray_value(pvm_t* vm, pd_value value) {
  if(!IS_OBJECT(value)) return;
  pd_gc_gray_object(vm, AS_OBJECT(value));
}

// Walks the linked list of objects and frees them.
void pd_gc_free_objects(pvm_t* vm) {
  pd_object* object = vm->objects;
  while (object != NULL) {
    pd_object* next = object->next;
    pd_gc_free_object(vm, object);
    object = next;
  }
  free(vm->gray_stack);
}

static void pd_gc_gray_array(pvm_t* vm, pd_value_array* array) {
  for (int i = 0; i < array->count; i++) {
    pd_gc_gray_value(vm, array->data[i]);
  }
}

static void pd_gc_table_remove_white(pvm_t* vm, pd_table* table) {
  for(int i = 0; i < table->capacity; i++) {
    pd_table_entry* entry = &table->entries[i];
    if(entry->key != NULL && entry->key->obj.mark != vm->mark_bit) {
      pd_table_delete(table, entry->key);
    }
  }
}

static void pd_gc_gray_table(pvm_t* vm, pd_table* table) {
  for(int i = 0; i < table->capacity; i++) {
    pd_table_entry* entry = &table->entries[i];
    pd_gc_gray_object(vm, (pd_object*)entry->key);
    pd_gc_gray_value(vm, entry->value);
  }
}

static void blackenObject(pvm_t* vm, pd_object* object) {
#ifdef DEBUG_TRACE_GC
  printf("(%p) blacken ", object);
  pd_value_print(PD_FROM(object));
  printf("\n");
#endif
  switch(object->type) {
    case PD_OBJ_CLASS: {
      pd_gc_gray_object(vm, ((pd_object*)(((pd_class*)object)->name)));
      break;
    }
    case PD_OBJ_CLOSURE: {
      pd_closure* closure = (pd_closure*)object;
      pd_gc_gray_object(vm, (pd_object*)closure->function);
      for(int i = 0; i < closure->upvalue_count; i++) {
        pd_gc_gray_object(vm, (pd_object*)closure->upvalues[i]);
      }
      break;
    }
    case PD_OBJ_FUNCTION: {
        pd_function* function = (pd_function*)object;
        pd_gc_gray_object(vm, (pd_object*)function->name);
        pd_gc_gray_array(vm, &function->chunk.constants);
        break;
    }
    case PD_OBJ_UPVALUE:
      pd_gc_gray_value(vm, ((pd_upvalue*)object)->closed);
      break;
    case PD_OBJ_STRING:
    case PD_OBJ_NATIVE:
      // No references.
      break;
  }
/*
  switch (object->type) {
//> Methods and Initializers not-yet
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = (ObjBoundMethod*)object;
      grayValue(bound->receiver);
      grayObject((Obj*)bound->method);
      break;
    }
//< Methods and Initializers not-yet
//> Classes and Instances not-yet

    case OBJ_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      grayObject((Obj*)klass->name);
//> Methods and Initializers not-yet
      grayTable(&klass->methods);
//< Methods and Initializers not-yet
      break;
    }

//< Classes and Instances not-yet
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      grayObject((Obj*)closure->function);
      for (int i = 0; i < closure->upvalueCount; i++) {
        grayObject((Obj*)closure->upvalues[i]);
      }
      break;
    }

    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      grayObject((Obj*)function->name);
      grayArray(&function->chunk.constants);
      break;
    }

//> Classes and Instances not-yet
    case OBJ_INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      grayObject((Obj*)instance->klass);
      grayTable(&instance->fields);
      break;
    }

//< Classes and Instances not-yet
    case OBJ_UPVALUE:
      grayValue(((ObjUpvalue*)object)->closed);
      break;

    case OBJ_NATIVE:
    case OBJ_STRING:
      // No references.
      break;
  }*/
}

void pd_gc_free_object(pvm_t* vm, pd_object* object) {
//> Garbage Collection not-yet
#ifdef DEBUG_TRACE_GC
  printf("(%p) free ", object);
  pd_value_print(PD_FROM(object));
  printf("\n");
#endif

  switch(object->type) {
    case PD_OBJ_CLASS: {
      PD_FREE(vm, pd_class, object);
      break;
    }
    case PD_OBJ_STRING: {
      pd_str* str = (pd_str*) object;
      pd_gc_realloc(vm, object, sizeof(pd_str) + str->len + 1, 0);
      break;
    }
    case PD_OBJ_FUNCTION: {
      pd_function* function = (pd_function*)object;
      pvm_chunk_free(vm, &function->chunk);
      PD_FREE(vm, pd_function, object);
      break;
    }
    case PD_OBJ_NATIVE:
      PD_FREE(vm, pd_native_function, object);
      break;
    case PD_OBJ_UPVALUE:
      PD_FREE(vm, pd_upvalue, object);
      break;
    case PD_OBJ_CLOSURE: {
      pd_closure* closure = (pd_closure*)object;
      PD_FREE_ARRAY(vm, pd_value, closure->upvalues, closure->upvalue_count);
      PD_FREE(vm, pd_closure, object);
      break;
    }
  }
/*
//< Garbage Collection not-yet
  switch (object->type) {
//> Methods and Initializers not-yet
    case OBJ_BOUND_METHOD:
      FREE(ObjBoundMethod, object);
      break;

//< Methods and Initializers not-yet
//> Classes and Instances not-yet
//> Methods and Initializers not-yet
    case OBJ_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      freeTable(&klass->methods);
//< Methods and Initializers not-yet
      FREE(ObjClass, object);
      break;
//> Methods and Initializers not-yet
    }
//< Methods and Initializers not-yet

//< Classes and Instances not-yet
//> Closures free-closure
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
//> free-upvalues
      FREE_ARRAY(Value, closure->upvalues, closure->upvalueCount);
//< free-upvalues
      FREE(ObjClosure, object);
      break;
    }

//< Closures free-closure
//> Calls and Functions free-function
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      freeChunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }

//< Calls and Functions free-function
//> Classes and Instances not-yet
    case OBJ_INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      freeTable(&instance->fields);
      FREE(ObjInstance, object);
      break;
    }

//< Classes and Instances not-yet
//> Calls and Functions free-native
    case OBJ_NATIVE:
      FREE(ObjNative, object);
      break;

//< Calls and Functions free-native
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
//> Closures free-upvalue

    case OBJ_UPVALUE:
      FREE(ObjUpvalue, object);
      break;
//< Closures free-upvalue
  } */
}

void pd_gc_collect(pvm_t* vm) {
#ifdef DEBUG_TRACE_GC
  printf("-- gc begin\n");
  size_t before = vm->bytes_allocated;
  PD_TIMER_START;
#endif

  // Mark the stack roots.
  for (pd_value* slot = vm->stack; slot < vm->stack_top; slot++) {
    pd_gc_gray_value(vm, *slot);
  }

  for (int i = 0; i < vm->frame_count; i++) {
    pd_gc_gray_object(vm, (pd_object*)vm->frames[i].closure);
  }

  // Mark the open upvalues.
  for(pd_upvalue* upvalue = vm->open_upvalues; upvalue != NULL; upvalue = upvalue->next) {
    pd_gc_gray_object(vm, (pd_object*)upvalue);
  }

  // Mark the global roots.
  pd_gc_gray_table(vm, &vm->globals);
  pd_gc_gray_array(vm, &vm->global_values);

  // Mark the compiler roots, the compiler also creates objects that could trigger the GC.
  // If we don't handle that the GC will free them before the code even runs.
  pd_code_ctx* compiler = vm->compiler;
  while(compiler != NULL) {
    pd_gc_gray_object(vm, (pd_object*)compiler->function);
    compiler = compiler->enclosing;
  }

//> Methods and Initializers not-yet
  //grayObject((Obj*)vm.initString);
//< Methods and Initializers not-yet

  // Traverse the references.
  while (vm->gray_count > 0) {
    // Pop an item from the gray stack.
    pd_object* object = vm->gray_stack[--vm->gray_count];
    blackenObject(vm, object);
  }

  // Delete unused interned strings.
  pd_gc_table_remove_white(vm, &vm->strings);

  // Collect the white objects.
  pd_object** object = &vm->objects;
  while(*object != NULL) {
    if((*object)->mark != vm->mark_bit) {
      // This object wasn't reached, so remove it from the list and
      // free it.
      pd_object* unreached = *object;
      *object = unreached->next;
      pd_gc_free_object(vm, unreached);
    } else {
      // Move on to the next.
      object = &(*object)->next;
    }
  }

  // Adjust the heap size based on live memory.
  vm->next_gc = vm->bytes_allocated * GC_HEAP_GROW_FACTOR;
  // Flip the meaning of mark_bit (see pvm.h for details)
  vm->mark_bit = !vm->mark_bit;

#ifdef DEBUG_TRACE_GC
  PD_TIMER_STOP;
  printf("-- gc collected %ld bytes (from %ld to %ld) next at %ld (took %.3fs)\n",
         (unsigned long)before - vm->bytes_allocated, (unsigned long)before, (unsigned long)vm->bytes_allocated,
         (unsigned long)vm->next_gc, elapsed);
#endif
}
