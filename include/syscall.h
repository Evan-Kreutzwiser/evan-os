/*
 * evan-os/include/syscall.h
 * 
 * Declares functions for setting up system calls
 * 
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

void syscall_init(void);

uint64_t syscall_register(uint64_t id, uint64_t (*new_syscall) (uint64_t arg) );
uint64_t syscall_unregister(uint64_t id);

#endif 