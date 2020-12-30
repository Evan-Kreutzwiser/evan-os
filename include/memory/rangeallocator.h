/*
 * evan-os/inc/memory/rangeallocator.h
 *
 * Manages the allocation of physical memory ranges, for use by the page allocator
 * 
 */

#ifndef RANGE_ALLOCATOR_H
#define RANGE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

void identity_map_memory(void);

void read_memory_map(void);

#endif // RANGE_ALLOCATOR_H
