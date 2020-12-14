/*
 * evan-os/src/memory.c
 * 
 * Manages kernel memory allocation
 * 
 */

#include <memory.h>

#include <tty.h>

#include <stdint.h>

typedef struct memory_block_t{

    struct memory_block_t* next_block;
    size_t size;
    uint8_t is_free;

} memory_block_t;

// Allocate a chunk of memory in the requested size
void* kmalloc(size_t size) {

    return 0;
}

// Release an allocated section of memory
uint64_t kfree(void * pointer) {

    memory_block_t* descriptor = pointer;

    if (descriptor->is_free == 0) {
        
        // Mark the memory as free
        descriptor->is_free = 1;

        return 0;
    }
    else {
        tty_print_string("\nError: Tried to free unallocated memory\n");
        return -1; // Return an error
    }
}