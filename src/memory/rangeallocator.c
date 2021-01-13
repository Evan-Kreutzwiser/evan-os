/*
 * evan-os/src/memory/rangeallocator.c
 *
 * Manages the allocation of physical memory ranges, for use by the page allocator
 * This file also contains the functions used to read the memory map and map the full RAM at an offset
 * 
 */

#include <memory/rangeallocator.h>

#include <memory/paging.h>
#include <asm.h>
#include <tty.h>
#include <kernel.h>
#include <bootboot.h>

#include <stdint.h>

#define MMAP_ARRAY_SIZE 32

extern BOOTBOOT bootboot; // The bootboot structer, containing the memory map

uint64_t identity_map_pdp_table[512] __attribute__((aligned(4096)));
uint64_t identity_map_pd_tables[128][512] __attribute__((aligned(4096)));

range_t memory_map[MMAP_ARRAY_SIZE]; // Space for range descriptors representing the memory map, and later the broken up segments allocated here

// Indentity maps the entire RAM
void identity_map_memory(void) {

    // Get the current (firmware generated) page table 
    uint64_t* pml4_table = (uint64_t*)(cr3_read()+IDENTITY_MAP_OFFSET);

    // Get the phyiscal address of the new tables being made here
    uint64_t* pdp_table_physical_address = (uint64_t*)paging_get_physical_address((uint64_t)&identity_map_pdp_table[0]);
    uint64_t* pd_tables_physical_address = (uint64_t*)paging_get_physical_address((uint64_t)&identity_map_pd_tables[0]);

    // Create the page directory pointer, which must contain entries for every page     
    for (uint16_t index = 0; index < 128; index++) { // Fill every entry
        // Map the page directory
        paging_set_entry((uint64_t*)&identity_map_pdp_table[index], (uint64_t)&pd_tables_physical_address[index*512], 0b000000011);
    }

    // Fill the whole table with 2 MiB identity map entries
    uint64_t address = 0;
    for (uint16_t table_num = 0; table_num < 128; table_num++) { // For each table
        for (uint16_t index = 0; index < 512; index++) { // Fill every entry
            // Map the large page
            paging_set_entry((uint64_t*)(&identity_map_pd_tables[table_num][index]), address, 0b10000011);
            // Invalidate the tlb
            invlpg((void *)(address+IDENTITY_MAP_OFFSET));
            // Increase the address pointer
            address += 0x200000;
        }
    }

    // Get the index in the pml4 to map too
    uint16_t pml4_index = (IDENTITY_MAP_OFFSET >> 39) & 0x1FF;

    // Map the pdp table as a pml4 entry
    paging_set_entry(&pml4_table[pml4_index], (uint64_t)pdp_table_physical_address, 0b11);
}

void read_memory_map(void) {

    // Get the number of entries
    uint8_t mmap_entry_count = (bootboot.size - 128) / 16;
        
    // If the memory map is too big 
    if (mmap_entry_count >= MMAP_ARRAY_SIZE) {
        tty_print_string("ERROR: memory map too large. Halting kernel.");
    }

    // Zero the table
    for (uint64_t i = 0; i < MMAP_ARRAY_SIZE; i++) {
        // Only .next and.base are checked to find unused entries
        memory_map[i].next = NULL;
        memory_map[i].base = 0;
    }

    // Get the pointer to the memory map
    MMapEnt* mmap = (MMapEnt*)&bootboot.mmap.ptr;

    // Record every index of the map   
    for (uint8_t index = 0; index < mmap_entry_count; index++) {
        // Copy the entry's base address
        memory_map[index].base = mmap[index].ptr;
        // Copy the entry's size
        memory_map[index].size = mmap[index].size & 0xFFFFFFFFFFFFFFF0;
        // Set the availability of the area based on the mmap entry type
        uint16_t entry_type = (mmap[index].size & 0xF);
        // Store available ram as unused
        if (entry_type == 1)      { memory_map[index].used = 0; }
        // ACPI and memory IO areas are marked as "used" but with different numbers to tell them apart from the rest  
        else if (entry_type != 0) { memory_map[index].used = entry_type; }
        // Mark unusable areas as used to prevent them from being allocated
        else                      { memory_map[index].used = 1; }

        // Link the list items together
        memory_map[index].next = (range_t*)&memory_map[index+1];
    }
    // The last memory range entry should not have a next entry
    memory_map[mmap_entry_count-1].next = NULL; 
}

// Allocate a region of memory that is 'size' bytes large, rounded upwards to the nearest page
uint64_t allocate_range(uint64_t size) {

    // Round the size upwards to the nearest page
    // size += 0x1000 - (size & 0xfff); 

    // Find the first unused range of the requested size
    for (uint64_t i = 0; i < MMAP_ARRAY_SIZE; i++) {
        // If the current descriptor is free and has enough space
        if (memory_map[i].used == 0 && memory_map[i].size >= size) {
            // Mark this entry as used
            memory_map[i].used = 1;
            // Get an unused entry to split this one with
            range_t* new_entry = range_get_unused_entry();
            // Fill in the new entry
            new_entry->size = memory_map[i].size - size; // It fills the space not being allocated with the old descriptor
            new_entry->base = memory_map[i].base + size;
            new_entry->next = memory_map[i].next;
            new_entry->used = 0; // Make sure it is marked as unused
            // Modify the descriptor of the range being allocated to include the new block,
            memory_map[i].next = new_entry;
            // And shrink it to fit the allocated amount
            memory_map[i].size = size;

            // Return the physical address of the allocated range
            return memory_map[i].base;
        }
    }
    // If the range could not be allocated
    tty_print_string("ERROR: Range allocator list full or large enough space not found.");
    return -1;
}

// TODO: Plan and implement deallocation of memory ranges that are no longer needed (eg. the memory of a killed process)

// Get an unused range descriptor, used to split ranges
range_t* range_get_unused_entry(void) {
    // Search for an unused entry and return it
    for (uint64_t i = 0; i < MMAP_ARRAY_SIZE; i++) {
        // If the descriptor's next block and base addresses are both null, then it should be unused
        if (memory_map[i].next == NULL && memory_map[i].base == 0) {
            // Return a pointer to the entry
            return &memory_map[i];
        }
    }
    // If no free entry was found
    tty_print_string("ERROR: Range allocator list full.");
    return NULL;
}