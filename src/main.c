#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
	VM vm;
	init_vm(&vm);

	Chunk chunk;
	init_chunk(&chunk);
	
	int constant_index = add_constant(&chunk, 5);
	write_chunk(&chunk, OP_CONSTANT, 100);
	write_chunk(&chunk, constant_index, 100);

	constant_index = add_constant(&chunk, 3);
	write_chunk(&chunk, OP_CONSTANT, 100);
	write_chunk(&chunk, constant_index, 100);
	
	write_chunk(&chunk, OP_ADD, 100);
	
	constant_index = add_constant(&chunk, 2);
	write_chunk(&chunk, OP_CONSTANT, 100);
	write_chunk(&chunk, constant_index, 100);
	
	write_chunk(&chunk, OP_DIVIDE, 100);
	write_chunk(&chunk, OP_NEGATE, 122);
	write_chunk(&chunk, OP_RETURN, 122);
	
	disassemble_chunk(&chunk, "test");
	interpret(&vm, &chunk);
	free_vm(&vm);
	free_chunk(&chunk);
	return 0;
}