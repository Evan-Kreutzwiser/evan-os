
#include <gdt.h>

#include <stdint.h>

#define GDT_FLAGS_32_BIT 0b1100
#define GDT_FLAGS_64_BIT 0b1110

#define GDT_ACCESS_CODE_0 0b10011010
#define GDT_ACCESS_DATA_0 0b10010010

// 0x0000e093 should probably be:
// 0x0000a09b

// and 0x0000c093 for data

#define GDT_ACCESS_CODE_3 0b11111010
#define GDT_ACCESS_DATA_3 0b11110010

typedef struct gdt_pointer_t {

    uint16_t limit; // The size of the table
    uint64_t base; // The address of the table

} __attribute__((packed)) gdt_pointer_t;

gdt_pointer_t gdt_ptr;

uint8_t exeption_stack[4096]; // 1 KB of stack for exceptions

typedef struct gdt_entry_t {
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, determine what ring this segment can be used in.
   uint8_t  granularity;
   uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_tss_entry_t {
	uint16_t limit_low;          // The lower 16 bits of the limit.
	uint16_t base_low;           // The lower 16 bits of the base.
	uint8_t  base_middle_low;    // The next 8 bits of the base.
	uint8_t  access;             // Access flags, determine what ring this segment can be used in.
	uint8_t  granularity;
	uint8_t  base_middle_high;   // 8 more bits of the base.
	uint32_t base_high; 		 // The last/highest 32 bits of the base address
	uint32_t zero; 		 // Set to all 0s	
} __attribute__((packed)) gdt_tss_entry_t;

typedef struct tss_t {
	uint32_t reserved;
	
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;

	uint64_t reserved2;

	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;

	uint64_t resevred3;
	uint32_t reserved4;

	uint32_t iopb_offset;
} __attribute__((packed)) __attribute__((aligned(4096))) tss_t;

gdt_entry_t gdt[16];
tss_t tss;


void gdt_set_segment(uint8_t index, uint64_t offset, uint64_t limit, uint8_t access, uint8_t flags) {

	//gdt[index] = 0; // Reset the value to avoid mixing previouslt set segments

	gdt[index].limit_low = (uint16_t)(limit & 0xffff); // Low 16 bits of limit
	
	gdt[index].base_low = (uint16_t)(offset & 0xffff); // Low 16 bits of offset
	
	gdt[index].base_middle = (uint8_t)((offset & 0xff0000) >> 16); // Mid 8 bits of offset
	
	gdt[index].granularity = (uint8_t)((limit & 0x0f0000) >> 16); // Mid 8 bits of limit
	gdt[index].granularity |=  (uint8_t)(flags & 0xf ) << 4; // Flags
	
	gdt[index].base_high = (uint8_t)((offset & 0xff000000) >> 24); // High 8 bits of offset

	gdt[index].access =  (uint8_t)(access & 0xff); // Access
}

void gdt_set_tss(uint64_t index) {

	// Get a pointer to the entry (tss entry fills up 2 slots)
	gdt_tss_entry_t* tss_entry = (gdt_tss_entry_t*)&gdt[index];

	tss_entry->base_low = (uint16_t)((uint64_t)&tss & 0xffff); // Low 16 bits of base
	tss_entry->base_middle_low = (uint8_t)(((uint64_t)&tss & 0xff0000) >> 16); // Mid 8 bits of base
	
	tss_entry->base_middle_high = (uint8_t)((uint64_t)&tss >> 24);

	tss_entry->base_high = (uint32_t)((uint64_t)&tss >> 32);
	
	tss_entry->limit_low = (uint16_t)(sizeof(tss_t) & 0xffff); // Low 16 bits of limit
	tss_entry->granularity = (uint8_t)((sizeof(tss_t) & 0x0f0000) >> 16); // Mid 8 bits of limit
	
	tss_entry->granularity |= (uint8_t)0b00010000;

	tss_entry->access = 0b10001001; // Type 0b1001

	tss_entry->zero = 0;
}

// Set up the kernel's memory segments
void gdt_init(void) {

	// Fill the GDT with our own memory segments
	gdt_set_segment(0, 0, 0xffffffff, 0x0, 0x0); // Null segment
	
	gdt_set_segment(1, 0, 0xffffffff, GDT_ACCESS_CODE_0, 0b0010); // Kernel code
	gdt_set_segment(2, 0, 0xffffffff, GDT_ACCESS_DATA_0, 0b0000); // Kernel data
	
	// Due to how syscalls set selectors upon return, the user data segment must come before the code segment
	gdt_set_segment(3, 0, 0xffffffff, GDT_ACCESS_DATA_3, 0b0000); // User data
	gdt_set_segment(4, 0, 0xffffffff, GDT_ACCESS_CODE_3, 0b0010); // User code

	// Set up a tss at index 5	
	gdt_set_tss(5);

	tss.rsp0 = (uint64_t)&exeption_stack[4096];

	// Set up the pointer
	gdt_ptr.base = (uint64_t)(uint64_t*)gdt;
	gdt_ptr.limit = (sizeof(gdt_entry_t)*16) -1;

	// Load the new table
	// Load the gdt and reset the segment registers
    asm volatile (" lgdt %0; \
	mov $0x10, %%ax; \
	mov %%ax, %%ds; \
	mov %%ax, %%es; \
	mov %%ax, %%fs; \
	mov %%ax, %%gs; \
	mov %%rsp, %%rax; \
	push $0x10; \
	pushq %%rax; \
	pushf; \
	push $0x08; \
	pushq $flush_end; \
	iretq; \
	flush_end: \
	mov $0x2b, %%rax; \
	ltr %%ax;\
	" : : "m" (gdt_ptr));

}