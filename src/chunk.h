#ifndef _PERIDOT_CHUNK_H
#define _PERIDOT_CHUNK_H

#include "value.h"

typedef struct pvm_t pvm_t;

// Chunk stores a chunk of instructions to be executed by the VM
// Each function holds its own chunk of code.
// TODO: reuse this to implement disassembly in language itself.
// Since functions are objects something like fn.disasm() should do.

typedef struct pvm_chunk {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  pd_value_array constants;
} pvm_chunk;

void pvm_chunk_init(pvm_chunk* chunk);
void pvm_chunk_free(pvm_t* vm, pvm_chunk* chunk);
void pvm_chunk_write(pvm_t* vm, pvm_chunk* chunk, uint8_t byte, int line);
int pvm_add_constant(pvm_t* vm, pvm_chunk* chunk, pd_value value);

#endif // _PERIDOT_CHUNK_H
