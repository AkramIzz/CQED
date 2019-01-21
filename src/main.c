#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
	VM vm;
	init_vm(&vm);

	Chunk chunk;
	init_chunk(&chunk);
	
	int constant_index = add_constant(&chunk, 1);
	write_chunk(&chunk, OP_CONSTANT, 122);
	write_chunk(&chunk, constant_index, 122);
	write_chunk(&chunk, OP_NEGATE, 122);
	write_chunk(&chunk, OP_RETURN, 122);
	
	disassemble_chunk(&chunk, "test");
	interpret(&vm, &chunk);
	free_vm(&vm);
	free_chunk(&chunk);
	return 0;
}