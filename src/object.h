#ifndef QED_OBJECT_H
#define QED_OBJECT_H

#include "common.h"
#include "value.h"

typedef enum {
   OBJ_STRING
} ObjType;

struct sObj {
   ObjType type;
};

struct sObjString {
   Obj obj;
   int length;
   char *chars;
};

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

// using a function because [value] is used twice (in function) thus if 
// we wrote it here directly it will result in evaluating the [value]
// expression twice!
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) AS_STRING(value)->chars

static inline bool is_obj_type(Value value, ObjType type) {
   return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* take_string(char *chars, int length);
ObjString* copy_string(const char *chars, int length);
void print_obj(Value value);

#endif