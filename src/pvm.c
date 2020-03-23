#include "pvm.h"
#include "chunk.h"
#include <stdlib.h>
#include "opcodes.h"
#include "runtime.h"
#include "gc.h"
// Experimental libuv attempts.
#include <uv.h>

// Pushes a value on top of the stack.
void pvm_push(pvm_t* vm, pd_value value) {
  *vm->stack_top = value;
  vm->stack_top++;
}

// Pops a value from top of the stack.
pd_value pvm_pop(pvm_t* vm) {
  vm->stack_top--;
  return *vm->stack_top;
}

static void resetStack(pvm_t* vm) {
  vm->stack_top = vm->stack;
  vm->frame_count = 0;
  vm->open_upvalues = NULL;
}

// Peek into the stack without popping it.
static inline pd_value peek(pvm_t* vm, int distance) {   
  return vm->stack_top[-1 - distance];
}

pvm_t* pvm_new() {
  pvm_t* vm = malloc(sizeof(pvm_t));
  vm->objects = NULL;
  vm->gray_capacity = 0;
  vm->gray_count = 0;
  vm->bytes_allocated = 0;
  vm->next_gc = 1024 * 1024;
  vm->gray_stack = NULL;
  vm->compiler = NULL;
  vm->mark_bit = true;
  vm->loop = uv_default_loop();
  resetStack(vm);
  pd_table_init(&vm->strings);
  return vm;
}

void pvm_free(pvm_t* vm) {
  pd_table_free(vm, &vm->strings);
  pd_gc_free_objects(vm);
  free(vm->gray_stack);
  free(vm);
  // uv_loop_close(vm->loop);
}

// TODO this will need a huge change, for now its like this to follow the tutorial.
// We will need to implement exception handling for production.
// I'll wait until classes so i can get an idea of an Exception class.
static void runtimeError(pvm_t* vm, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for(int i = vm->frame_count - 1; i >= 0; i--) {
    pvm_frame* frame = &vm->frames[i];                            
    pd_function* function = frame->closure->function;
    // -1 because the IP is sitting on the next instruction to be executed.
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "  [line %d] in ", function->chunk.lines[instruction]);
    if(function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->bytes);          
    }
  }
  
  resetStack(vm);
}

void pvm_define_function(pvm_t* vm, char* name, pd_native function) {
  // GC guards
  pvm_push(vm, PD_FROM(pd_str_new(vm, name, (int)strlen(name))));
  pvm_push(vm, PD_FROM(pd_native_function_new(vm, function)));

  pd_value index;
  pd_str* identifier = PD_AS_STRING(vm->stack[0]);
  if(pd_table_get(&vm->globals, identifier, &index)) {
    vm->global_values.data[(uint8_t)AS_DOUBLE(index)] = vm->stack[1];
  }
  
  pvm_pop(vm);
  pvm_pop(vm);
}

