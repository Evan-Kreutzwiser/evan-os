/*
 * evan-os/include/memory/paging.h 
 * 
 * Delcares functions for manipulating memory paging structures
 * 
 */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// 0x400 0000 0000 = 4 TiB: All physical memory is linearly mapped here
// Every pml4 entry represents 512 GiB, or half a TiB
#define IDENTITY_MAP_OFFSET 0x40000000000ul

#define PAGE_ADDRESS_MASK 0x000FFFFFFFFFF000ul
#define extract_entry_address(e) (e & PAGE_ADDRESS_MASK)
#define entry_present(e) (e & 0b1)

// Set up/prepare the page tables
void paging_init(void);


// Make the paging system use dynamic memory allocation instead of the boot time data area
void paging_enable_memory_allocation(void);


// Map a virtual addressto physical RAM
void paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint16_t flags, uint64_t* table_base);

// Unmap a page of memory to prevent access
void paging_unmap_page(uint64_t virtual_address, uint64_t* table_base);


// Set a page table entry to contain the given flags and address
void paging_set_entry(uint64_t* entry, uint64_t physical_address, uint16_t flags);


// Allocate pages starting from the given address
// Requested allocations are mapped for kernel use, so they do not need to specify the table
void paging_allocate_kernel_pages(void * start_address, uint64_t count, uint16_t flags);

// Allocate pages starting from the given address
// The address space pml4 pointer should point to the first of the 2 entries stored in the process control block
void paging_allocate_user_pages(void * start_address, uint64_t count, uint16_t flags, uint64_t* address_space_pml4);


// Creates and sets up a blank address space with the kernel mapped to it, and returns a pointer to the PML4 for loading it
void * paging_create_address_space(void);


// Get the physical address of a 4KB page's lowest level table entry
uint64_t* paging_get_pte_address(uint64_t virtual_address, uint64_t* table_base);

// Get the physical address mapped to the given virtual address
uint64_t paging_get_physical_address(uint64_t virtual_address);

// Get the physcial address mapped to a virtual address in the given paging table 
uint64_t paging_get_physical_address_from_table(uint64_t virtual_address, uint64_t* table_base);

// Allocate some pages to put a level of the page table in
void * paging_allocate_table_level(void);

// Deallocate a section of the page table data area to resuse later
void paging_free_table_level(void * pointer);


// Load a new virtual address space from the processes pml4
void paging_load_process_address_space(void * table_base_address);

#endif // PAGINH_H