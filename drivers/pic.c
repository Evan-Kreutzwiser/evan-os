
#include <module.h>
#include <asm.h>

#include <stdint.h>

__attribute__((section("driverdata")))
driver_info_t driverdata = {
	.name = "Programable Interrupt Controler",
	.type = DEV_PIC,
	.subtype = DEV_PIC_PIC,
	.connection_type = DEV_CONNECTION_MOTHERBOARD,
	.init = (void*)&init,
	.connection_data.pic.end_of_int = (void*)&end_int,
	.connection_data.pic.end_of_int = (void*)&mask,
	.connection_data.pic.end_of_int = (void*)&unmask
};

// Interrupt offsets
uint8_t offset1, offset2;

uint8_t init(void) {

	/*
	 * Port 0x20 and 0x21 are the first PIC's command and data ports.
	 * Ports 0xa0 and 0xa1 are he command and dat ports for the second PIC.
	 */

	// Set offsets
	offset1 = 32;
	offset2 = 40;

	// Reinitialize the PIC chips (in cascade mode))
	outb(0x20, 0x11);
	outb(0xa0, 0x11);
	// Set the interrupt offsets
	outb(0x21, offset1);
	outb(0xa1, offset2);
	// Inform the chips about how they are wired to each other
	outb(0x21, 4); // Tell PIC 1 that theres a second on line 2 (0000 0100)
	outb(0xa1, 2); // Tell PIC 2 its cascade identity (0000 0010)
	// Set the PIC's mode
	outb(0x21, 0x1);
	outb(0xa1, 0x1);

	unmask(offset1+2); // Enable the second chip by unmasking the connecting interrupt line

	return 0;
}

void mask(uint8_t index) {

	uint8_t mask1, mask2, bit_to_change;

	// Read the current masks
	mask1 = inportb(0x21);
	mask2 = inportb(0xa1);

	// Check which chip manages the interrupt
	if (index >= offset2) {
		bit_to_change = index - offset2;
		// Make the new mask
		mask2 |= (0x1 << bit_to_change);
		// Use the new mask with PIC 2
		outportb(0xa1, mask2);
	}
	else {	
		bit_to_change = index - offset1;
		// Make the new mask
		mask1 |= (0x1 << bit_to_change);
		// Use the new mask with PIC 1
		outportb(0x21, mask1);
	}
}

void unmask(uint8_t index) {

	uint8_t mask1, mask2, bit_to_change;

	// Read the current masks
	mask1 = inportb(0x21);
	mask2 = inportb(0xa1);

	// Check which chip manages the interrupt
	if (index >= offset2) {
		bit_to_change = index - offset2;
		// Make the new mask
		mask2 &= ~(0x1 << bit_to_change);
		// Use the new mask with PIC 2
		outportb(0xa1, mask2);
	}
	else {	
		bit_to_change = index - offset1;
		// Make the new mask
		mask1 &= ~(0x1 << bit_to_change);
		// Use the new mask with PIC 1
		outportb(0x21, mask1);
	}
}

void end_int(uint8_t index) {
	if (index >= offset2) {
		outportb(0xa0, 0x20); // Send an EOI to pic 2
	}
	outportb(0x20, 0x20); // Send an EOI to pic 1
}

