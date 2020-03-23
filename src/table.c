#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "table.h"
#include "value.h"
#include "gc.h"

#define TABLE_MAX_LOAD 0.75

void pd_table_init(pd_table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void pd_table_free(pvm_t* vm, pd_table* table) {
  PD_FREE_ARRAY(vm, pd_table_entry, table->entries, table->capacity);
  pd_table_init(table);
}

static pd_table_entry* findEntry(pd_table_entry* entries, int capacity, pd_str* key) {
  uint32_t index = key->hash % capacity;
  pd_table_entry* tombstone = NULL;

  for(;;) {
    pd_table_entry* entry = &entries[index];
    
    if(entry->key == NULL) {
      if(IS_NULL(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone.
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
      // We found the key.
      return entry;
    }

    index = (index + 1) % capacity;                  
  }
}

static void adjustCapacity(pvm_t* vm, pd_table* table, int capacity) {
  pd_table_entry* entries = pd_gc_malloc(vm, sizeof(pd_table_entry) * capacity);
  for(int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL_VALUE;
  }

  table->count = 0;
  for(int i = 0; i < table->capacity; i++) {
    pd_table_entry* entry = &table->entries[i];
    if(entry->key == NULL) continue;
    pd_table_entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  PD_FREE_ARRAY(vm, pd_table_entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool pd_table_set(pvm_t* vm, pd_table* table, pd_str* key, pd_value value) {
  if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = PD_GROW_CAPACITY(table->capacity);
    adjustCapacity(vm, table, capacity);
  }

  pd_table_entry* entry = findEntry(table->entries, table->capacity, key);
  
  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NULL(entry->value)) table->count++;
  
  entry->key = key;
  entry->value = value;
  return isNewKey;
}

void pd_table_add_all(pvm_t* vm, pd_table* from, pd_table* to) {
  for(int i = 0; i < from->capacity; i++) {
    pd_table_entry* entry = &from->entries[i];
    if(entry->key != NULL) {
      pd_table_set(vm, to, entry->key, entry->value);
    }
  }
}

bool pd_table_get(pd_table* table, pd_str* key, pd_value* value) {
  if(table->count == 0) return false;
  
  pd_table_entry* entry = findEntry(table->entries, table->capacity, key);
  if(entry->key == NULL) return false;
  
  *value = entry->value;
  return true;                                                   
}

bool pd_table_delete(pd_table* table, pd_str* key) {
  if(table->count == 0) return false;
  
  // Find the entry.
  pd_table_entry* entry = findEntry(table->entries, table->capacity, key);
  if(entry->key == NULL) return false;
  
  // Place a tombstone in the entry.
  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  
  return true;                                                   
}


pd_str* pd_table_find_string(pd_table* table, const char* chars, size_t length, uint32_t hash) {
  if(table->count == 0) return NULL;
  
  uint32_t index = hash % table->capacity;
  
  for (;;) {
    pd_table_entry* entry = &table->entries[index];

    if(entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if(IS_NULL(entry->value)) return NULL;
    } else if (entry->key->len == length && entry->key->hash == hash && memcmp(entry->key->bytes, chars, length) == 0) {
      // We found it.
      return entry->key;
    }
    index = (index + 1) % table->capacity;
  }
}
