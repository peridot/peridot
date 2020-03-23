#include <stdlib.h>
#include "compiler.h"
#include "opcodes.h"
#include "value.h"
#include "str.h"
#include "pvm.h"

// TODO: The compiler is extremely messed up.
// I just wanted to quickly get some progress, this will need a massive cleanup soon.
// Also read the TODO comments in each section of this file to get a better idea of what needs to be cleaned up.

// Given a context, returns the chunk for writing the resulting bytecode.
static pvm_chunk* currentChunk(pd_code_ctx* ctx) {
  return &ctx->function->chunk;
}

// Emits a byte to the current chunk.
static void emitByte(pd_code_ctx* ctx, uint8_t byte) {
  pvm_chunk_write(ctx->vm, currentChunk(ctx), byte, ctx->line);
}

// Helper to emit 2 bytes.
static void emitBytes(pd_code_ctx* ctx, uint8_t byte1, uint8_t byte2) {
  emitByte(ctx, byte1);
  emitByte(ctx, byte2);
}

// Add a compile error, at end of compilation these errors are reported and code won't execute.
static void error(pd_code_ctx* ctx, const char* format, ...) {
  va_list args;
  fprintf(stderr, "[line %d] Error: ", ctx->line);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);
  ctx->errors++;
}

// Emits an empty return. TODO: just why have this?
static void emitReturn(pd_code_ctx* ctx) {
  emitByte(ctx, PVM_OP_RETURN_NULL);
}

// Creates a constant and returns the index to the constant table.
// Constants are a byte (uint8_t)
// When it exceeds that amount the upcoming constants are treated as long constants
// Which is 2 bytes (uint16_t)
// This means the maximum number of constants we can store is 65536 constants. (UINT16_MAX)
static uint16_t makeConstant(pd_code_ctx* ctx, pd_value value) {
  int constant = pvm_add_constant(ctx->vm, currentChunk(ctx), value);
  pd_assert(constant < UINT16_MAX, "Too many constants.");
  return (uint16_t)constant;
}

// Emits a constant load instruction in the code.
// This calls makeConstant() to create the constant index.
// Then it checks the size (a byte or two?) 
// and emits the appropriate instruction for loading it (constant or long constant?)
// Long constants splits the bytes into two operands encoded in Little-Endian format.
static void emitConstant(pd_code_ctx* ctx, pd_value value) {
  uint16_t constant = makeConstant(ctx, value);
  if(constant < UINT8_MAX) {
    emitBytes(ctx, PVM_OP_CONSTANT, constant);
  } else {
    emitByte(ctx, PVM_OP_CONSTANT_LONG);
    emitBytes(ctx, constant & 0xFF, (constant >> 8) & 0xFF);
  }
}

// TODO: handle long constants.
// For now we are limited to 256 globals per compiler.
// Also mirror the same change at defineNative() in the VM.
static uint8_t identifierConstant(pd_code_ctx* ctx, char* name, size_t len) {
  pd_value index;
  pd_str* identifier = PD_AS_STRING(pd_str_new(ctx->vm, name, len));
  if (pd_table_get(&ctx->vm->globals, identifier, &index)) {
    return (uint8_t)AS_DOUBLE(index);
  }

  pd_value_array_write(ctx->vm, &ctx->vm->global_values, UNDEFINED_VALUE);
  uint8_t newIndex = (uint8_t)ctx->vm->global_values.count - 1;

  pd_table_set(ctx->vm, &ctx->vm->globals, identifier, NUMBER_VAL((double)newIndex));
  return newIndex;
}

// TODO: deprecate this? i have a little idea to handle scopes in a better way, more info soon.
static void beginScope(pd_code_ctx* ctx) {
  ctx->scopeDepth++;
}

// TODO: get rid of this if we don't need it. (see beginScope())
// Though i'm not sure about the OP_CLOSE_UPVALUE part if i do it the other way.
void endScope(pd_code_ctx* ctx) {
  ctx->scopeDepth--;
  //int pop = 0;
  while(ctx->localCount > 0 && ctx->locals[ctx->localCount - 1].depth > ctx->scopeDepth) {
    if (ctx->locals[ctx->localCount - 1].isCaptured) {
      emitByte(ctx, PVM_OP_CLOSE_UPVALUE);
    } else {
      emitByte(ctx, PVM_OP_POP);
    }
    ctx->localCount--;
    //pop++;
  }
  // Optimize popping
  // When we go out of scope all locals get popped, dispatch is expensive so we avoid emitting multiple pop instructions
  // and reduce them to a single POPN but if we only have to pop a single value then
  // A single pop is better than popn 1 since it doesn't need an operand and results in faster execution.
  // TODO: Craftinginterpreters makes it harder to follow the book if you add optimizations early.
  // turns out when adding closures we need a conditional pop, damn it.
  /*if(pop == 1) emitByte(ctx, PVM_OP_POP);
  else if(pop != 0) emitBytes(ctx, PVM_OP_POPN, pop);*/
}                         

