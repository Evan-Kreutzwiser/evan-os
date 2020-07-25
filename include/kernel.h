
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>


// Kernel entry point
void _start(void);

void double_fault(void);
void gp_fault(void);
void div_0_fault(void);

#endif