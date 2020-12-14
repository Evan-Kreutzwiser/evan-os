/*
 * evanos/src/asm.c
 * 
 * Contatins functions that represent single assembly commands used by the OS and drivers
 * 
 */


#include <asm.h>

#include <stdint.h>

// Interrupts

inline void sti(void) {
    asm volatile ("sti");
}

inline void cli(void) {
    asm volatile ("cli");
}

// Stop the computer

inline void hlt(void) {
    asm volatile ("hlt");
}

// Model Specific Register manupulation

inline void wrmsr(uint32_t msr_id, uint32_t low, uint32_t high) {
	// Write the value to the msr
	asm volatile (" mov %0, %%eax; \
		mov %1, %%edx; \
		mov %2, %%ecx; \
		wrmsr;" : : "m"(low), "m"(high), "m"(msr_id) : "eax", "edx", "ecx");
}

inline uint64_t rdmsr(uint32_t msr_id) {
	uint64_t output;
	uint32_t low = 0, high = 0;
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

inline uint32_t rdmsr_low(uint32_t msr_id) {
	uint32_t low = 0;
	// Read the value from the msr and take only the low 32 bits
	asm volatile ("	mov %1, %%ecx; \
		rdmsr; \
		mov %%eax, %0; \
	" : : "m"(low), "m"(msr_id) : "eax", "edx", "ecx");

	return low;
}

inline uint32_t rdmsr_high(uint32_t msr_id) {
	uint32_t high = 0;
	// Read the value from the msr and take only the high 32 bits
	asm volatile ("	mov %1, %%ecx; \
		rdmsr; \
		mov %%edx, %0; \
	" : : "m"(high), "m"(msr_id) : "eax", "edx", "ecx");

	return high;
}

// Outputing to io ports

inline void outportb(uint16_t port, uint8_t data) {
    
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

inline void outportw(uint16_t port, uint16_t data) {

    asm volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

inline void outportl(uint16_t port, uint32_t data) {

    asm volatile ("outl %0, %1" : : "a"(data), "Nd"(port));
}

// Input through io ports

inline uint8_t inportb(uint16_t port) {
    
	uint8_t volatile data;
	asm volatile ("inb %%dx,%%al":"=a" (data) : "d" (port));
	return data;
}

inline uint16_t inportw(uint16_t port) {

	uint16_t volatile data;
	asm volatile ("inw %%dx,%%ax":"=a" (data) : "d" (port));
	return data;
}

inline uint32_t inportl(uint16_t port) {

	uint32_t volatile data;
	asm volatile ("inl %%dx,%%eax":"=a" (data) : "d" (port));
	return data;
}

// Invalidate Page

inline void invlpg(void * page_address) {
	// Invalidate / flush a page of memory
	asm volatile("invlpg (%0);" : : "r" (page_address) : "memory");
}
