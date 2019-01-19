#include <stdio.h>

#include "value.h"
#include "memory.h"

static void grow_value_array(ValueArray *array);

void print_value(Value value) {
	printf("%g", value);
}

void init_value_array(ValueArray *array) {
	array->capacity = 0;
	array->count = 0;
	array->values = NULL;
}

void free_value_array(ValueArray *array) {
	FREE_ARRAY(array->values, Value, array->capacity);
	init_value_array(array);
}

void write_value_array(ValueArray *array, Value value) {
	if (array->capacity < array->count + 1) {
		grow_value_array(array);
	}

	array->values[array->count] = value;
	++array->count;
}

static void grow_value_array(ValueArray *array) {
	int old_capacity = array->capacity;
	array->capacity = GROW_CAPACITY(array->capacity);
	array->values = GROW_ARRAY(array->values, Value, old_capacity, array->capacity);
}