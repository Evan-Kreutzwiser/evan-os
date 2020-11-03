
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <interrupt.h>

// Kernel entry point
void _start(void);

void double_fault(struct interrupt_frame *frame, uint64_t error_code); 
void gp_fault(struct interrupt_frame *frame, uint64_t error_code);
void div_0_fault(struct interrupt_frame *frame);
void invalid_opcode_fault(struct interrupt_frame *frame);

void print_val(uint64_t value, uint8_t bits);

#endif
