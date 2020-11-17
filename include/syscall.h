/*
 * evan-os/include/syscall.h
 * 
 * Declares functions for setting up system calls
 * 
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef uint64_t (*syscall_t)(uint64_t, uint64_t, uint64_t, uint64_t);

void syscall_init(void);

uint64_t syscall_register(uint64_t id, syscall_t new_syscall, 
    __attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3);
uint64_t syscall_unregister(uint64_t id, __attribute__ ((unused)) uint64_t arg1, __attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3);

uint64_t execute_syscall(uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3);

uint64_t syscall_wrapper(uint64_t id, uint64_t arg0, uint64_t arg1, 
    __attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3);

#endif 