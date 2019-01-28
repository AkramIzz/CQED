#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include "value.h"
#include "object.h"

static void reset_stack(VM *vm);
static InterpretResult run(VM *vm);
static bool is_falsy(Value value);
static bool values_equal(Value a, Value b);
static void concatenate(VM *vm);
static void runtime_error(VM *vm, const char *format, ...);

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

Value peek(VM *vm, int distance) {
      // stack_top points past the last element so we need to subtract 1 to
      // get the last element
      return vm->stack_top[-1 - distance];
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
   
   #define BINARY_OP(result_type, op) \
      do { \
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
                  runtime_error(vm, "Operands must be numbers"); \
                  return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop(vm)); \
            double a = AS_NUMBER(pop(vm)); \
            push(vm, result_type(a op b)); \
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
         case OP_NIL: push(vm, NIL_VAL); break;
         case OP_TRUE: push(vm, BOOL_VAL(true)); break;
         case OP_FALSE: push(vm, BOOL_VAL(false)); break;
         case OP_EQUAL: {
            Value b = pop(vm);
            Value a = pop(vm);
            push(vm, BOOL_VAL(values_equal(a, b)));
            break;
         }
         case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
         case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
         case OP_ADD: {
            if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
               concatenate(vm);
            } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
               double b = AS_NUMBER(pop(vm));
               double a = AS_NUMBER(pop(vm));
               push(vm, NUMBER_VAL(a + b));
            } else {
               runtime_error(vm, "Operands must be two numbers or two strings");
               return INTERPRET_RUNTIME_ERROR;
            }
            break;
         }
         case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
         case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
         case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
         case OP_NOT:
            push(vm, BOOL_VAL(is_falsy(pop(vm)))); break;
         case OP_NEGATE:
            if (!IS_NUMBER(peek(vm, 0))) {
                  runtime_error(vm, "Operand must be a number");
                  return INTERPRET_RUNTIME_ERROR;
            }

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

static bool is_falsy(Value value) {
   if (IS_BOOL(value) && AS_BOOL(value) == false) return true;
   if (IS_NIL(value)) return true;
   if (IS_NUMBER(value) && AS_NUMBER(value) == 0) return true;

   return false;
}

static bool values_equal(Value a, Value b) {
   if (a.type != b.type) return false;

   switch(a.type) {
      case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
      case VAL_NIL: return true;
      case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
      case VAL_OBJ: {
         ObjString *a_string = AS_STRING(a);
         ObjString *b_string = AS_STRING(b);
         return a_string->length == b_string->length
            && memcmp(a_string->chars, b_string->chars, a_string->length) == 0;
      }
   }
}

static void concatenate(VM *vm) {
   ObjString *b = AS_STRING(pop(vm));
   ObjString *a = AS_STRING(pop(vm));

   int length = a->length + b->length;
   char *chars = ALLOCATE(char, length + 1);
   memcpy(chars, a->chars, a->length);
   memcpy(chars + a->length, b->chars, b->length);
   chars[length] = '\0';

   ObjString *result = take_string(chars, length);
   push(vm, OBJ_VAL(result));
}

static void runtime_error(VM *vm, const char *format, ...) {
      va_list args;
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
      fputs("\n", stderr);

      size_t instruction = vm->ip - vm->chunk->code;
      fprintf(stderr, "[line %d] in script\n",
            vm->chunk->lines[instruction]);

      reset_stack(vm);
}