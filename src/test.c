#include "dyn_str.h"
#include <stdio.h>

int main() {
  pd_dyn_str* str = pd_dyn_str_new();
  pd_dyn_str_append(str, "Hello, World!");
  pd_dyn_str_append(str, " dynamic!");
  printf("%s\n", PD_DYN_CSTR(str));
  return 0;
}
