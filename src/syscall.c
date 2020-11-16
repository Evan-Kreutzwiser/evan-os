/*
 * evan-os/src/syscall.c
 * 
 * Manages the creation and usage of system calls
 * 
 */

#include <syscall.h>

#include <interrupt.h>
#include <tty.h>
#include <kernel.h>

#include <stdint.h>


__attribute__((packed))
struct registers {



};

// 256 entry long list of syscalls with a 64 bit return value and argument,
// for passing sinlge values or struct pointers
uint64_t (*syscall[256]) (uint64_t, uint64_t, uint64_t, uint64_t);

__attribute__((naked))
void syscall_and_return(void) {
	
	// Store the rcx register
	asm volatile ("push %rcx");

	asm volatile (" call syscall_interrupt_handler; ");

	// Restore the rcx register and return
	asm volatile ("push %rcx; sysret");
}

uint64_t execute_syscall(uint64_t id, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3) {

	// Check that the syscall exists
	if (syscall[id] != 0 && id < 256) {
		// If it does, call the syscall
		return syscall[id](arg0, arg1, arg2, arg3);
	}
}

void syscall_init(void) {

	// Register the sysscall interrupt
	interrupt_set_gate(0x80, (uint64_t)&syscall_and_return, INTERRUPT_PRESENT | INTERRUPT_RING_3 | INTERRUPT_INTERRUPT_GATE);

	tty_print_string("call 2 address: ");
	print_hex(&syscall_unregister);
	tty_print_char('\n');

	// Add baseline system interrupts to the array
	syscall[0] = (uint64_t)&syscall_register;
	syscall[1] = (uint64_t)&syscall_unregister;
	// TODO: Add interrupt setting syscalls
	// TODO: Add file system syscalls

	tty_print_string("call 2 address: ");
	print_hex(syscall[1]);
	tty_print_char('\n');

	// Set the segments that syscall will set
	// Set STAR to the segment selectors
	wrmsr(0xC0000081, 0x0, 0x8);
	
	tty_print_string("Set segment\n");

	// Set the syscall bit
    wrmsr(0xC0000080, rdmsr_low(0xC0000080) | 1, rdmsr_high(0xC0000080));

	tty_print_string("Set syscall bit\n");

	print_hex(syscall_and_return);

	// Set the LSTAR MSR to the 64 bit syscall entry point
	wrmsr(0xC0000082, (uint32_t)(&syscall_and_return), (uint32_t)((uint64_t)&syscall_and_return >> 32) );

	tty_print_string("Set entry address to:\n");
	print_hex(rdmsr(0xC0000082));
}


uint64_t syscall_register(uint64_t id, uint64_t (*new_syscall) (uint64_t, uint64_t, uint64_t, uint64_t), 
	__attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3) {

	// TODO: Permission checking and error codes
	if (id >= 4) {
		// Set the system call to the passed function pointer
		tty_print_string("Added a syscall\n");
		syscall[id] = new_syscall;
		return 0;
	}

	return 1;
}


uint64_t syscall_unregister(uint64_t id, __attribute__ ((unused)) uint64_t arg1, __attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3) {

	// TODO: Permission checking

	// Void the system call
	syscall[id] = 0;


	return 0;
}



uint64_t syscall_wrapper(uint64_t id, uint64_t arg0, uint64_t arg1, 
	__attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3) {

	uint64_t return_value;

	asm volatile ("	mov %1, %%rbx; mov %2, %%rdx; mov %3, %%r8; \
					syscall; \
					mov %%rbx, %0;" 
					: "=m" (return_value) : "m" (id), "m" (arg0), "m" (arg1) : "rbx", "rdx", "r8");

	return return_value;
}