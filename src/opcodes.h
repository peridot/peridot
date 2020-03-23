#ifndef _PERIDOT_OPCODES_H
#define _PERIDOT_OPCODES_H

// This file defines all VM opcodes and documents them.
// Being in a seperate file helps people to include the opcodes seperately if they for example want to write a custom frontend.
// Currently not all opcodes are implemented in the VM.
// The instruction set is little-endian

typedef enum {
  // Loads a constant
  // OP_CONSTANT <constant index>
  PVM_OP_CONSTANT,

  // Loads a 2 byte long constant
  // OP_CONSTANT <byte 1> <byte 2>
  // The bytes are in little-endian order.
  PVM_OP_CONSTANT_LONG,

  // Pushes NULL on the stack.
  PVM_OP_NULL,

  // Pushes True on the stack.
  PVM_OP_TRUE,
  
  // Pushes false on the stack.
  PVM_OP_FALSE,

  // Pops the top of stack.
  PVM_OP_POP,
  // Like pop but pops n times, useful to reduce multiple pops into a single instruction.
  PVM_OP_POPN,

  // Get/Set a local variable.
  PVM_OP_GET_LOCAL,
  PVM_OP_SET_LOCAL,

  // Get/Set/Define global variables.
  PVM_OP_GET_GLOBAL,
  PVM_OP_DEFINE_GLOBAL,
  PVM_OP_SET_GLOBAL,

  // Get/Set upvalues (closures)
  PVM_OP_GET_UPVALUE,
  PVM_OP_SET_UPVALUE,

  // Get/Set properties.
  PVM_OP_GET_PROPERTY,
  PVM_OP_SET_PROPERTY,

  // Get the Super class.
  PVM_OP_GET_SUPER,

  // Comparision ops
  PVM_OP_EQ, // ==
  PVM_OP_NEQ, // !=
  PVM_OP_GT, // >
  PVM_OP_LT, // <
  PVM_OP_LE, // <=
  PVM_OP_GE, // >=

  // Logical and (&&) and or (||)
  PVM_OP_AND,
  PVM_OP_OR,

  // Binary ops.
  // Binary +
  PVM_OP_ADD,
  // Binary -
  PVM_OP_SUBTRACT,
  // Binary *
  PVM_OP_MULTIPLY,
  // Binary /
  PVM_OP_DIVIDE,

  // Unary !
  PVM_OP_NOT,
  // Unary -
  PVM_OP_NEGATE,

  // Bitwise <<
  PVM_OP_SHL,
  // Bitwise >>
  PVM_OP_SHR,
  // Bitwise &
  PVM_OP_BAND,
  // Bitwise |
  PVM_OP_BOR,
  // Bitwise ^
  PVM_OP_XOR,
  // Bitwise ~
  PVM_OP_BNOT,

  // Jumps and loops
  PVM_OP_JUMP,
  PVM_OP_JUMP_IF_FALSE,
  PVM_OP_LOOP,

  // Calls and methods.
  PVM_OP_CALL,
  PVM_OP_INVOKE,
  PVM_OP_SUPER,
  // Closures
  PVM_OP_CLOSURE,
  PVM_OP_CLOSE_UPVALUE,
  // Returns a value from the current function frame.
  PVM_OP_RETURN,
  // Like return but implicitly returns null.
  PVM_OP_RETURN_NULL,
  PVM_OP_CLASS,
  PVM_OP_INHERIT,
  PVM_OP_METHOD,

  // We have a lot of opcodes space so let's optimize some common cases with a single instruction dedicated for them.
  // Dispatch is a bit expensive so it's a huge performance boost to reduce instructions to be as little dispatches as possible.
  // This instructions pushes numbers from -1 to 5, saving a constant slot and load.
  // This might get removed if we ever need way more instructions but that's unlikely.
  PVM_OP_PUSH_NEG_ONE, // I don't think negative zero (-0) is needed? I see -1 a lot but never -0 in most programs.
  PVM_OP_PUSH_ZERO,
  PVM_OP_PUSH_ONE,
  PVM_OP_PUSH_TWO,
  PVM_OP_PUSH_THREE,
  PVM_OP_PUSH_FOUR,
  PVM_OP_PUSH_FIVE
} pvm_opcode;

#endif // _PERIDOT_OPCODES_H
