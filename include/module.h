


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
#define DEV_PS2					0x6 // PS2 devices
	#define DEV_PS2_KEYBOARD	0x1
	#define DEV_PS2_MOUSE		0x2
#define DEV_IDE					0x7 // IDE drives
	#define DEV_IDE_HDD			0x1 // IDE hard drive
	#define DEV_IDE_CD			0x2 // IDE disk drive

// Device connection type
#define DEV_CONNECTION_DEV			0x0
#define DEV_CONNECTION_MOTHERBOARD	0x1 // Generic board devices
#define DEV_CONNECTION_PCI			0x2 // PCI devices
#define DEV_CONNECTION_USB			0x3 // USB devices


typedef struct driver_info_t {
	char* name; // The divice's human readable name
	uint16_t type; // What type of device it is
	uint16_t subtype;
	uint16_t connection_type; // How the device is connected
	void * init; // Pointer to the device initialization function
	union {
		struct {
			uint16_t vendor;
			uint16_t device;

		} pci; 
		struct {
			void * end_of_int; // Pass a uint8_t holding the interrupt's index to all 3
			void * mask_int;
			void * unmsak_int; 
		} pic;
		

	} connection_data;
} driver_info_t;

uint8_t module_call_function(char* function_name);

uint8_t module_send_value(char* module, uint64_t value);

uint8_t module_load(driver_info_t* driver);

#endif // MODULE_H
