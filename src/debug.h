#ifndef _PERIDOT_DEBUG_H
#define _PERIDOT_DEBUG_H

// Dependency cycles...
typedef struct pvm_chunk pvm_chunk;

// Assertions and debug stuff, only enabled on debug builds since they add a little bit of overhead.
#ifdef PD_DEBUG

// Make sure stdio/stdlib is included.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// This is cleaner than using <assert.h> and using && to signal a failed message.
#define pd_assert(cond, msg) do { \
  if(!(cond)) { \
    fprintf(stderr, "[%s:%d] Assertion failed in %s(): %s\n", __FILE__, __LINE__, __func__, msg); \
    abort(); \
  } \
} while(0)

// Used to signal that a block of code should never be reached, if we reach it on debug builds we report it and abort
#define pd_unreachable() do { \
  fprintf(stderr, "[%s:%d] Reached the unreachable in %s()\n", __FILE__, __LINE__, __func__); \
  abort(); \
} while(0)

// Simply API to benchmark execution times.
// Usage:
//   PD_TIMER_START;
//   // Do some long-running code you want to benchmark
//   PD_TIMER_STOP;
//   // At this point the variable elapsed will be available to use.
//   // But to make reporting easier you can also do
//   PD_TIMER_REPORT("Label");
//   It takes a label to prefix the output like "<label> took (time)s"
#define PD_TIMER_START double startTime = (double)clock() / CLOCKS_PER_SEC;
#define PD_TIMER_STOP double elapsed = ((double)clock() / CLOCKS_PER_SEC) - startTime;
#define PD_TIMER_REPORT(str) printf("%s took %.3fs\n", (str), elapsed)
#else
// Release builds.
#define PD_TIMER_START
#define PD_TIMER_STOP
#define PD_TIMER_REPORT(str)

// no-op on release.
#define pd_assert(cond, msg) do {} while(0)

// On Release we signal unreachable code via compiler builtins if available to help compiler do better optimization
// This part is taken from Wren https://github.com/wren-lang/wren/blob/master/src/vm/wren_common.h
#if defined(_MSC_VER)
#define pd_unreachable() __assume(0)
// if gnuc > 4 || gnuc >= 4.5
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#define pd_unreachable() __builtin_unreachable()
#else
// If there isn't a compiler builtin fallback to doing nothing.
#define pd_unreachable()
#endif

#endif // PD_DEBUG

// General debug functions that are included in release builds too.
void pvm_disassemble_chunk(pvm_chunk* chunk, const char* name);
int pvm_disassemble_instruction(pvm_chunk* chunk, int i);
#endif // _PERIDOT_DEBUG_H
