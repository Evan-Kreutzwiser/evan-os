/*
 * evan-os/src/memory/paging.c
 * 
 * Manages page tables and the mapping of virtual addresses to physical RAM
 * 
 */

#include <memory/paging.h>

#include <kernel.h>
#include <memory/memory.h>
#include <memory/rangeallocator.h>
#include <tty.h>
#include <asm.h>
#include <bootboot.h>

#include <stdint.h>

// Get a bit from a bitmap
#define bitmap_get_bit(bitmap, index, bit) (bitmap[index] & (1 << bit))
// Set a bit of a bitmap to 1
#define bitmap_set_bit(bitmap, index, bit)   (((uint8_t*)bitmap)[index] |= (1 << bit))
// Set a bit of a bitmap to 0
#define bitmap_unset_bit(bitmap, index, bit) (((uint8_t*)bitmap)[index] &= ~(1 << bit))

extern BOOTBOOT bootboot; // Bootboot structure

extern uint8_t* page_bitmap; // Page usage bitmap
uint64_t page_bitmap_size; // Size of the bitmap

uint64_t *page_table; //[512][512][512][512];
uint64_t kernel_pml4_entry[512];

// Only half of these 16 slots can be used due to alignment requirements
uint64_t table_allocation_area[16][512] __attribute__((aligned(4096)));

uint8_t allocation_map[8];

uint64_t max_physical_address = 0;

// Set to 1 when the paging system can allocate new tables using malloc
uint8_t use_malloc = 0;

void paging_init(void) {

    // Save the address of the page tables created by the uefi
    page_table = (uint64_t*)cr3_read();

    // Map ALL of the ram linearly at a large offset
    identity_map_memory();

    // Read the memory map to get the usable ranges of memory
    read_memory_map();

    // Save the kernel pml4 table level
    memcpy(kernel_pml4_entry, page_table, 4096);

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

// Make the paging system use dynamic memory allocation instead of the boot time data area
// void paging_enable_memory_allocation(void);

uint64_t* paging_get_pte_address(uint64_t virtual_address) {
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

    return &table[page_table_index];
}

uint64_t paging_get_physical_address(uint64_t virtual_address) {

    uint64_t* address = paging_get_pte_address(virtual_address);

    tty_print_string("PTE at: ");
    print_hex((uint64_t)address);

    // Get and return the physical address recorded in the page, and the offset into the page
    return extract_entry_address(address[0]) | (virtual_address & 0xfff);
}


// Allocate a group of pages



void paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint16_t flags) {

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

    // Mark it as present and set the flags and address
    paging_set_entry((uint64_t*)&table_pointer[page_table_index], physical_address, flags);

    invlpg((uint64_t*)virtual_address);
}

void paging_unmap_page(uint64_t virtual_address) {
    
    // Get the page entry address
    uint64_t* entry = paging_get_pte_address(virtual_address);

    // Remove the present bit
    entry[0] &= ~1;

    // Refresh the cache to account for the missing page
    invlpg((void*)virtual_address);

    // TODO: Count how many pages are used to allow for deallocating unneeded table segments
}

void paging_set_entry(uint64_t* entry, uint64_t physical_address, uint16_t flags) {
    // Fill in the entry with the physical address of the next page level or the memory being mapped, and the flags
    entry[0] = (physical_address & PAGE_ADDRESS_MASK) | flags;
}


// Allocate pages starting from the given address
uint64_t paging_allocate_pages(void * start_address, uint64_t count) {




}


// Creates and sets up a blank address space with the kernel mapped to it, and returns a pointer to the PML4 for loading it
void * paging_create_address_space(void) {

    // Allocate a table level for the pml4
    uint64_t* pml4_pointer = (uint64_t*)paging_allocate_table_level();

    print_hex((uint64_t)pml4_pointer);

    // Zero the allocated table
    for (uint16_t i = 0; i < 512; i++) {
        pml4_pointer[i] = 0;
    }

    // Copy the top level table to the newly allocated table
    // TODO: Remove mappings of lower addresses to make room for process address spaces
    memcpy(pml4_pointer, kernel_pml4_entry, 4096);

    // Return the new address space
    return pml4_pointer;
}


// Allocate some pages to put a level of the page table in
void * paging_allocate_table_level(void) {

    //if (use_malloc) {
      //  return kmalloc(2048);
    //}

    // A level of the page table can neatly fit into a single page
    // If the page allocation tracking supports it, 2 groups of 512 entries can fit in a page

    // Find the first available page
    for (int i = 0; i < 8; i++) {
        // Check if the page is unused
        if (allocation_map[i] == 0) {
            // Mark it as used
            allocation_map[i] = 1;
            // Return a pointer to that section of the data area
            return &table_allocation_area[i*2][0];
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
    for (int i = 0; i < 8; i++) {
        // Check if the pointer matches
        if (pointer == &table_allocation_area[i*2][0]) {
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
void paging_load_address_space(void * table_base_address) {

    tty_print_string("\nLoading new table: ");
    print_hex((uint64_t)table_base_address);
    // Load the new table address into the CR3 register 
    asm volatile ("mov %0, %%cr3;" : : "r" (table_base_address) : );

}
