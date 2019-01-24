#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include "value.h"

static void reset_stack(VM *vm);
static InterpretResult run(VM *vm);

void init_vm(VM *vm) {
      reset_stack(vm);
}

static void reset_stack(VM *vm) {
      vm->stack_top = vm->stack;
}

void free_vm(VM *vm) {

}

void push(VM *vm, Value value) {
      *vm->stack_top = value;
      ++vm->stack_top;
}

Value pop(VM *vm) {
      --vm->stack_top;
      return *vm->stack_top;
}

InterpretResult interpret(VM *vm, const char *source) {
   Chunk chunk;
   init_chunk(&chunk);

   if (!compile(source, &chunk)) {
      free_chunk(&chunk);
      return INTERPRET_COMPILE_ERROR;
   }

   vm->chunk = &chunk;
   vm->ip = vm->chunk->code;

   InterpretResult result = run(vm);
   
   free_chunk(&chunk);
   return result;
}

static InterpretResult run(VM *vm) {
   #define READ_BYTE() (*vm->ip++)
   #define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
   
   #define BINARY_OP(op) \
      do { \
            double b = AS_NUMBER(pop(vm)); \
            double a = AS_NUMBER(pop(vm)); \
            push(vm, NUMBER_VAL(a op b)); \
      } while (false)

   for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
      printf("         ");
      for (Value *slot = vm->stack; slot < vm->stack_top; ++slot) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
      }
      printf("\n");
      disassemble_instruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif

      uint8_t instruction;
      switch(instruction = READ_BYTE()) {
         case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(vm, constant);
            printf("\n");
            break;
         }
         case OP_ADD: BINARY_OP(+); break;
         case OP_SUBTRACT: BINARY_OP(-); break;
         case OP_MULTIPLY: BINARY_OP(*); break;
         case OP_DIVIDE: BINARY_OP(/); break;
         case OP_NEGATE:
            push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));
            break;
         case OP_RETURN: {
            print_value(pop(vm));
            printf("\n");
            return INTERPRET_OK;
         }
      }
   }

   #undef READ_BYTE
   #undef READ_CONSTANT
   #undef BINARY_OP
}