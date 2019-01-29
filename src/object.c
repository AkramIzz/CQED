#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"

static ObjString* allocate_string(VM *vm, char *chars, int length);

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
   return allocate_string(vm, chars, length);
}

ObjString* copy_string(VM *vm, const char *chars, int length) {
   char *heap_chars = ALLOCATE(char, length + 1);
   memcpy(heap_chars, chars, length);
   heap_chars[length] = '\0'; // for c functions support
   return allocate_string(vm, heap_chars, length);
}

static ObjString* allocate_string(VM *vm, char *chars, int length) {
   ObjString *string = ALLOCATE_OBJ(vm, ObjString, OBJ_STRING);
   string->length = length;
   string->chars = chars;

   return string;
}

void print_obj(Value value) {
   switch(OBJ_TYPE(value)) {
   case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
   }
}