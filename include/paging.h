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

// Get the physical address mapped to the given virtual address
uint64_t paging_get_physical_address(uint64_t virtual_address);

#endif // PAGINH_H