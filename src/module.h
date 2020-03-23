#ifndef _PERIDOT_MODULE_H
#define _PERIDOT_MODULE_H

#include "object.h"
#include "str.h"
#include "table.h"

typedef struct {
  pd_str* name;
  // Map<String, Number> the values map to an index to the global_values field.
  pd_table* globals;
  pd_value_array* global_values;
  // Identify if this is a builtin module.
  bool builtin;
} pd_module;

#endif
