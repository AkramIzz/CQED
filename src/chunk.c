#include "chunk.h"
#include "memory.h"
#include "value.h"

static void grow_chunk(Chunk *chunk);

void init_chunk(Chunk *chunk) {
   chunk->count = 0;
   chunk->capacity = 0;
   chunk->code = NULL;
   chunk->lines = NULL;
   init_value_array(&chunk->constants);
}

void free_chunk(Chunk *chunk) {
   FREE_ARRAY(chunk->code, uint8_t, chunk->capacity);
   FREE_ARRAY(chunk->lines, int, chunk->capacity);
   free_value_array(&chunk->constants);
   
   init_chunk(chunk);
}

void write_chunk(Chunk *chunk, uint8_t byte, int line) {
   if (chunk->capacity < chunk->count + 1) {
      grow_chunk(chunk);
   }

   chunk->code[chunk->count] = byte;
   chunk->lines[chunk->count] = line;
   ++chunk->count;
}

static void grow_chunk(Chunk *chunk) {
   int old_capacity = chunk->capacity;
   chunk->capacity = GROW_CAPACITY(old_capacity);
   chunk->code = GROW_ARRAY(chunk->code, uint8_t, old_capacity, chunk->capacity);
   chunk->lines = GROW_ARRAY(chunk->lines, int, old_capacity, chunk->capacity);
}

int add_constant(Chunk *chunk, Value constant) {
   write_value_array(&chunk->constants, constant);
   return chunk->constants.count - 1; 
}