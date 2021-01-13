
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define PROCESS_STATUS_RUNNING 0x0
#define PROCESS_STATUS_

typedef struct process_control_block_t {

    // Process information
    uint64_t pid; // The Process ID
    uint64_t status;
    uint64_t* address_space_pml4; // Points to a partially filled pml4, to be copied into the in-use one during process entry

    // Registers
    uint64_t rip; // Instruction pointer

    uint64_t rsp; // Stack pointer
    uint64_t rbp; // Base pointer

    uint64_t rax; // General purpose registers
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t rsi; // String operation registers
    uint64_t rdi;

    uint64_t flags; // Flags register

    uint16_t cs; // Memory segments // Will allow for preempting the kernel when supported
    uint16_t ds;
    uint16_t es;

} process_control_block_t;

// Create a new process and return the control block
process_control_block_t* process_create();

// Enter a usermode process
void process_enter(process_control_block_t process);

void user_code(void);

#endif
