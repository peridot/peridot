#ifndef _PERIDOT_DYN_ARR_H
#define _PERIDOT_DYN_ARR_H

// Generates a dynamic array data structure for a given type, since we don't have generics in C this will do the trick.
// This file expects that pvm.h is included for declare and gc.h for define

#define PD_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define PD_GROW_ARRAY(vm, previous, type, oldCount, count) \
    (type*)pd_gc_realloc(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

// Declares the functions, use this in a header file.
#define PERIDOT_DEC_DYN_ARR(name, type) \
  typedef struct { \
    type *data; \
    PERIDOT_DYN_ARR_FIELDS \
  } name; \
  void name##_init(name* arr); \
  void name##_write(pvm_t* vm, name* arr, type data); \
  void name##_clear(pvm_t* vm, name* arr);

// Includes the fields a dyn arr needs useful if you need more fields in the struct.
#define PERIDOT_DYN_ARR_FIELDS int capacity; int count;

// Implements the functions for the dynarr, use this in a source file.
#define PERIDOT_DEF_DYN_ARR(name, type) \
  void name##_init(name* arr) { \
    arr->data = NULL; \
    arr->capacity = 0; \
    arr->count = 0; \
  } \
  void name##_write(pvm_t* vm, name* arr, type data) { \
    if(arr->capacity < arr->count + 1) { \
      int oldCapacity = arr->capacity; \
      arr->capacity = PD_GROW_CAPACITY(oldCapacity); \
      arr->data = PD_GROW_ARRAY(vm, arr->data, type, \
        oldCapacity, arr->capacity); \
    } \
    arr->data[arr->count++] = data; \
  } \
  void name##_clear(pvm_t* vm, name* arr) { \
    PD_FREE_ARRAY(vm, type, arr->data, arr->capacity); \
    name##_init(arr); \
  }

#endif // _PERIDOT_DYN_ARR_H
