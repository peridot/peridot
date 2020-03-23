#include "pvm.h"
#include "value.h"
#include "gc.h"
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static pd_value println(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  if(argc > 0) pd_value_print(args[0]);
  printf("\n"); // pd_value_print doesn't print a newline but println builtin should.
  return NULL_VALUE;
}

// Print doesn't put a new line.
static pd_value print(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  if(argc > 0) pd_value_print(args[0]);
  return NULL_VALUE;
}

static pd_value pd_clock(pvm_t* vm, int argc, pd_value* args) {
  (void)vm;
  (void)argc;
  (void)args;
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

// Get gc heap size in bytes.
static pd_value gc_heap_size(pvm_t* vm, int argc, pd_value* args) {
  (void)argc;
  (void)args;
  return NUMBER_VAL((double)vm->bytes_allocated);
}

// Forces a GC cycle.
static pd_value gc_collect(pvm_t* vm, int argc, pd_value* args) {
  (void)argc;
  (void)args;
  pd_gc_collect(vm);
  return NULL_VALUE;
}

// Destroys the VM and exits the process.
static pd_value pd_exit(pvm_t* vm, int argc, pd_value* args) {
  int status = 0;
  // Right now we don't have a way to report errors so if the argument isn't a number
  // we simply act like it wasn't even passed, but this is a TODO for when we have exception handling.
  if(argc > 0) {
    if(IS_DOUBLE(args[0]))
      status = (int)AS_DOUBLE(args[0]);
  }
  // Destroy the VM and deallocate all objects.
  pvm_free(vm);
  exit(status);
  return NULL_VALUE; // exit kills the process anyway.
}

typedef struct {
  pvm_t* vm;
  pd_closure* cb;
} pd_timer_t;

void cb(uv_timer_t* handle) {
  pd_timer_t* data = handle->data;
  //pvm_exec(data->vm, data->cb);
  pvm_push(data->vm, PD_FROM(data->cb));
  pvm_call(data->vm, PD_FROM(data->cb), 0);
  pvm_run(data->vm);
  uv_close((uv_handle_t*)handle, NULL);
  free(handle);
  free(data);
}

pd_value setTimeout(pvm_t* vm, int argc, pd_value* args) {
  if(argc < 2 || !IS_DOUBLE(args[0]) || !PD_IS_CLOSURE(args[1])) return NULL_VALUE;
  int ms = (int)AS_DOUBLE(args[0]);
  pd_closure* fn = PD_AS_CLOSURE(args[1]);
  uv_timer_t* timer = malloc(sizeof(uv_timer_t));
  uv_timer_init(vm->loop, timer);
  //pd_timer_t data = {vm, fn};
  pd_timer_t* data = malloc(sizeof(pd_timer_t));
  data->vm = vm;
  data->cb = fn;
  timer->data = data;
  uv_timer_start(timer, cb, ms, 0);
  return NULL_VALUE;
}

// Registers all builtins.
void pvm_init_builtins(pvm_t* vm) {
  pvm_define_function(vm, "println", println);
  pvm_define_function(vm, "print", print);
  pvm_define_function(vm, "clock", pd_clock);
  pvm_define_function(vm, "gc_heap_size", gc_heap_size);
  pvm_define_function(vm, "gc_collect", gc_collect);
  pvm_define_function(vm, "exit", pd_exit);
  pvm_define_function(vm, "setTimeout", setTimeout);
}
