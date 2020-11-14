/*
 * evan-os/src/syscall.c
 * 
 * Manages the creation and usage of system calls
 * 
 */

#include <syscall.h>

#include <interrupt.h>
#include <tty.h>

#include <stdint.h>


__attribute__((packed))
struct registers {



};

extern uint64_t syscall_and_return asm("syscall_and_return");

// 256 entry long list of syscalls with a 64 bit return value and argument,
// for passing sinlge values or struct pointers
uint64_t (*syscall[256]) (uint64_t, uint64_t, uint64_t, uint64_t);


__attribute__((interrupt))
void syscall_interrupt_handler(struct interrupt_frame *frame) {

	// Get the requested syscall's id and the argument
	uint64_t syscall_id;
	uint64_t arg0, arg1, arg2, arg3;
	uint64_t return_value;
		
	asm volatile ("" : "=b" (syscall_id), "=c" (arg0), "=d" (arg1) :: );

	// Check that the syscall exists
	if (syscall[syscall_id] != 0) {
		// If it does, call the syscall
		return_value = syscall[syscall_id](arg0, arg1, arg2, arg3);
		// Return the return value in rdx
		asm volatile ("mov %%rbx, %0" : "=m" (syscall_id) :: );
	}

	// Change the saved rbx register value to the syscall's return value
	asm volatile ("	pop %%rax; pop %%rdx; pop %%rcx; pop %%rbx; \
					mov %0, %%rbx ; \
					push %%rbx; push %%rcx; push %%rdx; push %%rax;"
					:: "m" (return_value) :);
}

asm  ("	syscall_and_return: \
		call syscall_interrupt_handler; \
		sysret; ");

void syscall_init(void) {

	// Register the sysscall interrupt
	interrupt_set_gate(0x80, (uint64_t)&syscall_and_return, INTERRUPT_PRESENT | INTERRUPT_RING_3 | INTERRUPT_INTERRUPT_GATE);

	// Add baseline system interrupts to the array
	syscall[0] = &syscall_register;
	syscall[1] = &syscall_unregister;
	// TODO: Add interrupt setting syscalls
	// TODO: Add file system syscalls

	// Set the segments that syscall will set
	// Set STAR to the segment selectors
	wrmsr(0xC0000081, 0x8, 0);
	
	// Set the LSTAR MSR to the 64 bit syscall entry point
	wrmsr(0xC0000082, (uint32_t)(syscall_and_return&0xffffffff), (uint32_t)(syscall_and_return >> 32) );

	// Set the syscall bit
    wrmsr(0xC0000080, 1, 0); /*rdmsr_low(0xC0000080) | 1, rdmsr_high(0xC0000080));
*/
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



uint64_t syscall_wrapper(uint64_t id, __attribute__ ((unused)) uint64_t arg0, __attribute__ ((unused)) uint64_t arg1, 
	__attribute__ ((unused)) uint64_t arg2, __attribute__ ((unused)) uint64_t arg3) {

	uint64_t return_value;

	asm volatile ("	mov %1, %%rbx; mov %2, %%rcx; mov %3, %%rdx; \
					syscall; \
					mov %%rbx, %0;" 
					: "=m" (return_value) : "m" (id), "m" (arg0), "m" (arg1) : "rbx", "rcx", "rdx");

	return return_value;
}