// Adds a local variable given a [name]
static void addLocal(pd_code_ctx* ctx, char* name) {
  if (ctx->localCount == 256) {
    error(ctx, "Too many local variables in function.");
    return;
  }
  pd_compiler_local* local = &ctx->locals[ctx->localCount++];
  local->name = name;
  // TODO get length info from the parser.
  local->len = strlen(name);
  local->depth = -1;
  local->isCaptured = false;
}

// Checks if the two identifiers are equal.
// Since this is used before the string interning optimization we need to compare the whole string.
static bool identifiersEqual(char* a, size_t alen, char* b, size_t blen) {
  if(alen != blen) return false;
  return memcmp(a, b, alen) == 0;
}

// Resolves a local variable.
static int resolveLocal(pd_code_ctx* ctx, char* name, size_t len) {
  for(int i = ctx->localCount - 1; i >= 0; i--) {
    pd_compiler_local* local = &ctx->locals[i];
    if(identifiersEqual(name, len, local->name, local->len)) {
      if (local->depth == -1) {
        error(ctx, "Cannot read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

// Declares a local variable.
static bool declareLocal(pd_code_ctx* ctx, char* name, size_t len) {
  bool found = 0;
  for(int i = ctx->localCount - 1; i >= 0; i--) {
    pd_compiler_local* local = &ctx->locals[i];
    if(local->depth != -1 && local->depth < ctx->scopeDepth) {
      break;
    }
    
    if(identifiersEqual(name, len, local->name, local->len)) {
      found = true;
      break;
    }
  }
  if(!found) addLocal(ctx, name);
  return !found;
}

// Marks a variable initialized.
// This is used to avoid code like `a = a`
static void markInitialized(pd_code_ctx* ctx) {
  if(ctx->scopeDepth == 0) return;
  ctx->locals[ctx->localCount - 1].depth = ctx->scopeDepth;
}

// Emits a jump instruction and returns the offset of the instruction.
// Jumps are done by emitting a placeholder offset
// and returning back and patch the offsets to the correct one.
// This is because we don't know how long the code to jump over is.
// Jump offsets are 2 bytes so the maximum amount of instructions we can jump over is 65536
static int emitJump(pd_code_ctx* ctx, uint8_t instruction) {
  emitByte(ctx, instruction);
  emitByte(ctx, 0xff);
  emitByte(ctx, 0xff);
  return currentChunk(ctx)->count - 2;
}

// Patches the emitted jump instruction, see emitJump()
static void patchJump(pd_code_ctx* ctx, int offset) {
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentChunk(ctx)->count - offset - 2;
  
  if(jump > UINT16_MAX) {
    error(ctx, "Too much code to jump over.");
  }

  currentChunk(ctx)->code[offset] = jump & 0xff;
  currentChunk(ctx)->code[offset + 1] = (jump >> 8) & 0xff;
}

static void emitLoop(pd_code_ctx* ctx, int loopStart) {
  emitByte(ctx, PVM_OP_LOOP);
  
  int offset = currentChunk(ctx)->count - loopStart + 2;
  if(offset > UINT16_MAX) error(ctx, "Loop body too large.");
  
  emitByte(ctx, offset & 0xff);
  emitByte(ctx, (offset >> 8) & 0xff);
}

static int addUpvalue(pd_code_ctx* ctx, uint8_t index, bool isLocal) {
  int upvalueCount = ctx->function->upvalue_count;
  for(int i = 0; i < upvalueCount; i++) {
    pd_compiler_upvalue* upvalue = &ctx->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if(upvalueCount == 256) {
    error(ctx, "Too many closure variables in function.");
    return 0;
  }

  ctx->upvalues[upvalueCount].isLocal = isLocal;
  ctx->upvalues[upvalueCount].index = index;
  return ctx->function->upvalue_count++;
}

static int resolveUpvalue(pd_code_ctx* ctx, char* name, size_t len) {
  if(ctx->enclosing == NULL) return -1;
  
  int local = resolveLocal(ctx->enclosing, name, len);
  if(local != -1) {
    ctx->enclosing->locals[local].isCaptured = true;
    return addUpvalue(ctx, (uint8_t)local, true);
  }
  int upvalue = resolveUpvalue(ctx->enclosing, name, len);
  if(upvalue != -1) {
    return addUpvalue(ctx, (uint8_t)upvalue, false);
  }
  return -1;                                                
}

// Compiles a number node.
void pd_compile_number(pd_code_ctx* ctx, pd_ast_node* node) {
  // Optimizing small numbers to be a single instruction.
  // This is also what JVM does.
  // -1 is handled in unary compilation.
  double num = node->number.value;
  // TODO: Is there a cleaner way to do this check?
  // Before i used a switch case but after introducing float numbers
  // switch case does not allow doubles.
  if(num == 0) {
    emitByte(ctx, PVM_OP_PUSH_ZERO);
  } else if(num == 1) {
    emitByte(ctx, PVM_OP_PUSH_ONE);
  } else if(num == 2) {
    emitByte(ctx, PVM_OP_PUSH_TWO);
  } else if(num == 3) {
    emitByte(ctx, PVM_OP_PUSH_THREE);
  } else if(num == 4) {
    emitByte(ctx, PVM_OP_PUSH_FOUR);
  } else if(num == 5) {
    emitByte(ctx, PVM_OP_PUSH_FIVE);
  } else {
    emitConstant(ctx, NUMBER_VAL(num));
  }
}

void pd_compile_string(pd_code_ctx* ctx, pd_ast_node* node) {
  // TODO: store length in the node, find a way to get them on the parser side.
  emitConstant(ctx, PD_FROM(pd_str_new(ctx->vm, node->string.value, strlen(node->string.value))));
}

void pd_compile_boolean(pd_code_ctx* ctx, pd_ast_node* node) {
  if(node->boolean.value)
    emitByte(ctx, PVM_OP_TRUE);
  else
    emitByte(ctx, PVM_OP_FALSE);
}

void pd_compile_logical(pd_code_ctx* ctx, pd_ast_node* node, uint8_t instruction) {
  pd_compile(ctx, node->binop.lhs);
  int jump = emitJump(ctx, instruction);
  pd_compile(ctx, node->binop.rhs);
  patchJump(ctx, jump);
}

void pd_compile_bin_op(pd_code_ctx* ctx, pd_ast_node* node) {
  // Compile operands.
  // Logical operators compile their operands manually.
  if(node->binop.op != PD_BIN_AND && node->binop.op != PD_BIN_OR) {
    pd_compile(ctx, node->binop.lhs);
    pd_compile(ctx, node->binop.rhs);
  }

  /*
   * Operands are compiled and an instruction is sent to load them on the stack then the specific bytecode is emitted to do
   * the actual operation.
   *
   * Given an expression:
   * 5 + 2
   * 
   * Emits:
   * OP_CONSTANT <idx>
   * OP_CONSTANT <idx>
   * OP_ADD
   *
   * Which runs like:
   * stack: []
   * OP_CONSTANT <idx>
   * stack: [5]
   * OP_CONSTANT <idx>
   * stack: [5, 2]
   * OP_ADD
   * which does:
   * POP
   * stack: [5]
   * b = 2
   * POP
   * stack: []
   * a = 5
   * PUSH <a + b>
   * stack: [7]
   *
   * Note that operands is loaded in opposite order since stack is LIFO (Last-In First-Out) so when we pop we get the last operand
   */
  switch(node->binop.op) {
    case PD_BIN_PLUS:
      emitByte(ctx, PVM_OP_ADD);
      break;
    case PD_BIN_MINUS:
      emitByte(ctx, PVM_OP_SUBTRACT);
      break;
    case PD_BIN_DIV:
      emitByte(ctx, PVM_OP_DIVIDE);
      break;
    case PD_BIN_MUL:
      emitByte(ctx, PVM_OP_MULTIPLY);
      break;
    case PD_BIN_GT:
      emitByte(ctx, PVM_OP_GT);
      break;
    case PD_BIN_LT:
      emitByte(ctx, PVM_OP_LT);
      break;
    case PD_BIN_GE:
      emitByte(ctx, PVM_OP_GE);
      break;
    case PD_BIN_LE:
      emitByte(ctx, PVM_OP_LE);
      break;
    case PD_BIN_SHL:
      emitByte(ctx, PVM_OP_SHL);
      break;
    case PD_BIN_SHR:
      emitByte(ctx, PVM_OP_SHR);
      break;
    case PD_BIN_BOR:
      emitByte(ctx, PVM_OP_BOR);
      break;
    case PD_BIN_BAND:
      emitByte(ctx, PVM_OP_BAND);
      break;
    case PD_BIN_EQ:
      emitByte(ctx, PVM_OP_EQ);
      break;
    case PD_BIN_NEQ:
      emitByte(ctx, PVM_OP_NEQ);
      break;
    case PD_BIN_XOR:
      emitByte(ctx, PVM_OP_XOR);
      break;
    case PD_BIN_AND:
      pd_compile_logical(ctx, node, PVM_OP_AND);
      break;
    case PD_BIN_OR:
      pd_compile_logical(ctx, node, PVM_OP_OR);
      break;
    default:
      pd_unreachable();
  }
}

void pd_compile_null(pd_code_ctx* ctx) {
  emitByte(ctx, PVM_OP_NULL);
}

void pd_compile_unary(pd_code_ctx* ctx, pd_ast_node* node) {
  if(node->unary.type == PD_UNARY_MINUS && node->unary.rhs->type == PD_AST_NUMBER && node->unary.rhs->number.value == 1) {
    // If we are compiling a literal -1 optimize it to a single instruction.
    emitByte(ctx, PVM_OP_PUSH_NEG_ONE);
    return;
  }
  pd_compile(ctx, node->unary.rhs);
  switch(node->unary.type) {
    case PD_UNARY_MINUS:
      emitByte(ctx, PVM_OP_NEGATE);
      break;
    case PD_UNARY_NOT:
      emitByte(ctx, PVM_OP_NOT);
      break;
    case PD_UNARY_BNOT:
      emitByte(ctx, PVM_OP_BNOT);
      break;
  }
}

void pd_compile_block(pd_code_ctx* ctx, pd_ast_node* block) {
  for(int x = 0; x < block->block.count; x++) {
    pd_ast_node* node = block->block.statements[x];
    pd_compile(ctx, node);

    // If we compiled an expression statement, pop it after
    // This expressions are expressions that we don't need the results but only the side effects they do, such as
    // function calls, so we pop the results after to save stack slots.
    if(pd_ast_is_expr(node) && !ctx->nopop) emitByte(ctx, PVM_OP_POP);
    ctx->nopop = 0;
  }
}

// TODO store length of strings in the node themselves from the information we get at lex time.
void pd_compile_assign(pd_code_ctx* ctx, pd_ast_node* node) {
  pd_compile(ctx, node->assign.expr);
  if(ctx->scopeDepth > 0) {
    // handle locals.
    if(declareLocal(ctx, node->assign.name, strlen(node->assign.name))) {
      markInitialized(ctx);
      ctx->nopop = 1;
      return;
    }
    int arg = resolveLocal(ctx, node->assign.name, strlen(node->assign.name));
    pd_assert(arg != -1, "Should never be -1");
    emitBytes(ctx, PVM_OP_SET_LOCAL, (uint8_t)arg);
    return;
  }
  // handle globals
  emitBytes(ctx, PVM_OP_SET_GLOBAL, identifierConstant(ctx, node->assign.name, strlen(node->assign.name)));
}

void pd_compile_variable(pd_code_ctx* ctx, pd_ast_node* node) {
  if(ctx->scopeDepth > 0) {
    // handle locals.
    int arg = resolveLocal(ctx, node->variable.name, strlen(node->variable.name));
    if(arg != -1) {
      emitBytes(ctx, PVM_OP_GET_LOCAL, (uint8_t)arg);
      return;
    } else if((arg = resolveUpvalue(ctx, node->variable.name, strlen(node->variable.name))) != -1) {
      emitBytes(ctx, PVM_OP_GET_UPVALUE, (uint8_t)arg);
      return;
    }
  }
  // handle globals.
  emitBytes(ctx, PVM_OP_GET_GLOBAL, identifierConstant(ctx, node->variable.name, strlen(node->variable.name)));
}

void pd_compile_conditional(pd_code_ctx* ctx, pd_ast_node* node) {
  // compile the condition
  pd_compile(ctx, node->conditional.condition);
  int then = emitJump(ctx, PVM_OP_JUMP_IF_FALSE);
  pd_compile(ctx, node->conditional.trueNode);
  int elseJump;
  if(node->conditional.falseNode != NULL) elseJump = emitJump(ctx, PVM_OP_JUMP);
  patchJump(ctx, then);

  if(node->conditional.falseNode != NULL) { // else clause is optional.
    pd_compile(ctx, node->conditional.falseNode);
    patchJump(ctx, elseJump);
  }
}

void pd_compile_while(pd_code_ctx* ctx, pd_ast_node* node) {
  int loopStart = currentChunk(ctx)->count;
  pd_compile(ctx, node->while_loop.condition);
  
  int exitJump = emitJump(ctx, PVM_OP_JUMP_IF_FALSE);
  if(node->while_loop.body != NULL) pd_compile(ctx, node->while_loop.body);
  emitLoop(ctx, loopStart);
  patchJump(ctx, exitJump);
}

void pd_compile_function(pd_code_ctx* ctx, pd_ast_node* node) {
  uint8_t global = 0;
  // this is awful
  char* name = node->function.prototype->prototype.name;
  size_t len = strlen(name);
  if(ctx->scopeDepth > 0)
    declareLocal(ctx, name, len);
  else
    global = identifierConstant(ctx, name, len);
  markInitialized(ctx);

  pd_code_ctx fnctx;
  pd_compile_ctx_init(&fnctx, ctx->vm, PD_TYPE_FUNCTION);
  fnctx.enclosing = ctx;
  fnctx.function->name = PD_AS_STRING(pd_str_new(ctx->vm, name, len));
  // Init ctx is built with top-level block in mind so scope is actually -1, begin two scopes to fix it.
  // TODO: We have access to fn type in init avoid the hack if we are making a function ctx.
  beginScope(&fnctx);
  // beginScope(&fnctx);
  // Compile the parameter list.                                
  /* consume(TOKEN_LEFT_PAREN, "Expect '(' after function name."); */
  if(node->function.prototype->prototype.argc > 255)
    error(&fnctx, "Cannot have more than 255 parameters.");
  for(int x = 0; x < node->function.prototype->prototype.argc; x++) {
    char* name = node->function.prototype->prototype.args[x];
    fnctx.function->arity++;
    if(declareLocal(&fnctx, name, strlen(name)))
      markInitialized(&fnctx);
  }
  //fnctx.scopeDepth -= 1; // hack, cuz compiling body starts a new scope which gets us out of the current one we made here.
  pd_compile(&fnctx, node->function.body);
  
  // Create the function object.
  pd_function* function = pd_compile_ctx_end(&fnctx);
  ctx->errors += fnctx.errors; // We want to keep track of all nested context errors in the top-most one.
  emitBytes(ctx, PVM_OP_CLOSURE, makeConstant(ctx, PD_FROM(function)));
  for(int i = 0; i < function->upvalue_count; i++) {
    emitByte(ctx, fnctx.upvalues[i].isLocal ? 1 : 0);
    emitByte(ctx, fnctx.upvalues[i].index);
  }
  if(ctx->scopeDepth == 0) {
    emitBytes(ctx, PVM_OP_SET_GLOBAL, global);
    // On globals the results shouldn't stay on the stack, we are manually creating the variable so we have to pop ourselves
    // if it was a regular var assignment the compiler already pops temporaries.
    emitByte(ctx, PVM_OP_POP);
  }
}

void pd_compile_call(pd_code_ctx* ctx, pd_ast_node* node) {
  // We are duplicating the code inside compile var, this is bad practice, we will need to do a huge cleanup sometimes
  // but for now we'll say "it works so don't touch it"
  if(ctx->scopeDepth > 0) {
    // handle locals.
    int arg = resolveLocal(ctx, node->call.name, strlen(node->call.name));
    if(arg != -1)
      emitBytes(ctx, PVM_OP_GET_LOCAL, (uint8_t)arg);
    else if((arg = resolveUpvalue(ctx, node->call.name, strlen(node->call.name))) != -1)
      emitBytes(ctx, PVM_OP_GET_UPVALUE, (uint8_t)arg);
    else // try globals
      emitBytes(ctx, PVM_OP_GET_GLOBAL, identifierConstant(ctx, node->call.name, strlen(node->call.name)));
  } else {
    // handle globals.
    emitBytes(ctx, PVM_OP_GET_GLOBAL, identifierConstant(ctx, node->variable.name, strlen(node->variable.name)));
  }
  if(node->call.argc > 255) error(ctx, "Cannot have more than 255 arguments.");
  for(int x = 0; x < node->call.argc; x++)
    pd_compile(ctx, node->call.args[x]);
  emitBytes(ctx, PVM_OP_CALL, node->call.argc);
}

void pd_compile_return(pd_code_ctx* ctx, pd_ast_node* node) {
  if(ctx->type == PD_TYPE_SCRIPT) {
    error(ctx, "Cannot return from top-level code.");
  }

  if(node->ret.expr == NULL) {
    emitReturn(ctx);
  } else {
    pd_compile(ctx, node->ret.expr);
    emitByte(ctx, PVM_OP_RETURN);
  }
}

void pd_compile_class(pd_code_ctx* ctx, pd_ast_node* node) {}

void pd_compile(pd_code_ctx* ctx, pd_ast_node* node) {
  if(node == NULL) return;
  ctx->line = node->line;
  switch(node->type) {
    case PD_AST_CLASS:
      pd_compile_class(ctx, node);
      break;
    case PD_AST_EMPTY:
      break; // nothing to do.
    case PD_AST_WHILE:
      pd_compile_while(ctx, node);
      break;
    case PD_AST_NUMBER:
      pd_compile_number(ctx, node);
      break;
    case PD_AST_CONDITIONAL:
    case PD_AST_TERNARY:
      pd_compile_conditional(ctx, node);
      break;
    case PD_AST_FUNCTION:
      pd_compile_function(ctx, node);
      break;
    case PD_AST_CALL:
      pd_compile_call(ctx, node);
      break;
    case PD_AST_NULL:
      pd_compile_null(ctx);
      break;
    case PD_AST_STRING:
      pd_compile_string(ctx, node);
      break;
    case PD_AST_BIN_OP:
      pd_compile_bin_op(ctx, node);
      break;
    case PD_AST_UNARY:
      pd_compile_unary(ctx, node);
      break;
    case PD_AST_BLOCK:
      //beginScope(ctx);
      pd_compile_block(ctx, node);
      //endScope(ctx);
      break;
    case PD_AST_RETURN:
      pd_compile_return(ctx, node);
      break;
    case PD_AST_BOOLEAN:
      pd_compile_boolean(ctx, node);
      break;
    case PD_AST_ASSIGN:
      pd_compile_assign(ctx, node);
      break;
    case PD_AST_VARIABLE:
      pd_compile_variable(ctx, node);
      break;
    default:
      pd_unreachable();
  }
}

// Initializes basic compilation context.
void pd_compile_ctx_init(pd_code_ctx* ctx, pvm_t* vm, pd_function_type type) {
  ctx->enclosing = NULL;
  ctx->vm = vm;
  ctx->vm->compiler = ctx;
  ctx->function = NULL;
  ctx->type = type;
  ctx->line = 0;
  ctx->localCount = 0;
  // Global is 0 but the top level file is also a "block" so it would increment it to 1 when we are on global scope!
  // To fix this we make it -1 and when the top level block is compiling it would make it 0
  ctx->scopeDepth = 0 /*-1*/;
  ctx->errors = 0;
  ctx->nopop = 0;
  ctx->function = pd_function_new(vm);

  pd_compiler_local* local = &ctx->locals[ctx->localCount++];
  local->depth = 0;
  local->name = "";
  local->len = 0;
  local->isCaptured = false;
}

// End compilation.
// Returns the compiled top-level function that the context compiled.
pd_function* pd_compile_ctx_end(pd_code_ctx* ctx) {
  ctx->line = 0;
  emitReturn(ctx); // Add the implicit return, this will be written using line 0 as it's an internally inserted code.
  pd_function* fn = ctx->function;
  // Now is also a good time to disassemble the function.
  //pvm_disassemble_chunk(currentChunk(ctx), fn->name != NULL ? fn->name->bytes : "<script>");
  if(ctx->enclosing != NULL) ctx->vm->compiler = ctx->enclosing;
  return fn;
}
