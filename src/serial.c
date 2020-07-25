
#include <serial.h>

#include <asm.h>

#include <stdint.h>

#define PORT 0x3f8 // COM1 serial port base address

 void serial_init(void) {
   outportb(PORT + 1, 0x00);    // Disable all interrupts
   outportb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outportb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outportb(PORT + 1, 0x00);    //                  (hi byte)
   outportb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outportb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outportb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

// Receiving data

uint8_t serial_received(void) {
   return inportb(PORT + 5) & 1;
}
 
char serial_read(void) {
   while (serial_received() == 0);
 
   return inportb(PORT);
}

// Sending data

uint8_t is_transmit_empty(void) {
   return inportb(PORT + 5) & 0x20;
}
 
void serial_write(char c) {
   while (is_transmit_empty() == 0);
 
   outportb(PORT,c);
}

void serial_write_string(const char* c, uint32_t size) {

	for (uint32_t i = 0; i < size; i++) {
		serial_write(c[i]); // Send the character
	}

}