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

// Used internally to hold information about memory ranges. *Should never be used outside rangeallocator.c*
typedef struct range_t {
    struct range_t* next; // Points to the next range descriptor
    uint64_t base; // The ranges starting address
    uint64_t size; // The size of the address range in bytes
    uint8_t used; // If the block is used or not
} range_t;

void identity_map_memory(void);

void read_memory_map(void);

// Allocate a region of memory that is 'size' bytes large, rounded upwards to the nearest page
uint64_t allocate_range(uint64_t size);

// Return a pointer to a free range descriptor array entry. *Should never be called from outside rangeallocator.c*
range_t* range_get_unused_entry(void);

#endif // RANGE_ALLOCATOR_H
