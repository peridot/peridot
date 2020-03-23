#ifndef _PERIDOT_RUNTIME_H
#define _PERIDOT_RUNTIME_H

#include "value.h"

// Functions callable at runtime

// Prints a value to stdout.
void pd_value_print(pd_value value);
void pvm_init_builtins(pvm_t* vm);

/*
pd_value pd_println(pvm_t* vm, int argc, pd_value* args);
pd_value pd_print(pvm_t* vm, int argc, pd_value* args);
pd_value pd_clock(pvm_t* vm, int argc, pd_value* args);
pd_value pd_gc_heap_size(pvm_t* vm, int argc, pd_value* args);
*/

#endif // _PERIDOT_RUNTIME_H
