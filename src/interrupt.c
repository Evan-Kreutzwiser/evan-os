
#include <interrupt.h>

#include <stdint.h>

typedef struct idt_entry_t {
   uint16_t offset_low; 	// Bits 0-15
   uint16_t selector; 		// GDT Code segment
   uint8_t  ist;       		// Bits 0-2 hold the Interrupt Stack Table offset, rest is zero.
   uint8_t  type_attr; 		// Type and attributes
   uint16_t offset_mid; 	// Bits 16-31
   uint32_t offset_high; 	// Bits 32-63
   uint32_t zero;     		// Reserved
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr_t {
	uint16_t limit; // The size of the table / highest byte in the table relative to the base
	idt_entry_t* base; // The address of the idt
} __attribute__((packed)) idt_ptr_t;


// The Interrupt Descriptor Table, made of 256 decsriptors
idt_entry_t idt[256];

void interrupt_set_gate(uint8_t index, uint64_t address, uint8_t type_attributes) {

	idt[index].selector = 0x38;
	idt[index].offset_low = (uint16_t)(address & 0xffff);
	idt[index].offset_mid = (uint16_t)((address & 0xffff0000) >> 16);
	idt[index].offset_high = (uint32_t)((address & 0xffffffff00000000) >> 32);
	idt[index].type_attr = type_attributes;
}

void interrupt_load_table(void) {

	// Create the pointer read by the lgdt instructtion
	idt_ptr_t idt_pointer = {
		.limit = (sizeof(idt_entry_t) * 256) - 1, // 256 entries
		.base = (idt_entry_t*)&idt[0]							  // Address of the table
	};

	asm volatile("lidt (%0)" : : "r" (&idt_pointer));

}
