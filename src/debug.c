#include <stdio.h>

#include "common.h"
#include "debug.h"

static int simple_instruction(const char *instruction, int offset);

void disassemble_chunk(Chunk *chunk, const char *name) {
	printf(">>> %s <<<\n", name);
	for (int offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
}

int disassemble_instruction(Chunk *chunk, int offset) {
	printf("%04d\n", offset);

	uint8_t instruction = chunk->code[offset];
	switch(instruction) {
		case OP_RETURN:
			return simple_instruction("OP_RETURN", offset);
		default:
			printf("Unknown Opcode: %d\n", instruction);
			return offset + 1;
	}
}

static int simple_instruction(const char *instruction, int offset) {
	printf("%s\n", instruction);
	return offset + 1;
}