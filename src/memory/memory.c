/*
 * evan-os/src/memory/memory.c
 * 
 * Manages kernel memory allocation
 * 
 */

#include <memory/memory.h>

#include <memory/paging.h>
#include <tty.h>
#include <kernel.h>

#include <stdint.h>

extern uint8_t heap_location;
uint64_t heap_physical_location;

void * kernel_heap;

uint64_t heap_size = 0;
void* heap_end;

typedef struct memory_block_t{
    size_t size;
    uint8_t in_use; // 0 = Free, 1 = Allocated

} memory_block_t;

void memory_allocation_init(void) {

    // Create the heap now that the location is recorded
    memory_allocation_expand_heap(131072); // Allocate 128 4KB pages for the heap

    // Make a block header marking the whole thing as free
    memory_block_t* first_block = (memory_block_t*)&heap_location;
    // Set up the first block
    first_block->in_use = 0;
    first_block->size = heap_size-sizeof(memory_block_t); // Fills the whole heap
}

void memory_allocation_expand_heap(size_t new_size) {

    // Remove the first 12 bits of the address. Only deal with complete pages
    new_size &= ~0xfff;
    // Calculate how many new pages must be allocated
    uint64_t pages = (new_size - heap_size) / 4096;

    // Allocate the new pages
    paging_allocate_kernel_pages(&heap_location + heap_size, pages, 0b011);

    // Update the pointer to the end of the heap
    heap_end = (void*)(&heap_location + heap_size);
}

// Allocate a chunk of memory in the requested size
void* kmalloc(size_t size) {

    // Start at the first block
    memory_block_t* block = (memory_block_t*)&heap_location;

    while ((uint64_t)block < (uint64_t)heap_end) {

        // Check if the block is free 
        if (block->in_use == 0) {
            // Check if the block is large enough for the requested area
            if (block->size >= size) {

                // Is there space for a new block to be allocated?
                if (block->size > size + sizeof(memory_block_t)) {

                    // Make a new block after this one
                    memory_block_t* new_block = (memory_block_t*)((uint64_t)block + size + sizeof(memory_block_t));

                    // The new block is not being used yet
                    new_block->in_use = 0; 
                     // This block is the size of what wasnt used of the other block, minus the tiny space for this header
                    new_block->size = block->size - size - sizeof(memory_block_t);

                    // Reduce the blocks size
                    block->size = size;
                }
                // If there is not, then leave the extra space in this section

                // Mark this allocated block as used
                block->in_use = 1;

                // Return the address of the allocated area
                return (void*)((uint64_t)block + sizeof(memory_block_t));
            }
        }

        // Go to the next block
        block = (memory_block_t*)((uint64_t)block + block->size + sizeof(memory_block_t));
    }

    return NULL;
}

// Release an allocated section of memory
uint64_t kfree(void * pointer) {

    // Make sure the pointer isnt outside the heap
    if ((uint64_t)pointer < (uint64_t)&heap_location || (uint64_t)pointer > (uint64_t)heap_end) {
        return -1; // Pointer our of bounds
    }

    // Get the block descriptor for the memory the caller wants freed
    memory_block_t* block = (memory_block_t*)((uint64_t)pointer-sizeof(memory_block_t));

    // Mark the block as not in use
    block->in_use = 0;

    return -1;
}

// Reallocate data with a new size
void* krealloc(void * pointer, size_t new_size) {

    // If the caller requested 0 bytes of memory, free their old block and dont allocate a new one 
    if (new_size == 0) {
        kfree(pointer);
        return NULL;
    }

    // Extract the old blocks size from the pointer
    size_t old_size = ((memory_block_t*)( (uint64_t)pointer - sizeof(memory_block_t)) )->size;
    
    // Allocate a new space in memory for the data    
    void* new_pointer = kmalloc(new_size);

    if (new_pointer != NULL) {
        // Copy the data into the new location
        memcpy(new_pointer, pointer, old_size);
        // Free the old block
        kfree(pointer);
        // Return the new pointer
        return new_pointer;
    }

    return NULL;
}

// Copy a section of memory from one location to another
void* memcpy(void* dest, void* src, size_t size) {
    
    // TODO: This needs some serious optimization

    char* destination = (char*)dest;
    char* source = (char*)src;

    for (size_t n = 0; n < size; n++) {
        destination[n] = source[n];
    }

    return dest;
}
