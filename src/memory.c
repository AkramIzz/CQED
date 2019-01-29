#include <stdlib.h>

#include "common.h"
#include "memory.h"

static void free_object(Obj *object);

void* reallocate (void *previous, size_t old_size, size_t new_size) {
   if (new_size == 0) {
      free(previous);
      return NULL;
   }

   return realloc(previous, new_size);
}

void free_objects(Obj *objects) {
   while (objects != NULL) {
      Obj *obj = objects;
      objects = objects->next;
      free_object(obj);
   }
}

static void free_object(Obj *object) {
   switch (object->type) {
      case OBJ_STRING: {
         ObjString *string = (ObjString *)object;
         FREE_ARRAY(string->chars, char, string->length + 1);
         FREE(ObjString, string);
         break;
      }
   }

}