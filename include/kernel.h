
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <interrupt.h>

// Kernel entry point
void _start(void);

void double_fault(interrupt_frame *frame); // TODO: Add error code to paramters
void gp_fault(interrupt_frame *frame);
void div_0_fault(void);

#endif
