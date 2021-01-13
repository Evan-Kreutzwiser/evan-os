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

    asm volatile("movq %0, %%rdx; \
            movq $0x1b, %%rax; \
            movq %%rax, %%ds; \
            movq %%rax, %%es; \
            pushq $0x1b; \
            pushq %2; \
            pushfq; \
            pushq $0x23; \
            pushq %1;\
            iretq;" : : "m" (process.rdx), "m" (process.rip), "m" (process.rsp) : );

}

void user_code(void) {
    // Get the current stack segment
    uint64_t segment;
    asm volatile ("mov %%ss, %0;" : "=r" (segment) ::);
    syscall_wrapper(2, segment, 0x0, 0x0, 0x0);
    asm volatile ("ud2");
}
