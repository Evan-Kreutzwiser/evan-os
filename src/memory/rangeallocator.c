/*
 * evan-os/src/memory/rangeallocator.c
 *
 * Manages the allocation of physical memory ranges, for use by the page allocator
 * 
 */

#include <memory/rangeallocator.h>

#include <memory/paging.h>
#include <asm.h>
#include <tty.h>
#include <kernel.h>
#include <bootboot.h>

#include <stdint.h>

extern BOOTBOOT bootboot; // The bootboot structer, containing the memory map

uint64_t identity_map_pdp_table[512] __attribute__((aligned(4096)));
uint64_t identity_map_pd_tables[128][512] __attribute__((aligned(4096)));

// 0x20 0000 = 2 MiB

typedef struct range_t {
    struct range_t* next; // Points to the next range descriptor
    uint64_t base; // The ranges starting address
    uint64_t size; // The size of the address range in bytes
    uint8_t used; // If the block is used or not
} range_t;

range_t memory_map[32]; // Room for 32 range descriptors for the memory map

// Indentity maps the entire RAM. Run before paging_init
void identity_map_memory(void) {
    // Get the current (firmware generated) page table 
    uint64_t* pml4_table = (uint64_t*)cr3_read();

    // Get the phyiscal address of the new tables being made here
    uint64_t* pd_tables = (uint64_t*)paging_get_physical_address((uint64_t)&identity_map_pd_tables);
    uint64_t* pdp_table = (uint64_t*)paging_get_physical_address((uint64_t)&identity_map_pdp_table);
    
    // Get the index in the pml4 to map to
    uint16_t pml4_index = (IDENTITY_MAP_OFFSET >> 39) & 0x1FF;

    // Map the pdp table as a pml4 entry
    pml4_table[pml4_index] = ((uint64_t)&pdp_table[0] & PAGE_ADDRESS_MASK) | 0b11;

    // Create the page directory pointer, which must contain entries for every page     
    for (uint16_t index = 0; index < 128; index++) { // Fill every entry
        // Map the page directory
        paging_set_entry((uint64_t*)&pdp_table[index], (uint64_t)&pd_tables[index*5124],  0b000000011);
    }
    
    // Fill the whole table with 2 MiB identity map entries
    uint64_t address = 0;
    for (uint16_t tableNum = 0; tableNum < 128; tableNum++) { // For each table
        for (uint16_t index = 0; index < 512; index++) { // Fill every entry
            // Map the large page
            paging_set_entry((uint64_t*)&pd_tables[(tableNum*512)+index], address,  0b10000011);
            // Invalidate the tlb
            invlpg((void *)(address + IDENTITY_MAP_OFFSET));
            // Increase the address pointer
            address += 0x200000;
        }
    }
}

void read_memory_map(void) {

    // Get the number of entries
    uint8_t mmap_entry_count = (bootboot.size - 128) / 16;
        
    // If the memory map is too big 
    if (mmap_entry_count > 32) {
        tty_print_string("ERROR: memory map too large. Halting kernel.");
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