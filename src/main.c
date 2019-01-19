#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
	Chunk chunk;
	init_chunk(&chunk);
	
	int constant_index = add_constant(&chunk, 1);
	write_chunk(&chunk, OP_CONSTANT, 122);
	write_chunk(&chunk, constant_index, 122);
	write_chunk(&chunk, OP_RETURN, 122);
	
	disassemble_chunk(&chunk, "test");
	free_chunk(&chunk);
	return 0;
}