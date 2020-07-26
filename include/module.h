


#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

// Device types and subtypes
#define DEV_PICTURE				0x1
	#define DEV_PICTURE_PICTURE	0x1
	#define DEV_PICTURE_VGA		0x2
#define DEV_IO					0x2
	#define DEV_IO_CONTROL		0x1
#define DEV_CLOCK				0x4
	#define DEV_CLOCK_CLOCK		0x1
	#define DEV_CLOCK_TIMER		0x2
#define DEV_PIC					0x5
	#define DEV_PIC_PIC			0x1
	#define DEV_PIC_APIC		0x2
#define DEV_PS2					0x6
	#define DEV_PS2_KEYBOARD	0x1
	#define DEV_PS2_MOUSE		0x2

// Device connection type
#define DEV_CONNECTION_DEV			0x0
#define DEV_CONNECTION_MOTHERBOARD	0x1 // Generic board devices
#define DEV_CONNECTION_PCI			0x2 // PCI devices


typedef struct driver_info_t {
	char* name;
	uint16_t type;
	uint16_t connection_type;
	void* info;
} driver_info_t;

typedef struct drive_info_pci_t {

	
} drive_info_pci_t;

uint8_t module_call_function(char* function_name);

uint8_t module_send_value(char* module, uint64_t value);

uint8_t module_load(driver_info_t* driver);

#endif // MODULE_H
