#ifndef _PERIDOT_PVM_H
#define _PERIDOT_PVM_H

// Peridot's Virtual Machine for executing bytecode
// in the future this will be able to do basic Just in time compilation for faster speed.

// The max stack size before overflowing.
#define PVM_FRAMES_MAX 64
#define PVM_STACK_MAX (PVM_FRAMES_MAX * 256)

#include <stdint.h>
#include "value.h"
#include "table.h"
#include "function.h"
#include "compiler.h"

// TEMP TRY
#include <uv.h>

// Represents a call frame in the VM.
typedef struct {
  pd_closure* closure;
  uint8_t* ip;
  pd_value* slots;
} pvm_frame;

typedef struct pvm_t {
  // Stores the inner most compiler, this is to keep track of values allocated during compile time
  // To avoid freeing values at compile time, (i.e the functions/strings)
  // This is set back to NULL at runtime.
  pd_code_ctx* compiler;
  // Stack stuff.
  pd_value stack[PVM_STACK_MAX];
  pd_value* stack_top;

  pd_upvalue* open_upvalues;

  // Instructions stuff
  pvm_frame frames[PVM_FRAMES_MAX];
  int frame_count;

  // GC stuff.
  size_t bytes_allocated;
  size_t next_gc;
  int gray_count;
  int gray_capacity;
  pd_object** gray_stack;

  // This bit is used to represent that an object is marked from GC.
  // After sweeping we flip this bit which flips the meaning of marked objects to mean unmarked.
  // Which is cheaper than flipping every object's mark bit.
  //
  // To make it clearer, think this is always the boolean value true
  // but internally the meaning gets flipped and everywhere this bool was used will be treated as false.
  bool mark_bit;

  // Linked list of all alive objects.
  pd_object* objects;

  // Interned strings.
  pd_table strings;

  // Global variables
  // Globals maps name to the index for global_values
  // It is used at compile time to store globals and emit instructions with the index
  // So at runtime there is no overhead for hash table lookup, it's just a direct index to the values array
  //
  // One might ask why globals isn't defined inside compiler if it's only used at compile time
  // That's to ease repl implementation because repl runs compiler on every line so we will lose the globals information
  // so we store it in VM to persist it across repl lines.
  pd_table globals;
  pd_value_array global_values;

  // The libuv event-loop.
  // TODO: We will always use uv_default_loop() pretty much, do we need this?
  uv_loop_t* loop;
} pvm_t;

void pvm_push(pvm_t* vm, pd_value value);
pd_value pvm_pop(pvm_t* vm);

pvm_t* pvm_new();
void pvm_run(pvm_t* vm);
void pvm_free(pvm_t* vm);
void pvm_exec(pvm_t* vm, pd_function* fn);
void pvm_define_function(pvm_t* vm, char* name, pd_native fn);
bool pvm_call(pvm_t* vm, pd_value value, int argc);

#endif // _PERIDOT_PVM_H
