/*
 * evan-os/src/paging.c
 * 
 * Manages page tables and the mapping of virtual addresses to physical RAM
 * 
 */

#include <paging.h>

#include <kernel.h>
#include <tty.h>
#include <asm.h>
#include <bootboot.h>

#include <stdint.h>

#define PAGE_ADDRESS_MASK 0x000FFFFFFFFFF000
#define extract_entry_address(e) ((e & PAGE_ADDRESS_MASK))
#define entry_present(e) (e & 0b1)

extern BOOTBOOT bootboot; // Bootboot structure

uint64_t *page_table; //[512][512][512][512];

uint64_t *table_allocation_area[16][512] __attribute__((aligned(4096)));

uint8_t allocation_map[16];

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

    // Allocate 3 table levels
    void * pointer = paging_allocate_table_level();
    void * pointer2 = paging_allocate_table_level();
    void * pointer3 = paging_allocate_table_level();

    // Free the second one
    paging_free_table_level(pointer2);

    // Allocate a 4th area (should take the space of the freed second area)
    void * pointer4 = paging_allocate_table_level();

    tty_print_string("Startup page allocator ");
    if (pointer4 == pointer2) { tty_print_string("check passed\n"); }
    else { tty_print_string("check failed\n"); }

    // Free the test pointers
    paging_free_table_level(pointer);
    paging_free_table_level(pointer3);
    paging_free_table_level(pointer4);

    // Print out the memory size
    tty_print_string("Table base address: ");
    print_hex((uint64_t)page_table);
    tty_print_string("\nMemory size (Bytes): ");
    print_hex(max_physical_address);
    tty_print_char('\n');
}

uint64_t paging_get_pte_address(uint64_t virtual_address) {
        // Get the table indexes by breaking up the virtual address into 9-bit sections
    uint16_t page_table_index =             (virtual_address >> 12) & 0x1FF;
    uint16_t page_directory_index =         (virtual_address >> 21) & 0x1FF;
    uint16_t page_directory_pointer_index = (virtual_address >> 30) & 0x1FF;
    uint16_t page_level_4_index =           (virtual_address >> 39) & 0x1FF;

    // Work down the levels of the table to find the physical address

    if (entry_present(page_table[page_level_4_index]) == 0) {
        tty_print_string("\npml4 entry not present");
        return 0; // Return null
    }

    // Get the page directory pointer's address 
    uint64_t* table = (uint64_t*)extract_entry_address(page_table[page_level_4_index]);

    if (entry_present(table[page_directory_pointer_index]) == 0) {
        tty_print_string("\npdp entry not present");
        return 0; // Return null
    }

    // Get the page directory's address 
    table = (uint64_t*)extract_entry_address(table[page_directory_pointer_index]);

    if (entry_present(table[page_directory_index]) == 0) {
        tty_print_string("\npd entry not present");
        return 0; // Return null
    }

    // Get the page table's address 
    table = (uint64_t*)extract_entry_address(table[page_directory_index]);

    if (entry_present(table[page_table_index]) == 0) {
        tty_print_string("\npt entry not present");
        return 0; // Return null
    }

    return (uint64_t)&table[page_table_index];
}

uint64_t paging_get_physical_address(uint64_t virtual_address) {

    uint64_t* address = (uint64_t*)paging_get_pte_address(virtual_address);

    // Get and return the physical address recorded in the page 
    return extract_entry_address(address[0]);
}


void paging_map_page(uint64_t virtual_address, uint64_t phyiscal_address, uint16_t flags) {

    // Get the table indexes by breaking up the virtual address into 9-bit sections
    uint16_t page_table_index =             (virtual_address >> 12) & 0x1FF;
    uint16_t page_directory_index =         (virtual_address >> 21) & 0x1FF;
    uint16_t page_directory_pointer_index = (virtual_address >> 30) & 0x1FF;
    uint16_t page_level_4_index =           (virtual_address >> 39) & 0x1FF;

    uint64_t* table_pointer;

    // Make sure all levels of the table are present and have addresses set
    
    // If the top level table entry is not present, allocate the next level 
    if (entry_present(page_table[page_level_4_index]) == 0) {
        // Allocate a new table level, put the address in the table, and mark it as present
        page_table[page_level_4_index] = ((uint64_t)paging_allocate_table_level() & PAGE_ADDRESS_MASK) | 1;
        tty_print_string("Allocated new PDP\n");
    }

    // Store a pointer to the next level of the table
    table_pointer = (uint64_t*)extract_entry_address(page_table[page_level_4_index]);

    // If the next level of the table entry is not present, allocate it 
    if (entry_present(table_pointer[page_directory_pointer_index]) == 0) {
        // Allocate a new table level, put the address in the table, and mark it as present
        table_pointer[page_directory_pointer_index] = ((uint64_t)paging_allocate_table_level() & PAGE_ADDRESS_MASK) | 1;
        tty_print_string("Allocated new PD\n");
    }

    // Store a pointer to the next level of the table
    table_pointer = (uint64_t*)extract_entry_address(table_pointer[page_directory_pointer_index]);

    // If the next level of the table entry is not present, allocate it 
    if (entry_present(table_pointer[page_directory_index]) == 0) {
        // Allocate a new table level, put the address in the table, and mark it as present
        table_pointer[page_directory_index] = ((uint64_t)paging_allocate_table_level() & PAGE_ADDRESS_MASK) | 1;
        tty_print_string("Allocated new PT\n");
    }

    // Store a pointer to the page entry for the requested address
    table_pointer = (uint64_t*)extract_entry_address(table_pointer[page_directory_index]);

    // Mark it as present and set the flags
    table_pointer[page_table_index] = (phyiscal_address & PAGE_ADDRESS_MASK) | flags;

    invlpg((uint64_t*)virtual_address);
}

void paging_unmap_page(uint64_t virtual_address) {
    tty_print_string("\nERROR: Page unmapping not supported\n");
}

// Allocate some pages to put a level of the page table in
void * paging_allocate_table_level() {

    // A level of the page table can neatly fit into a single page
    // If the page allocation tracking supports it, 2 groups of 512 entries can fit in a page

    // Find the first available page
    for (int i = 0; i < 16; i++) {
        // Check if the page is unused
        if (allocation_map[i] == 0) {
            // Mark it as used
            allocation_map[i] = 1;
            // Return a pointer to that section of the data area
            return &table_allocation_area[i][0];
        }

    }

    // Return null if no available spacce was found
    tty_print_string("\nERROR: cannot allocate page table level (Out of space)\n");
    return 0;
}

// Deallocate a section of the page table data area to resuse later
void paging_free_table_level(void * pointer) {

    // Validate the pointer
    if ((uint64_t)pointer < (uint64_t)&table_allocation_area[0][0] || (uint64_t)pointer > (uint64_t)&table_allocation_area[15][0]) {
        // If the pointer is out of range, throw an error
        tty_print_string("\nERROR: Attempt to free invalid pointer\n");
        return;
    }

    // Compare the pointer with all of the array indices
    // TODO: FIND A BETTER WAY TO DO THIS
    for (int i = 0; i < 16; i++) {
        // Check if the pointer matches
        if (pointer == &table_allocation_area[i][0]) {
            // Check that the area was actually in use
            if (allocation_map[i] != 0) {
                // Mark the area as free
                allocation_map[i] = 0;
                return;
            }
            // Otherwise through an error
            else {
                tty_print_string("\nERROR: Attempt to free unused data\n");
                return;
            }
        }
    }

    // If the pointer didnt match any spaces
    tty_print_string("\nERROR: Area to free not found\n");

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
