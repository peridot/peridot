#include <stdio.h>

#include "debug.h"

#include "value.h"
#include "chunk.h"
#include "runtime.h"
#include "opcodes.h"
#include "function.h"

// TODO: this module uses ANSI escapes that would break the output on Windows terminals.


void pvm_disassemble_chunk(pvm_chunk* chunk, const char* name) {
  printf("\x1b[34m== \x1b[0m\x1b[32m\x1b[1m%s\x1b[0m\x1b[34m ==\x1b[0m\n", name);

  for (int i = 0; i < chunk->count;) {
    i = pvm_disassemble_instruction(chunk, i);
  }
}

static int constantInstruction(const char* name, pvm_chunk* chunk,
                               int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("\x1b[33m%-16s\x1b[0m %4d '", name, constant);
  pd_value_print(chunk->constants.data[constant]);
  printf("'\n");
  return offset + 2;
}

static int longConstantInstruction(const char* name, pvm_chunk* chunk, int offset) {
  uint16_t constant = chunk->code[offset + 1] | (chunk->code[offset + 2] << 8);
  printf("%-16s %4d '", name, constant);
  pd_value_print(chunk->constants.data[constant]);
  printf("'\n");
  return offset + 3;
}

static int simpleInstruction(const char* name, int offset) {
  printf("\x1b[33m%s\x1b[0m\n", name);
  return offset + 1;
}

static int jumpInstruction(const char* name, int sign, pvm_chunk* chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1]);
  jump |= chunk->code[offset + 2] << 8;
  printf("\x1b[33m%-16s\x1b[0m %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

static int byteInstruction(const char* name, pvm_chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("\x1b[33m%-16s\x1b[0m %4d\n", name, slot);
  return offset + 2;
}

int pvm_disassemble_instruction(pvm_chunk* chunk, int offset) {
  printf("\x1b[1m\x1b[36m%04d\x1b[0m ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   \x1b[32m|\x1b[0m ");
  } else {
    printf("\x1b[35m\x1b[1m%4d\x1b[0m ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case PVM_OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case PVM_OP_CONSTANT_LONG:
      return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
    case PVM_OP_GET_GLOBAL:
      return byteInstruction("OP_GET_GLOBAL", chunk, offset);
    case PVM_OP_SET_GLOBAL:
      return byteInstruction("OP_SET_GLOBAL", chunk, offset);
    case PVM_OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case PVM_OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case PVM_OP_GET_UPVALUE:
      return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case PVM_OP_SET_UPVALUE:
      return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case PVM_OP_JUMP:
      return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case PVM_OP_CLOSE_UPVALUE:                                 
      return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case PVM_OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case PVM_OP_AND:
      return jumpInstruction("OP_AND", 1, chunk, offset);
    case PVM_OP_OR:
      return jumpInstruction("OP_OR", 1, chunk, offset);
    case PVM_OP_LOOP:                                                  
      return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case PVM_OP_POP:
      return simpleInstruction("OP_POP", offset);
    case PVM_OP_POPN:
      return byteInstruction("OP_POPN", chunk, offset);
    case PVM_OP_NULL:
      return simpleInstruction("OP_NULL", offset);
    case PVM_OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case PVM_OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case PVM_OP_PUSH_NEG_ONE:
      return simpleInstruction("OP_PUSH_NEG_ONE", offset);
    case PVM_OP_PUSH_ZERO:
      return simpleInstruction("OP_PUSH_ZERO", offset);
    case PVM_OP_PUSH_ONE:
      return simpleInstruction("OP_PUSH_ONE", offset);
    case PVM_OP_PUSH_TWO:
      return simpleInstruction("OP_PUSH_TWO", offset);
    case PVM_OP_PUSH_THREE:
      return simpleInstruction("OP_PUSH_THREE", offset);
    case PVM_OP_PUSH_FOUR:
      return simpleInstruction("OP_PUSH_FOUR", offset);
    case PVM_OP_PUSH_FIVE:
      return simpleInstruction("OP_PUSH_FIVE", offset);
    case PVM_OP_EQ:
      return simpleInstruction("OP_EQ", offset);
    case PVM_OP_NEQ:
      return simpleInstruction("OP_NEQ", offset);
    case PVM_OP_GT:
      return simpleInstruction("OP_GT", offset);
    case PVM_OP_LT:
      return simpleInstruction("OP_LT", offset);
    case PVM_OP_GE:
      return simpleInstruction("OP_GE", offset);
    case PVM_OP_LE:
      return simpleInstruction("OP_LE", offset);
    case PVM_OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case PVM_OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case PVM_OP_SHR:
      return simpleInstruction("OP_SHR", offset);
    case PVM_OP_SHL:
      return simpleInstruction("OP_SHL", offset);
    case PVM_OP_BOR:
      return simpleInstruction("OP_BOR", offset);
    case PVM_OP_XOR:
      return simpleInstruction("OP_XOR", offset);
    case PVM_OP_BNOT:
      return simpleInstruction("OP_BNOT", offset);
    case PVM_OP_BAND:
      return simpleInstruction("OP_BAND", offset);
    case PVM_OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case PVM_OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case PVM_OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case PVM_OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case PVM_OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case PVM_OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case PVM_OP_RETURN_NULL:
      return simpleInstruction("OP_RETURN_NULL", offset);
    case PVM_OP_CLOSURE: {
      offset++;
      uint8_t constant = chunk->code[offset++];
      printf("\x1b[33m%-16s\x1b[0m %4d ", "OP_CLOSURE", constant);
      pd_value_print(chunk->constants.data[constant]);
      printf("\n");
      pd_function* function = PD_AS_FUNCTION(chunk->constants.data[constant]);
      for(int j = 0; j < function->upvalue_count; j++) {
        int isLocal = chunk->code[offset++];
        int index = chunk->code[offset++];
        printf("\x1b[1m\x1b[36m%04d\x1b[0m      \x1b[32m|\x1b[0m                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
      }
      return offset;
    }
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}
