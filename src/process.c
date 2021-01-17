/*
 * evan-os/src/process.c
 * 
 * Manages processes 
 * 
 */

#include <process.h>

#include <kernel.h>
#include <memory/paging.h>
#include <tty.h>
#include <syscall.h>

#include <stdint.h>

uint64_t next_pid = 0; // The pid given to the next created process 

process_control_block_t processes[64]; // List of all process control blocks
// TODO: Replace this very soon with a more scalable method

// Create a new process and return its control block
process_control_block_t* process_create(void) {

    // TODO: Add a mutex or other method to prevent the attempt the creation of multiple process on one block
    
    processes[next_pid].pid = next_pid;
    // Create a new address space for this process
    processes[next_pid].address_space_pml4 = paging_create_address_space();
    
    next_pid++;

    return &processes[next_pid-1];
}

// Enter a usermode process
// TODO: Replace with thread entering
void process_enter(process_control_block_t process) {

    // Load the process's address space
    paging_load_process_address_space(process.address_space_pml4);

    // TODO: Load segment registers from process control block

    // Set the data segment registers to the user segments,
    // then restore the contents of the process's registers,
    // and finish off by faking a return from an interrupt to jump to the process (with the user stack)
    asm volatile("movq $0x1b, %%rax; \
            movq %%rax, %%ds; \
            movq %%rax, %%es; \
            \
            movq %2, %%rax; \
            movq %3, %%rbx; \
            movq %4, %%rcx; \
            movq %5, %%rdx; \
            movq %6, %%r8; \
            movq %7, %%r9; \
            movq %8, %%r10; \
            movq %9, %%r11; \
            movq %10, %%r12; \
            movq %11, %%r13; \
            movq %12, %%r14; \
            movq %13, %%r15; \
            movq %14, %%rsi; \
            movq %15, %%rdi; \
            \
            pushq $0x1b; \
            pushq %1; \
            pushq %16; \
            pushq $0x23; \
            pushq %0; \
            iretq;" : : "m" (process.rip), "m" (process.rsp), 
                "m" (process.rax), "m" (process.rbx), "m" (process.rcx), "m" (process.rdx), // %2-5
                "m" (process.r8), "m" (process.r9), "m" (process.r10), "m" (process.r11),   // %6-9
                "m" (process.r12), "m" (process.r13), "m" (process.r14), "m" (process.r15), // %10-13
                "m" (process.rsi), "m" (process.rdi), "m" (process.flags)                   // %14-16
                : );
}

void user_code(void) {
    // Get the current stack segment
    uint64_t segment;
    asm volatile ("mov %%ss, %0;" : "=r" (segment) ::);
    syscall_wrapper(2, segment, 0x0, 0x0, 0x0);
    asm volatile ("ud2");
}
