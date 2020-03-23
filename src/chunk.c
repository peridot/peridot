#include <stdlib.h>

#include "chunk.h"
#include "pvm.h"
#include "value.h"
#include "gc.h"

// Chunk is currently the only exception where we don't use dyn_arr.h to define it
// This is because we need more fields than what a dynarr gives us but in the future this might get rewritten

void pvm_chunk_init(pvm_chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  pd_value_array_init(&chunk->constants);
}

void pvm_chunk_free(pvm_t* vm, pvm_chunk* chunk) {
  PD_FREE_ARRAY(vm, uint8_t, chunk->code, chunk->capacity);
  PD_FREE_ARRAY(vm, int, chunk->lines, chunk->capacity);
  pd_value_array_clear(vm, &chunk->constants);
  pvm_chunk_init(chunk);
}

void pvm_chunk_write(pvm_t* vm, pvm_chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = PD_GROW_CAPACITY(oldCapacity);
    chunk->code = PD_GROW_ARRAY(vm, chunk->code, uint8_t,
        oldCapacity, chunk->capacity);
    chunk->lines = PD_GROW_ARRAY(vm, chunk->lines, int,
        oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

int pvm_add_constant(pvm_t* vm, pvm_chunk* chunk, pd_value value) {
  // Make sure the value doesn't get collected when resizing the array.
  pvm_push(vm, value);
  pd_value_array_write(vm, &chunk->constants, value);
  pvm_pop(vm);
  return chunk->constants.count - 1;
}
//< add-constant
