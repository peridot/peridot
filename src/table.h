#ifndef _PERIDOT_TABLE_H
#define _PERIDOT_TABLE_H

// A hash table

#include "value.h"
#include "str.h"
#include "dyn_arr.h"

typedef struct {
  pd_str* key;
  pd_value value;
} pd_table_entry;

typedef struct {
  PERIDOT_DYN_ARR_FIELDS
  pd_table_entry* entries;
} pd_table;

void pd_table_init(pd_table* table);
void pd_table_free(pvm_t* vm, pd_table* table);
bool pd_table_set(pvm_t* vm, pd_table* table, pd_str* key, pd_value value);
void pd_table_add_all(pvm_t* vm, pd_table* from, pd_table* to);
bool pd_table_get(pd_table* table, pd_str* key, pd_value* value);
bool pd_table_delete(pd_table* table, pd_str* key);
// Finds an interned string.
pd_str* pd_table_find_string(pd_table* table, const char* chars, size_t length, uint32_t hash);

#endif // _PERIDOT_TABLE_H
