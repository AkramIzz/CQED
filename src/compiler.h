#ifndef QED_COMPILER_H
#define QED_COMPILER_H

#include "common.h"
#include "vm.h"
#include "chunk.h"

bool compile(const char *source, VM *vm, Chunk *chunk);

#endif