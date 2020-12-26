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

typedef struct range_descriptor_t {

    struct range_descriptor_t * next;
    void * start;
    size_t size;
} range_descriptor_t;

void identity_map_memory(void);

#endif // RANGE_ALLOCATOR_H
