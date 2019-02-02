#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "table.h"

static ObjString* allocate_string(VM *vm, char *chars, int length, uint32_t hash);
static uint32_t hash_string(const char *key, int length);

#define ALLOCATE_OBJ(vm, type, object_type) \
   (type*)allocate_object(vm, sizeof(type), object_type)

static Obj* allocate_object(VM *vm, size_t size, ObjType type) {
   Obj *object = (Obj*)reallocate(NULL, 0, size);
   object->type = type;

   object->next = vm->objects;
   vm->objects = object;
   return  object;
}

ObjString* take_string(VM *vm, char *chars, int length) {
   uint32_t hash = hash_string(chars, length);
   ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
   if (interned != NULL) {
      // we don't need a new string, it already exists in our strings collection
      // and since we own the string we can safely free it
      FREE_ARRAY(chars, char, length + 1);
      return interned;
   }

   return allocate_string(vm, chars, length, hash);
}

ObjString* copy_string(VM *vm, const char *chars, int length) {
   uint32_t hash = hash_string(chars, length);
   ObjString *interned = table_find_string(&vm->strings, chars, length, hash);
   if (interned != NULL) return interned;

   char *heap_chars = ALLOCATE(char, length + 1);
   memcpy(heap_chars, chars, length);
   heap_chars[length] = '\0'; // for c functions support
   
   return allocate_string(vm, heap_chars, length, hash);
}

static ObjString* allocate_string(VM *vm, char *chars, int length, uint32_t hash) {
   ObjString *string = ALLOCATE_OBJ(vm, ObjString, OBJ_STRING);
   string->length = length;
   string->chars = chars;
   string->hash = hash;

   // we only care about the keys, so we set the values to nil
   // more like a hash set than a hash table
   table_set(&vm->strings, string, NIL_VAL);

   return string;
}

static uint32_t hash_string(const char *key, int length) {
   // FNV-1a hashing algorithm
   uint32_t hash = 2166136261u;
   for (int i = 0; i < length; ++i) {
      hash ^= key[i];
      hash *= 16777619;
   }

   return hash;
}

void print_obj(Value value) {
   switch(OBJ_TYPE(value)) {
   case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
   }
}