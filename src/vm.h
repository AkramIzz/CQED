#ifndef QED_VM_H
#define QED_VM_H

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
   Chunk *chunk;
   // instruction pointer
   uint8_t *ip;
   Value stack[STACK_MAX];
   Value *stack_top;
   Table strings;
   Obj *objects;
} VM;

typedef enum {
   INTERPRET_OK,
   INTERPRET_COMPILE_ERROR,
   INTERPRET_RUNTIME_ERROR
} InterpretResult;

void init_vm(VM *vm);
void free_vm(VM *vm);
void push(VM *vm, Value value);
Value pop(VM *vm);
InterpretResult interpret(VM *vm, const char *source);

#endif