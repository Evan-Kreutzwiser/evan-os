/*
 * evan-os/include/memory.h
 * 
 * Declares functions for dynamically allocating memory.
 * 
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Allocate a chunk of memory in the requested size
void* kmalloc(size_t size);
// Release an allocated section of memory
uint64_t kfree(void * pointer);


#endif // MEMORY_H