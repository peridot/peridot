#ifndef _PERIDOT_JIT_H
#define _PERIDOT_JIT_H

#include "../../../dasm/dynasm/dasm_proto.h"

#define Dst &jit->state

typedef struct {
  dasm_State* state;
} pdjit_state;

void* pdjit_link(pdjit_state* jit);
void pdjit_free(pdjit_state* jit);
pdjit_state* pdjit_new(void);

#endif // _PERIDOT_JIT_H