static bool call(pvm_t* vm, pd_closure* closure, int argCount) {
  if(argCount != closure->function->arity) {
    runtimeError(vm, "Expected %d arguments but got %d.", closure->function->arity, argCount);
    return false;
  }

  if(vm->frame_count == PVM_FRAMES_MAX) {
    runtimeError(vm, "Stack overflow.");
    return false;
  }

  pvm_frame* frame = &vm->frames[vm->frame_count++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  
  frame->slots = vm->stack_top - argCount - 1;
  return true;
}

bool pvm_call(pvm_t* vm, pd_value callee, int argCount) {
  if(IS_OBJECT(callee)) {
    switch (OBJECT_TYPE(callee)) {
      case PD_OBJ_CLOSURE:
        return call(vm, PD_AS_CLOSURE(callee), argCount);
      case PD_OBJ_NATIVE: {
        pd_native native = PD_AS_NATIVE(callee);
        pd_value result = native(vm, argCount, vm->stack_top - argCount);
        vm->stack_top -= argCount + 1;
        pvm_push(vm, result);
        return true;
      }
      default:
        // Non-callable object type.
        break;
    }
  }
  
  runtimeError(vm, "Can only call functions and classes.");
  return false;
}

static pd_upvalue* captureUpvalue(pvm_t* vm, pd_value* local) {
  pd_upvalue* prevUpvalue = NULL;
  pd_upvalue* upvalue = vm->open_upvalues;
  
  while(upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }
  if(upvalue != NULL && upvalue->location == local) return upvalue;

  pd_upvalue* createdUpvalue = pd_upvalue_new(vm, local);
  createdUpvalue->next = upvalue;
  if(prevUpvalue == NULL) {
    vm->open_upvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }
  return createdUpvalue;
}

static void closeUpvalues(pvm_t* vm, pd_value* last) {
  while(vm->open_upvalues != NULL && vm->open_upvalues->location >= last) {
    pd_upvalue* upvalue = vm->open_upvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm->open_upvalues = upvalue->next;
  }
}

// This is it, the core of the VM.
// The interpreter loop, it executes all instructions
// which means that this part is highly performance critical so we want to squeeze every bit of performance we can here.
void pvm_run(pvm_t* vm) {
  //vm->chunk = chunk;
  //vm->ip = vm->chunk->code;
  pvm_frame* frame = &vm->frames[vm->frame_count - 1];

  register uint8_t* ip = frame->ip;

#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2]) | ip[-1] << 8))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.data[READ_BYTE()])
#define READ_CONSTANT_LONG() (frame->closure->function->chunk.constants.data[READ_SHORT()])

#define BINARY_OP(cast, op) \
  do { \
    if (!IS_DOUBLE(peek(vm, 0)) || !IS_DOUBLE(peek(vm, 1))) { \
      frame->ip = ip; \
      runtimeError(vm, "Operands must be numbers."); \
      return; \
    } \
    double b = AS_DOUBLE(pvm_pop(vm)); \
    double a = AS_DOUBLE(pvm_pop(vm)); \
    pvm_push(vm, cast(a op b)); \
  } while (false)

// Bitwise operations should be done on pure integers, so cast to int first.
#define BITWISE_OP(op) \
  do { \
    int b = (int) AS_DOUBLE(pvm_pop(vm)); \
    int a = (int) AS_DOUBLE(pvm_pop(vm)); \
    pvm_push(vm, DOUBLE_VAL((double)(a op b))); \
  } while(0)

// To allow string operands for == and !=
#define CMP(op) \
  do { \
    pd_value b = pvm_pop(vm); \
    pd_value a = pvm_pop(vm); \
    pvm_push(vm, BOOL_VAL(a == b)); \
  } while(0)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for(pd_value* slot = vm->stack; slot < vm->stack_top; slot++) {
      printf("\x1b[32m[\x1b[0m ");
      pd_value_print(*slot);
      printf(" \x1b[32m]\x1b[0m");
    }
    printf("\n");
    pvm_disassemble_instruction(&frame->closure->function->chunk, (int)(ip - frame->closure->function->chunk.code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case PVM_OP_POP:
        pvm_pop(vm);
        break;
      case PVM_OP_POPN:
        vm->stack_top -= READ_BYTE();
        break;
      case PVM_OP_CONSTANT:
        pvm_push(vm, READ_CONSTANT());
        break;
      case PVM_OP_CONSTANT_LONG:
        pvm_push(vm, READ_CONSTANT_LONG());
        break;
      case PVM_OP_NULL:
        pvm_push(vm, NULL_VALUE);
        break;
      case PVM_OP_TRUE:
        pvm_push(vm, TRUE_VALUE);
        break;
      case PVM_OP_FALSE:
        pvm_push(vm, FALSE_VALUE);
        break;
      case PVM_OP_PUSH_NEG_ONE:
        pvm_push(vm, DOUBLE_VAL(-1));
        break;
      case PVM_OP_PUSH_ZERO:
        pvm_push(vm, DOUBLE_VAL(0));
        break;
      case PVM_OP_PUSH_ONE:
        pvm_push(vm, DOUBLE_VAL(1));
        break;
      case PVM_OP_PUSH_TWO:
        pvm_push(vm, DOUBLE_VAL(2));
        break;
      case PVM_OP_PUSH_THREE:
        pvm_push(vm, DOUBLE_VAL(3));
        break;
      case PVM_OP_PUSH_FOUR:
        pvm_push(vm, DOUBLE_VAL(4));
        break;
      case PVM_OP_PUSH_FIVE:
        pvm_push(vm, DOUBLE_VAL(5));
        break;
      case PVM_OP_NEGATE:
        if(!IS_DOUBLE(peek(vm, 0))) {
          frame->ip = ip;
          runtimeError(vm, "Operand must be a number.");
          return;
        }
        
        pvm_push(vm, DOUBLE_VAL(-AS_DOUBLE(pvm_pop(vm))));        
        break;
      case PVM_OP_NOT:
        // AS_BOOL also casts to a boolean with truthy/falsy checks so no type checks are needed.
        pvm_push(vm, BOOL_VAL(!AS_BOOL(pvm_pop(vm))));
        break;
      case PVM_OP_ADD:
        BINARY_OP(DOUBLE_VAL, +);
        break;
      case PVM_OP_SUBTRACT:
        BINARY_OP(DOUBLE_VAL, -);
        break;
      case PVM_OP_MULTIPLY:
        BINARY_OP(DOUBLE_VAL, *);
        break;
      case PVM_OP_DIVIDE:
        BINARY_OP(DOUBLE_VAL, /);
        break;
      case PVM_OP_GT:
        BINARY_OP(BOOL_VAL, >);
        break;
      case PVM_OP_LT:
        BINARY_OP(BOOL_VAL, <);
        break;
      case PVM_OP_GE:
        BINARY_OP(BOOL_VAL, >=);
        break;
      case PVM_OP_LE:
        BINARY_OP(BOOL_VAL, <=);
        break;
      case PVM_OP_EQ:
        CMP(==);
        break;
      case PVM_OP_NEQ:
        CMP(!=);
        break;
      case PVM_OP_SHL:
        BITWISE_OP(<<);
        break;
      case PVM_OP_SHR:
        BITWISE_OP(>>);
        break;
      case PVM_OP_BAND:
        BITWISE_OP(&);
        break;
      case PVM_OP_BOR:
        BITWISE_OP(|);
        break;
      case PVM_OP_XOR:
        BITWISE_OP(^);
        break;
      case PVM_OP_JUMP: {
        uint16_t offset = READ_SHORT();
        ip += offset;
        break;
      }
      case PVM_OP_LOOP: {
        uint16_t offset = READ_SHORT();
        ip -= offset;
        break;
      }
      case PVM_OP_RETURN_NULL: {
        closeUpvalues(vm, frame->slots);
        vm->frame_count--;
        if(vm->frame_count == 0) {
          pvm_pop(vm);
          return;
        }
        vm->stack_top = frame->slots;
        pvm_push(vm, NULL_VALUE);
        frame = &vm->frames[vm->frame_count - 1];
        ip = frame->ip;
        break;
      }
      case PVM_OP_RETURN: {
        pd_value result = pvm_pop(vm);
        closeUpvalues(vm, frame->slots);
        vm->frame_count--;
        if(vm->frame_count == 0) {
          pvm_pop(vm);
          return;
        }
        
        vm->stack_top = frame->slots;
        pvm_push(vm, result);
        
        frame = &vm->frames[vm->frame_count - 1];
        ip = frame->ip;
        break;
      }
      case PVM_OP_CALL: {
        int argCount = READ_BYTE();
        frame->ip = ip;
        if(!pvm_call(vm, peek(vm, argCount), argCount))
          return;
        frame = &vm->frames[vm->frame_count - 1];
        ip = frame->ip;
        break;
      }
      case PVM_OP_AND: {
        uint16_t offset = READ_SHORT();
        if(!AS_BOOL(peek(vm, 0))) ip += offset;
        else pvm_pop(vm);
        break;
      }
      case PVM_OP_OR: {
        uint16_t offset = READ_SHORT();
        if(!AS_BOOL(peek(vm, 0))) pvm_pop(vm);
        else ip += offset;
        break;
      }
      case PVM_OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if(!AS_BOOL(pvm_pop(vm))) ip += offset;
        break;
      }
      // TODO bitwise ~, it's unary so we can't use BITWISE_OP macro.
      case PVM_OP_GET_LOCAL:
        pvm_push(vm, frame->slots[READ_BYTE()]);
        break;
      case PVM_OP_SET_LOCAL:
        frame->slots[READ_BYTE()] = peek(vm, 0);
        break;
      case PVM_OP_SET_GLOBAL:
        vm->global_values.data[READ_BYTE()] = peek(vm, 0);
        break;
      case PVM_OP_CLOSE_UPVALUE:
        closeUpvalues(vm, vm->stack_top - 1);
        pvm_pop(vm);
        break;
      case PVM_OP_GET_GLOBAL: {
        pd_value value = vm->global_values.data[READ_BYTE()];
        if(IS_UNDEFINED(value)) {
          frame->ip = ip;
          runtimeError(vm, "Undefined variable.");
          return;
        }
        pvm_push(vm, value);
        break;
      }
      case PVM_OP_CLOSURE: {
        pd_function* function = PD_AS_FUNCTION(READ_CONSTANT());
        pd_closure* closure = pd_closure_new(vm, function);
        pvm_push(vm, PD_FROM(closure));
        for(int i = 0; i < closure->upvalue_count; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if(isLocal) {
            closure->upvalues[i] = captureUpvalue(vm, frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;                                               
      }
      case PVM_OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        pvm_push(vm, *frame->closure->upvalues[slot]->location);
        break;                                          
      }
      case PVM_OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(vm, 0);
        break;                                              
      }
      default:
        pd_unreachable();
    }
  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef BINARY_OP
#undef BITWISE_OP
#undef CMP
}

// Executes the top-level function.
void pvm_exec(pvm_t* vm, pd_function* fn) {
  vm->compiler = NULL; // We don't need it at runtime, it's only a compile-time GC guard.
  pvm_init_builtins(vm);
  pvm_push(vm, PD_FROM(fn));
  pd_closure* closure = pd_closure_new(vm, fn);
  pvm_pop(vm);
  pvm_push(vm, PD_FROM(closure));
  pvm_call(vm, PD_FROM(closure), 0);
  pvm_run(vm);
  //uv_run(vm->loop, UV_RUN_DEFAULT);
}
