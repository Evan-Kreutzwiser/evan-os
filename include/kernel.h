
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <interrupt.h>

// Kernel entry point
void _start(void);

void double_fault(struct interrupt_frame *frame, uint64_t error_code); 
void gp_fault(struct interrupt_frame *frame, uint64_t error_code);
void div_0_fault(struct interrupt_frame *frame);

#endif
