/*
 * evan-os/include/paging.h 
 * 
 * Delcares functions for manipulating memory paging structures
 * 
 */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Set up/prepare the page tables
void paging_init(void);

// Map a virtual addressto physical RAM
void paging_map_page(uint64_t virtual_address, uint64_t phyiscal_address, uint16_t flags);
// Unmap a page of memory to prevent access
void paging_unmap_page(uint64_t virtual_address);

uint64_t paging_get_pte_address(uint64_t virtual_address);
// Get the physical address mapped to the given virtual address
uint64_t paging_get_physical_address(uint64_t virtual_address);

// Allocate some pages to put a level of the page table in
void * paging_allocate_table_level();
// Deallocate a section of the page table data area to resuse later
void paging_free_table_level(void * pointer);

// Load a new virtual address space from the page tables at the given address
void paging_load_address_space(uint64_t table_base_address);
// Load the address space where the ram is identity mapped
void paging_load_identity_map_space(void);

#endif // PAGINH_H