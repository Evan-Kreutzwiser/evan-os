/*
 * evan-os/include/interrupt.h
 * 
 * Declares functions for managing interrupts
 * 
 */


#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <stdbool.h>

#define INTERRUPT_PRESENT			0b10000000
#define INTERRUPT_RING_0            0b00000000 // Priviledge level 0
#define INTERRUPT_RING_3            0b01100000 // Priviledge level 3
#define INTERRUPT_INTERRUPT_GATE 	0xe // 64 Bit interrupt gate
#define INTERRUPT_TRAP_GATE 		0xf // 64 Bit trap gate (return to next instruction)
// OR a gate type and type value together to make the type_attributes parameter for interrupt setting

// The structure of information saved when exceptions or interrupts are tiggered
// Holds information used to restore the state of the cpu from before the interrupt
// This also allows the use of gcc's interrupt attribute
struct interrupt_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};

typedef void (*interrupt_handler_t) (void);


void interrupt_init(void);

void interrupt_handler(uint64_t interrupt_num);

void interrupt_set_gate(uint8_t index, uint64_t address, uint8_t type_attributes);

void interrupt_register(uint8_t index, interrupt_handler_t handler);
void interrupt_unregister(uint8_t index);

void interrupt_mask(uint8_t interrupt);
void interrupt_unmask(uint8_t interrupt);

void interrupt_load_table(void);

// Set whether the OS will use the newer APIC or the old PIC for interrupts
void interrupt_set_mode(bool use_apic); 

// Send an eoi to the PIC chips
void interrupt_end_pic(uint8_t index);

#endif // INTERRUPT_H
