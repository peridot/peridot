#include "pdjit.h"
#include <stdlib.h>

#if _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

// Links the DynASM buffer and returns an executable memory.
// Care should be taken to free the memory when not in-use to avoid memory leaks.
void* pdjit_link(pdjit_state* jit) {

  size_t size;
  void* buf;

  dasm_link(&jit->state, &size);

  // Allocate readable & writable memory.
#ifdef _WIN32
  buf = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  buf = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

  dasm_encode(&jit->state, buf);

  // Make the memory readable & executable but no longer writable (for security).
#ifdef _WIN32
  {DWORD dwOld; VirtualProtect(buf, size, PAGE_EXECUTE_READ, &dwOld); }
#else
  mprotect(buf, size, PROT_READ | PROT_EXEC);
#endif

  return buf;
}
