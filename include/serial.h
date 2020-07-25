
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define PORT 0x3f8   /* COM1 */
 
void serial_init(void);

// Receiving data

uint8_t serial_received(void); 
char serial_read(void);

// Sending data

uint8_t is_transmit_empty(void);
void serial_write(char c);
void serial_write_string(const char* c, uint32_t size);

#endif