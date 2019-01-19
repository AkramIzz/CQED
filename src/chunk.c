#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

static void grow_chunk(Chunk *chunk);

void init_chunk(Chunk *chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
}

void free_chunk(Chunk *chunk) {
	FREE_ARRAY(chunk->code, uint8_t, chunk->capacity);
	init_chunk(chunk);
}

void write_chunk(Chunk *chunk, uint8_t byte) {
	if (chunk->capacity < chunk->count + 1) {
		grow_chunk(chunk);
	}

	chunk->code[chunk->count] = byte;
	++chunk->count;
}

static void grow_chunk(Chunk *chunk) {
	int old_capacity = chunk->capacity;
	chunk->capacity = GROW_CAPACITY(old_capacity);
	chunk->code = GROW_ARRAY(chunk->code, uint8_t, old_capacity, chunk->capacity);
}