/*
 * evanos/src/asm.c
 * 
 * Contatins functions that represent single assembly commands used by the OS and drivers
 * 
 */


#include <asm.h>

#include <stdint.h>

// Interrupts

void sti(void) {
    asm volatile ("sti");
}

void cli(void) {
    asm volatile ("cli");
}

// Stop the computer

void hlt(void) {
    asm volatile ("hlt");
}

// Model Specific Register manupulation

void wrmsr(uint32_t msr_id, uint32_t low, uint32_t high) {
	// Write the value to the msr
	asm volatile (" mov %0, %%eax; \
		mov %1, %%edx; \
		mov %2, %%ecx; \
		wrmsr;" : : "m"(low), "m"(high), "m"(msr_id) : "eax", "edx", "ecx");
}

uint64_t rdmsr(uint32_t msr_id) {
	uint64_t output;
	uint32_t low, high;
	// Read the value from the msr and combine the 2 32 bit values returned
	asm volatile ("	mov %2, %%ecx; \
		rdmsr; \
		mov %%eax, %0; \
		mov %%edx, %1; \
	" : : "m"(low),"m"(high), "m"(msr_id) : "eax", "edx", "ecx");

	output = (uint64_t)high << 32;
	output |= low;

	return output;
}

uint32_t rdmsr_low(uint32_t msr_id) {
	uint32_t low;
	// Read the value from the msr and take only the low 32 bits
	asm volatile ("	mov %1, %%ecx; \
		rdmsr; \
		mov %%eax, %0; \
	" : : "m"(low), "m"(msr_id) : "eax", "edx", "ecx");

	return low;
}

uint32_t rdmsr_high(uint32_t msr_id) {
	uint32_t high;
	// Read the value from the msr and take only the high 32 bits
	asm volatile ("	mov %1, %%ecx; \
		rdmsr; \
		mov %%edx, %0; \
	" : : "m"(high), "m"(msr_id) : "eax", "edx", "ecx");

	return high;
}

// Outputing to io ports

void outportb(uint16_t port, uint8_t data) {
    
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void outportw(uint16_t port, uint16_t data) {

    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

void outportl(uint16_t port, uint32_t data) {

    asm volatile ("outl %0, %1" : : "a"(data), "Nd"(port));
}

// Input through io ports

uint8_t inportb(uint16_t port) {
    
	uint8_t volatile data;
	asm volatile ("inb %%dx,%%al":"=a" (data) : "d" (port));
	return data;
}

uint16_t inportw(uint16_t port) {

	uint16_t volatile data;
	asm volatile ("inw %%dx,%%ax":"=a" (data) : "d" (port));
	return data;
}

uint32_t inportl(uint16_t port) {

	uint32_t volatile data;
	asm volatile ("inl %%dx,%%eax":"=a" (data) : "d" (port));
	return data;
}
