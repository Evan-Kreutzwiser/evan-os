/*
 * evan-os/include/gdt.h
 * 
 * Declares functions for setting up memory segments. 
 * Although long mode uses a flat memory model, this 
 * allows the OS the chose its own selectors for the segments
 * 
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

void gdt_set_segment(uint8_t index, uint64_t offset, uint64_t limit, uint8_t access, uint8_t flags);
void gdt_set_tss(uint64_t index);
void gdt_init(void);

#endif
