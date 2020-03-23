#ifndef _PERIDOT_COMPILER_H
#define _PERIDOT_COMPILER_H

#include "chunk.h"
#include "ast.h"
#include "function.h"

/*
 * Peridot compiler pipeline
 * Source -> Lexer -> Tokens -> Parser -> AST -> Compiler -> Bytecode -> VM
 * This file is the compiler part and will emit bytecode for the VM to execute.
 */

typedef struct {
  char* name;
  size_t len; // length of name.
  int depth;
  bool isCaptured;
} pd_compiler_local;

typedef struct {
  uint8_t index;
  bool isLocal; 
} pd_compiler_upvalue;

typedef enum {
  PD_TYPE_FUNCTION,
  PD_TYPE_SCRIPT
} pd_function_type;

// Holds some contextual data to be used throughout the compilation.
typedef struct pd_code_ctx {
  pvm_t* vm; // The VM, The VM is needed to keep track of chunk allocations.

  pd_function* function;    
  pd_function_type type;

  struct pd_code_ctx* enclosing;

  // Line number of the current node we are compiling.
  // This will be 0 for internal code, i.e code implicitly inserted by the compiler that doesn't mirror the source code.
  int line;

  // Local variables information.
  pd_compiler_local locals[256];
  int localCount;
  pd_compiler_upvalue upvalues[256];
  // Depth of the scope, 0 is the global scope, 1 is an inner block 2 is a nested inner block and so on.
  int scopeDepth;

  int fn; // inside a top-level of a function.

  // Tracks how many compile errors we have seen.
  // We shall not proceed to executing if there are errors.
  int errors;

  // Signals the compiler to not pop the top of the value when compiling an expression statement
  // Used for variable assignment because we don't have a declare statement.
  int nopop;
} pd_code_ctx;

// Compiles the root [node] in context of [ctx]
// Writes bytecode to [ctx.chunk]
void pd_compile(pd_code_ctx* ctx, pd_ast_node* node);
void pd_compile_ctx_init(pd_code_ctx* ctx, pvm_t* vm, pd_function_type type);
pd_function* pd_compile_ctx_end(pd_code_ctx* ctx);

#endif // _PERIDOT_COMPILER_H
