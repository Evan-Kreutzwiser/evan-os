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

#include <stdint.h>

uint64_t identity_map_pdp_table[512] __attribute__((aligned(4096)));
uint64_t identity_map_pd_tables[256][512] __attribute__((aligned(4096)));
// To maintain page alignment, the 128 PD tables are spaced apart inside an area twice the size

// 0x20 0000 = 2 MiB

// Indentity maps the entire RAM. Run before paging_init
void identity_map_memory(void) {
    // Get the page table
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
        paging_set_entry((uint64_t*)&pdp_table[index], (uint64_t)&pd_tables[index*1024],  0b000000011);
        //tty_print_string("\nMapped pd: ");
        //print_hex((uint64_t)&pd_tables[index*1024]);
        //tty_print_string(" with pdp entry at ");
        //print_hex((uint64_t)&pdp_table[index]);
        //tty_print_string(" to make: ");
        //print_hex(pdp_table[index]);
    }
    
    // Fill the whole table with 2 MiB identity map entries
    uint64_t address = 0;
    for (uint16_t tableNum = 0; tableNum < 128; tableNum++) { // For each table
        for (uint16_t index = 0; index < 512; index++) { // Fill every entry
            // Map the large page
            paging_set_entry((uint64_t*)&pd_tables[(tableNum*1024)+index], address,  0b10000011);
            // Invalidate the tlb
            invlpg((void *)(address + IDENTITY_MAP_OFFSET));
            // Increase the address pointer
            address += 0x200000;
        }
    }
}
