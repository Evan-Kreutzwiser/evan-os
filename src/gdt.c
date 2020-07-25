
#include <gdt.h>

#include <stdint.h>

#define GDT_FLAGS_32_BIT 0b1100
#define GDT_FLAGS_64_BIT 0b1110

#define GDT_ACCESS_CODE_0 0b10011010
#define GDT_ACCESS_DATA_0 0b10010010

#define GDT_ACCESS_CODE_3 0b11111010
#define GDT_ACCESS_DATA_3 0b11110010

typedef struct gdt_pointer_t {

    uint64_t base; // The address of the table
    uint16_t limit; // The size of the table

} __attribute__((packed)) gdt_pointer_t;

gdt_pointer_t gdt_ptr;

uint64_t gdt[16];

void gdt_set_segment(uint8_t index, uint64_t offset, uint64_t limit, uint8_t access, uint8_t flags) {

	gdt[index] = 0; // Reset the value to avoid mixing previouslt set segments

	gdt[index] |= limit & 0xffff; // Low 16 bits of limit
	gdt[index] |= (limit & 0xff0000) << 48; // Mid 4 bits of offset

	gdt[index] |= (offset & 0xffff) << 16; // Low 16 bits of offset
	gdt[index] |= (offset & 0xff0000) << 32; // Mid 8 bits of offset
	gdt[index] |= (offset & 0xff000000) << 56; // High 8 bits of offset

	gdt[index] |=  ((uint64_t)flags & 0xf ) << 52; // Flags
	gdt[index] |=  ((uint64_t)access & 0xff ) << 40; // Access


}

void gdt_load(void) {


}

// Set up the kernel's memory segments
void gdt_init(void) {

	// Fill the GDT with our own memory segments
	gdt_set_segment(0, 0, 0xffffffff, 0x0, 0x0); // Null segment
	gdt_set_segment(1, 0, 0xffffffff, GDT_ACCESS_CODE_0 , GDT_FLAGS_64_BIT); // Kernel code
	gdt_set_segment(2, 0, 0xffffffff, GDT_ACCESS_DATA_0, GDT_FLAGS_64_BIT); // Kernel data
	gdt_set_segment(3, 0, 0xffffffff, GDT_ACCESS_CODE_3, GDT_FLAGS_64_BIT); // User code
	gdt_set_segment(4, 0, 0xffffffff, GDT_ACCESS_DATA_3, GDT_FLAGS_64_BIT); // User data

	// Set up the pointer
	gdt_ptr.base = (uint64_t)(uint64_t*)gdt;
	gdt_ptr.limit = (4 * 5) -1;

	// Load the new table

	// Load the gdt and reset the segment registers
    asm volatile (" lgdt %0; \
	mov $0x10, %%ax; \
	mov %%ax, %%ds; \
	mov %%ax, %%es; \
	mov %%ax, %%fs; \
	mov %%ax, %%gs; \
	mov %%esp, %%eax; \
	push $0x08; \
	#push %%eax; \
	pushf; \
	push $0x1B; \
	push $flush_end; \
	iret; \
	flush_end: \
	" : : "m" (gdt_ptr));

}