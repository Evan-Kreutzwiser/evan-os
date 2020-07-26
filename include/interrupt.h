/*
 * evan-os/include/interrupt.h
 * 
 * Declares functions for managing interrupts
 * 
 */


#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#define INTERRUPT_EXCEPTION_TYPE	0b10000000 // 0b1000
#define INTERRUPT_INTERRUPT_GATE 	0xe // 64 Bit interrupt gate
#define INTERRUPT_TRAP_GATE 		0xf // 64 Bit trap gate (return to next instruction)
// OR a gate type and type value together to make the type_attributes parameter for interrupt setting

void interrupt_set_gate(uint8_t index, uint64_t address, uint8_t type_attributes);

void interrupt_mask(uint8_t interrupt);
void interrupt_unmask(uint8_t interrupt);

void interrupt_load_table(void);

#endif // INTERRUPT_H
