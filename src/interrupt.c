/* 
 * evan-os/src/interrupt.c
 * 
 * Contains functions for managing cpu interrupts
 * 
 */

#include <interrupt.h>

#include <asm.h>

#include <stdint.h>
#include <stdbool.h>

bool using_apic;

#define PIC_COMMAND_PRIMARY		0x20		// IO address for primary PIC
#define PIC_COMMAND_SECONDARY	0xA0		// IO address for secondary PIC

#define PIC_DATA_PRIMARY		0x21		// IO address for primary PIC
#define PIC_DATA_SECONDARY		0xA1		// IO address for secondary PIC

// Create an interrupt stub called "stub_x" where x is the input number, 
// that calls the interrupt manager with an interrupt number
#define INTERRUPT_STUB(x) __attribute__((interrupt)) void x(__attribute__((unused)) struct interrupt_frame *frame) { interrupt_handler((uint64_t)x); }

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
volatile idt_entry_t idt[256];

volatile interrupt_handler_t interrupt_handlers[256];

INTERRUPT_STUB(stub_32)
INTERRUPT_STUB(stub_33)
INTERRUPT_STUB(stub_34)
INTERRUPT_STUB(stub_35)
INTERRUPT_STUB(stub_36)
INTERRUPT_STUB(stub_37)
INTERRUPT_STUB(stub_38)
INTERRUPT_STUB(stub_39)
INTERRUPT_STUB(stub_40)
INTERRUPT_STUB(stub_41)
INTERRUPT_STUB(stub_42)
INTERRUPT_STUB(stub_43)
INTERRUPT_STUB(stub_44)
INTERRUPT_STUB(stub_45)
INTERRUPT_STUB(stub_46)
INTERRUPT_STUB(stub_47)
INTERRUPT_STUB(stub_48)
INTERRUPT_STUB(stub_49)

void interrupt_init(void) {
	// Set up stubs for interrupts that can be claimed by drivers
	interrupt_set_gate(32, (uint64_t)&stub_32, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(33, (uint64_t)&stub_33, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(34, (uint64_t)&stub_34, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(35, (uint64_t)&stub_35, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(36, (uint64_t)&stub_36, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(37, (uint64_t)&stub_37, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(38, (uint64_t)&stub_38, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(39, (uint64_t)&stub_39, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	interrupt_set_gate(40, (uint64_t)&stub_40, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
	// TODO: Initialize the other 9 interrupt stubs
}

void interrupt_set_gate(uint8_t index, uint64_t address, uint8_t type_attributes) {

	// Check that the interrupt handler is not null
	if (address == 0) {
		// Dont set up the invalid interrupt
		return;
	}

	idt[index].selector = 0x08;
	idt[index].offset_low = (uint16_t)(address & 0xffff);
	idt[index].offset_mid = (uint16_t)((address & 0xffff0000) >> 16);
	idt[index].offset_high = (uint32_t)((address & 0xffffffff00000000) >> 32);
	idt[index].type_attr = type_attributes;
}

// Register a hardware interrupt with a manager for sending end-of-interrupt commands
void interrupt_register(uint8_t index, interrupt_handler_t handler) {

	// Dont register interrupts on the exception handler slots
	// Or overwrite the syscall handler.
	// And make sure the passed handler isnt null
	if (index < 32 || index == 0x80 || handler == 0) {
		return;
	}

	// Set the requested entry in the interrupt manager 
	interrupt_handlers[index] = handler;
}

// Remove a hardware interrupt
void interrupt_unregister(uint8_t index) {

	// Dont remove interrupts from the exception handler slots
	// Or overwrite the syscall handler
	if (index < 32 || index == 0x80) {
		return;
	}

	// Void the handler list entry
	interrupt_handlers[index] = 0;
}

void interrupt_handler(uint64_t interrupt_num) {

	// If the handler is null dont do anything
	if (interrupt_handlers[interrupt_num] == 0) {
		return;
	}

	// Call the interrupt handler
	interrupt_handlers[interrupt_num]();

	// If using the pic chips, send an eoi
	if (!using_apic) {
		interrupt_end_pic(interrupt_num);
	}
}

void interrupt_mask(uint8_t interrupt) {

	// If the OS is using the old PIC chip	
	if (!using_apic) {

		uint8_t primary_mask;
		uint8_t secondary_mask;

		// Get the current masks from the 2 PIC chips
		primary_mask = inportb(PIC_DATA_PRIMARY);
		secondary_mask = inportb(PIC_DATA_SECONDARY);

		if (interrupt < 8) {
			primary_mask |= 1 << interrupt;
			outportb(PIC_DATA_PRIMARY, primary_mask);
		}
		else {
			secondary_mask |= 1 << (interrupt-8);
			outportb(PIC_DATA_SECONDARY, secondary_mask);
		}

	}
}

void interrupt_unmask(uint8_t interrupt) {
	
	// If the OS is using the old PIC chip
	if (!using_apic) {

		uint8_t primary_mask;
		uint8_t secondary_mask;

		// Get the current masks from the 2 PIC chips
		primary_mask = inportb(PIC_DATA_PRIMARY);
		secondary_mask = inportb(PIC_DATA_SECONDARY);

		if (interrupt < 8) {
			primary_mask &= ~(1 << interrupt);
			outportb(PIC_DATA_PRIMARY, primary_mask);
		}
		else {
			secondary_mask &= ~(1 << (interrupt-8));
			outportb(PIC_DATA_SECONDARY, secondary_mask);
		}

	}
}

void interrupt_load_table(void) {

	// Create the pointer read by the lgdt instructtion
	volatile idt_ptr_t idt_pointer = {
		.limit = (sizeof(idt_entry_t) * 256) - 1, // 256 entries
		.base = (idt_entry_t*)&idt[0]							  // Address of the table
	};

	asm volatile("lidt (%0)" : : "r" (&idt_pointer));

}

/* Set which interrupt chip the OS will use (Should never be called after booting) */
void interrupt_set_mode(bool use_apic) {
	using_apic = use_apic;

	// If using the old PIC chips
	if (!use_apic) {
		// Port 0x20 and 0x21 are the first PIC's command and data ports.
		// Ports 0xa0 and 0xa1 are he command and dat ports for the second PIC.
		
		// Set offsets
		uint64_t offset1 = 32;
		uint64_t offset2 = 40;

		// Reinitialize the PIC chips (in cascade mode))
		outportb(PIC_COMMAND_PRIMARY, 0x11);
		outportb(PIC_COMMAND_SECONDARY, 0x11);
		// Set the interrupt offsets
		outportb(PIC_DATA_PRIMARY, offset1);
		outportb(PIC_DATA_SECONDARY, offset2);
		// Inform the chips about how they are wired to each other
		outportb(PIC_DATA_PRIMARY, 4); // Tell PIC 1 that theres a second on line 2 (0000 0100)
		outportb(PIC_DATA_SECONDARY, 2); // Tell PIC 2 its cascade identity (0000 0010)
		// Set the PIC's mode
		outportb(PIC_DATA_PRIMARY, 0x1);
		outportb(PIC_DATA_SECONDARY, 0x1);

		// Enable the second chip by unmasking the connecting interrupt line
		interrupt_unmask(offset1+2); 
	}
	else {
		// TODO: Implement APIC interrupt handling
	}
}

void interrupt_end_pic(uint8_t index) {
	if (index >= 40) {
		outportb(PIC_COMMAND_SECONDARY, 0x20); // Send an EOI to pic 2
	}
	outportb(PIC_COMMAND_PRIMARY, 0x20); // Send an EOI to pic 1
}