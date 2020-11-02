/*
 * evan-os/src/syscall.c
 * 
 * Manages the creation and usage of system calls
 * 
 */

#include <syscall.h>

#include <interrupt.h>

#include <stdint.h>


// 256 entry long list of syscalls with a 64 bit return value and argument,
// for passing sinlge values or struct pointers
uint64_t (*syscall[256]) (uint64_t);


__attribute__((interrupt))
void syscall_interrupt_handler(struct interrupt_frame *frame) {

	// Get the requested syscall's id and the argument
	uint64_t syscall_id;
	uint64_t argument;
	
	asm volatile ("mov %%rbx, %0; mov %%rdx, %1" : "=r" (syscall_id), "=r" (argument) :: );

	uint64_t return_value;
	
	// Check that the syscall exists
	if (syscall[syscall_id] != 0) {
		// If it does, call the syscall
		return_value = syscall[syscall_id](argument);
		// Return the return value in rdx
		asm volatile ("mov %0, %%rbx " :: "m" (syscall_id) : );
	}
}

void syscall_init(void) {

	// Register the sysscall interrupt
	interrupt_set_gate(80, (uint64_t)&syscall_interrupt_handler, INTERRUPT_PRESENT | INTERRUPT_RING_3 | INTERRUPT_INTERRUPT_GATE);

	// Add baseline system interrupts to the array
	//syscall_register();
}

uint64_t syscall_register(uint64_t id, uint64_t (*new_syscall) (uint64_t arg)) {

	// TODO: Permission checking and error codes
	if (syscall[id] != 0) {
		syscall[id] = new_syscall;
		return 0;
	}

	return 1;
}

uint64_t syscall_unregister(uint64_t id) {

	// TODO: Permission checking
	syscall[id] = 0;


	return 0;
}

