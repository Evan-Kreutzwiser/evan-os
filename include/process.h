
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef struct process_control_block_t {

    uint64_t pid; // The Process ID
    uint64_t status;
    uint64_t* address_space_pml4; // Points to a partially filled pml4, to be copied into the in-use one during process entry
    // Registers
    uint64_t rip; // Instruction pointer
    uint64_t rsp; // Stack pointer
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;

} process_control_block_t;

// Create a new process and return the control block
process_control_block_t* process_create();

// Enter a usermode process
void process_enter(process_control_block_t process);

void user_code(void);

#endif
