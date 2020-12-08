/*
 * evan-os/src/paging.c
 * 
 * Manages page tables and the mapping of virtual addresses to physical RAM
 * 
 */

#include <paging.h>

#include <kernel.h>
#include <tty.h>
#include <bootboot.h>

#include <stdint.h>

extern BOOTBOOT bootboot; // Bootboot structure

uint64_t *page_table; //[512][512][512][512];

uint64_t max_physical_address = 0;

void paging_init(void) {

    // Save the address of the page tables created by the uefi
    asm volatile ("mov %%cr3, %%rax; mov %%rax, %0;" : "=m" (page_table) : : "rax");


    // Use the memory map to get the size of the physical RAM
    uint8_t mmap_entry_count = (bootboot.size - 128) / 16; // Get the number of entries
    
    MMapEnt* mmap = (MMapEnt*)&bootboot.mmap.ptr; // Get the pointer to the memory map
    
    for (uint8_t i = 0; i < mmap_entry_count; i++) {
        // Add the entry's size to the maximum address
        max_physical_address += MMapEnt_Size(mmap);
        mmap++;
    }
    
    // Print out the memory size
    tty_print_string("Memory size (Bytes): ");
    print_hex(max_physical_address);
    tty_print_char('\n');
}
