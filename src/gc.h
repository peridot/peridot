#ifndef _PERIDOT_GC_H
#define _PERIDOT_GC_H

#include <stdint.h>
#include "pvm.h"

#define PD_FREE_ARRAY(vm, type, pointer, oldCount) pd_gc_realloc(vm, pointer, sizeof(type) * (oldCount), 0)

#define PD_FREE(vm, type, pointer) pd_gc_realloc(vm, pointer, sizeof(type), 0)

#define PD_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define PD_GROW_ARRAY(vm, previous, type, oldCount, count) \
    (type*)pd_gc_realloc(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

// Allocates an object tracked by the GC
// realloc is the actual function, malloc is just a wrapper calling realloc with (NULL, 0, size)
void* pd_gc_malloc(pvm_t* vm, size_t size) __attribute__((__malloc__));
void* pd_gc_realloc(pvm_t* vm, void* previous, size_t oldSize, size_t newSize);

// Frees a GC tracked object manually, used internally in the GC algorithm, not recommended for manual use.
void pd_gc_free_object(pvm_t* vm, pd_object* object);

// Marking functions for the Tri-color algorithm.
void pd_gc_gray_object(pvm_t* vm, pd_object* object);
void pd_gc_gray_value(pvm_t* vm, pd_value value);

// Collects garbage.
void pd_gc_collect(pvm_t* vm);

// Frees all objects, used internally when VM is destroyed to free all objects.
void pd_gc_free_objects(pvm_t* vm);

#endif // _PERIDOT_GC_H
