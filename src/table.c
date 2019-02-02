#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

static void adjust_capacity(Table *table, int capacity);
static Entry* find_entry(Entry *entries, int capacity, ObjString *key);

void init_table(Table *table) {
   table->count = 0;
   table->capacity = 0;
   table->entries = NULL;
}

void free_table(Table *table) {
   FREE_ARRAY(table->entries, Entry, table->capacity);
   init_table(table);
}

bool table_get(Table *table, ObjString *key, Value *value) {
   if (table->entries == NULL) return false;

   Entry *entry = find_entry(table->entries, table->capacity, key);
   if (entry->key == NULL) return false;

   *value = entry->value;
   return true;
}

bool table_set(Table *table, ObjString *key, Value value) {
   if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
      int capacity = GROW_CAPACITY(table->capacity);
      adjust_capacity(table, capacity);
   }

   Entry *entry = find_entry(table->entries, table->capacity, key);

   bool is_new_key = entry->key == NULL;
   // we don't increment the count if the entry is a tombstone
   // and we don't decrement the count when deleting an element
   // that is so if we end up with tombstones in all table entries
   // we run the risk of getting in an infinte loop in find_entry^
   if (is_new_key && IS_NIL(entry->value)) ++table->count;

   entry->key = key;
   entry->value = value;
   return is_new_key;
}

bool table_delete(Table *table, ObjString *key) {
   if (table->count == 0) return false;

   Entry *entry = find_entry(table->entries, table->capacity, key);
   if (entry->key == NULL) return false;

   entry->key = NULL;
   entry->value = BOOL_VAL(true);
   return true;
}

static void adjust_capacity(Table *table, int capacity) {
   Entry *entries = ALLOCATE(Entry, capacity);
   for (int i = 0; i < capacity; ++i) {
      entries[i].key = NULL;
      entries[i].value = NIL_VAL;
   }
   
   table->count = 0;
   for (int i = 0; i < table->capacity; ++i) {
      Entry *entry = &table->entries[i];
      // we skip tombstones too!
      if (entry->key == NULL) continue;
      
      Entry *dest = find_entry(entries, capacity, entry->key);
      dest->key = entry->key;
      dest->value = entry->value;
      ++table->count;
   }

   FREE_ARRAY(table->entries, Entry, table->capacity);
   table->entries = entries;
   table->capacity = capacity;
}

static Entry* find_entry(Entry *entries, int capacity, ObjString *key) {
   uint32_t index = key->hash % capacity;
   Entry *tombstone = NULL;
   for (;;) {
      Entry *entry = &entries[index];

      if (entry->key == NULL) {
         // we found an empty entry
         // it could be a tombstone or a truly empty entry
         if (IS_NIL(entry->value)) {
            // a truly empty entry so stop searching
            return tombstone != NULL ? tombstone : entry;
         } else if (tombstone == NULL) {
            // a tombstone. Continue searching but remember the tombstone
            tombstone = entry;
         }
      // we can't compare hashes because of hash collisions
      // so we make use of string interning that we support
      // in short, if the two keys have the same values, they are 
      // guarenteed to point to the same location in memory
      } else if (entry->key == key) {
         // entry found.
         return entry;
      }

      index = (index + 1) % capacity;
   }
}

ObjString* table_find_string(Table *table, const char *chars, int length, uint32_t hash) {
   if (table->entries == NULL) return NULL;

   uint32_t index = hash % table->capacity;
   for (;;) {
      Entry *entry = &table->entries[index];

      if (entry->key == NULL) {
         // empty entry. Stop searching
         if (IS_NIL(entry->value)) return NULL;
         // else its a tombstone. Just ignore
         else {}
      } else if (entry->key->length == length
               && entry->key->hash == hash
               && memcmp(entry->key->chars, chars, length) == 0) {
         // found it
         return entry->key;
      }

      index = (index + 1) % table->capacity;
   }
}