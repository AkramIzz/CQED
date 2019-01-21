#ifndef QED_VM_H
#define QED_VM_H

#include "chunk.h"

typedef struct {
   Chunk *chunk;
   // instruction pointer
   uint8_t *ip;
} VM;

typedef enum {
   INTERPRET_OK,
   INTERPRET_COMPILE_ERROR,
   INTERPRET_RUNTIME_ERROR
} InterpretResult;

void init_vm(VM *vm);
void free_vm(VM *vm);
InterpretResult interpret(VM *vm, Chunk *chunk);

#endif