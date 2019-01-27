#ifndef QED_MEMORY_H
#define QED_MEMORY_H

#define ALLOCATE(type, count) \
	(type*)reallocate(NULL, 0, sizeof(type) * (count))

#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(array, type, old_count, new_count) \
	(type*)reallocate((array), sizeof(type) * (old_count), sizeof(type) * (new_count))

#define FREE_ARRAY(array, type, count) \
	reallocate((array), sizeof(type) * (count), 0)

// This one function handles memory allocating, deallocating and reallocating
// We need all memory allocations in the interpreter to pass through it in order to be able
//   to implement a garbage collector
// Given a 0 [new_size], it frees the memory pointed to by [previous] and returns NULL
// Given a 0 [old_size], it allocates new memory and return a pointer to it
// Given non-zero [old_size] and [new_size] it resizes the memory pointed to by [previous]
//   and return a pointer pointing to the resized memory (may differ from [previous]) 
void* reallocate(void *previous, size_t old_size, size_t new_size);

#endif