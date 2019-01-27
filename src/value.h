#ifndef QED_VALUE_H
#define QED_VALUE_H

#include "common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;

typedef enum {
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
	VAL_OBJ
} ValueType;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		Obj *obj;
	} as;
} Value;

// constructors
#define BOOL_VAL(value) ((Value) { VAL_BOOL, { .boolean = value } })
#define NIL_VAL         ((Value) { VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((Value) {VAL_NUMBER, { .number = value } }) 
#define OBJ_VAL(object) ((Value) { VAL_OBJ, { .obj = (Obj *)object }})

// unpacks the real value from Value struct
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

// type checkers
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

void print_value(Value value);

typedef struct {
	int capacity;
	int count;
	Value *values;
} ValueArray;

void init_value_array(ValueArray *array);
void free_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);

#endif