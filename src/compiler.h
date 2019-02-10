#ifndef KI_COMPILER_H
#define KI_COMPILER_H

#include "common.h"
#include "vm.h"
#include "chunk.h"

bool compile(const char *source, VM *vm, Chunk *chunk);

#endif