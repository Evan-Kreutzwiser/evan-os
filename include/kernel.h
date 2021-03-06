
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <interrupt.h>

// Kernel entry point
void _start(void);

// Kernel main
void kernel(void);

// Exception handlers
void double_fault(struct interrupt_frame *frame /*, uint64_t error_code*/ ); 
void gp_fault(struct interrupt_frame *frame, uint64_t error_code);
void page_fault(struct interrupt_frame *frame, uint64_t error_code);
void div_0_fault(struct interrupt_frame *frame);
void invalid_opcode_fault(struct interrupt_frame *frame);

// Value printing
void print_val(uint64_t value, uint8_t bits);
void print_hex(uint64_t value);

// Number functions
uint64_t octal_string_to_int(char* octal_string, uint64_t length);

#endif
