#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "value.h"

static void print_line_info(Chunk *chunk, int offset);

static int simple_instruction(const char *instruction, int offset);
static int constant_instruction(const char *instruction, Chunk *chunk, int offset);

void disassemble_chunk(Chunk *chunk, const char *name) {
	printf(">>> %s <<<\n", name);
	for (int offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
}

int disassemble_instruction(Chunk *chunk, int offset) {
	printf("%04d ", offset);
	print_line_info(chunk, offset);

	uint8_t instruction = chunk->code[offset];
	switch(instruction) {
		case OP_CONSTANT:
			return constant_instruction("OP_CONSTANT", chunk, offset);
		case OP_NEGATE:
			return simple_instruction("OP_NEGATE", offset);
		case OP_RETURN:
			return simple_instruction("OP_RETURN", offset);
		default:
			printf("Unknown Opcode: %d\n", instruction);
			return offset + 1;
	}
}

static void print_line_info(Chunk *chunk, int offset) {
	if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
		printf("   | ");
	} else {
		printf("%4d ", chunk->lines[offset]);
	}
}

static int simple_instruction(const char *instruction, int offset) {
	printf("%s\n", instruction);
	return offset + 1;
}

static int constant_instruction(const char *instruction, Chunk *chunk, int offset) {
	uint8_t constant_index = chunk->code[offset + 1];

	printf("%-16s %4d '", instruction, constant_index);
	print_value(chunk->constants.values[constant_index]);
	printf("'\n");

	return offset + 2;
}