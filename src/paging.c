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

#define extract_entry_address(e) ((e & 0xFFFFFFFFFFFFF000))
#define entry_present(e) (e & 0b1)

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
    tty_print_string("Table base address: ");
    print_hex((uint64_t)page_table);
    tty_print_string("\nMemory size (Bytes): ");
    print_hex(max_physical_address);
    tty_print_char('\n');
}

uint64_t paging_get_physical_address(uint64_t virtual_address) {

    // Get the table indexes by breaking up the virtual address into 9-bit sections
    uint16_t page_table_index =             (virtual_address >> 12) & 0x1FF;
    uint16_t page_directory_index =         (virtual_address >> 21) & 0x1FF;
    uint16_t page_directory_pointer_index = (virtual_address >> 30) & 0x1FF;
    uint16_t page_level_4_index =           (virtual_address >> 39) & 0x1FF;

    // Work down the levels of the table to find the physical address

    if (entry_present(page_table[page_level_4_index]) == 0) {
        tty_print_string("\npml4 entry not present");
    }

    // Get the page directory pointer's address 
    uint64_t* table = (uint64_t*)extract_entry_address(page_table[page_level_4_index]);

    if (entry_present(table[page_directory_pointer_index]) == 0) {
        tty_print_string("\npdp entry not present");
    }

    // Get the page directory's address 
    table = (uint64_t*)extract_entry_address(table[page_directory_pointer_index]);

    if (entry_present(table[page_directory_index]) == 0) {
        tty_print_string("\npd entry not present");
    }

    // Get the page table's address 
    table = (uint64_t*)extract_entry_address(table[page_directory_index]);

    if (entry_present(table[page_table_index]) == 0) {
        tty_print_string("\npt entry not present");
    }

    // Get and return the physical address recorded in the page 
    return extract_entry_address(table[page_table_index]);
}


void paging_map_page(uint64_t virtual_address, uint64_t phyiscal_address, uint16_t flags) {

    // Get the table indexes by breaking up the virtual address into 9-bit sections
    uint16_t page_table_index =             (virtual_address >> 12) & 0x1FF;
    uint16_t page_directory_index =         (virtual_address >> 21) & 0x1FF;
    uint16_t page_directory_pointer_index = (virtual_address >> 30) & 0x1FF;
    uint16_t page_level_4_index =           (virtual_address >> 39) & 0x1FF;

    // Make sure all levels of the table are present and have addresses set

}

void paging_unmap_page(uint64_t virtual_address) {

    
}

// Load a new virtual address space from the page tables at the given address
void paging_load_address_space(uint64_t table_base_address) {

    // Load the new table address into the CR3 register 
    asm volatile ("mov %0, %%cr3;" : : "r" (table_base_address) : );

}

// Switch to the virtual address space where all of the RAM is identity mapped
void paging_load_identity_map_space(void) {

    // Load the new table address into the CR3 register 
    asm volatile ("mov %0, %%cr3;" : : "r" (page_table) : );

}
