
// Bootboot header
#include <bootboot.h>

// File header
#include <kernel.h>

// OS headers
#include <module.h> // Driver system
#include <asm.h> // Assembly single-instruction functions
#include <interrupt.h> // Hardware interrupts and exceptions
#include <gdt.h>
#include <tty.h>
#include <serial.h> // Serial port output

// Std headers
#include <stdint.h>

// Virtual addresses from linker script
extern BOOTBOOT bootboot;           // See ../dist/bootboot.h
extern unsigned char *environment;  // Configuration writen in key=value pairs
extern volatile uint8_t fb;         // Linear framebuffer mapped here
extern volatile unsigned char _binary_font_psf_start; // Font file

typedef struct tar_header_t {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12]; // In octal
    char mtime[12];
    char chksum[8];
    char typeflag[1];
} __attribute__((packed)) tar_header_t;

// Entry point
// NOTE: This code runs on all cores in parallel 
void _start(void) {

    uint32_t screen_width = bootboot.fb_width;
    uint32_t screen_height = bootboot.fb_height;
    uint32_t scanline = bootboot.fb_scanline;

    // Make a pointer the the screen's framebuffer
    volatile uint32_t* framebuffer = (uint32_t*)&fb;

    // Get the pointer to the ramdisk
    volatile void * initrd = (void *)bootboot.initrd_ptr;

    // Draw a rainbow box
    for (uint32_t xpos = 0; xpos < 256; xpos++) {
        for (uint32_t ypos = 0; ypos < 256; ypos++) {
            framebuffer[(scanline/4)*(ypos+(screen_height/2) - 128) + (xpos+(screen_width/2) - 128)] = ((255-xpos) << 16)+((255-ypos) << 8) + (xpos/2)+ (ypos/2);
        }
    }

    // Draw a rainbow box
    for (uint32_t xpos = 0; xpos < 256; xpos++) {
        for (uint32_t ypos = 0; ypos < 256; ypos++) {
            framebuffer[(scanline/4)*(ypos+(screen_height/2) - 128) + (xpos+(screen_width/2) - 128 - 256)] = ((xpos) << 16)+((255-ypos) << 8) + (128-xpos/2)+ (ypos/2);
        }
    }
    // Draw a rainbow box
    for (uint32_t xpos = 0; xpos < 256; xpos++) {
        for (uint32_t ypos = 0; ypos < 256; ypos++) {
            framebuffer[(scanline/4)*(ypos+(screen_height/2) - 128) + (xpos+(screen_width/2) + 128)] = ((xpos) << 16)+((255-ypos) << 8) + (128-xpos/2)+ (ypos/2);
        }
    }

    // Output text to show its working
    puts_at_pos("Welcome to Evan OS", (screen_width/2) - 9*8, screen_height/2 - 144);

    // Output text to show its working
    puts_at_pos_transparent("Loading", (screen_width/2) - 28, screen_height/2 - 8);

    // Display system information
    char text[27] = "The computer has   core(s)\n";
    text[17] = bootboot.numcores + '0';
    tty_print_string(text);

    // Load our interrupt table so we can add exception handlers
    interrupt_load_table();

    tty_print_string("Loaded interrupt table\n");

    // Add exception handlers

    tty_print_string("Adding exception handlers\n");

    // Set up the double fault handler (The most important one)
    interrupt_set_gate(0x8, (uint64_t)&double_fault, INTERRUPT_EXCEPTION_TYPE | INTERRUPT_INTERRUPT_GATE);
    // Set up the general protection fault handler
    interrupt_set_gate(0xd, (uint64_t)&gp_fault, INTERRUPT_EXCEPTION_TYPE | INTERRUPT_INTERRUPT_GATE);
    // Set up the div by 0 fault handler
    interrupt_set_gate(0x0, (uint64_t)&div_0_fault, INTERRUPT_EXCEPTION_TYPE | INTERRUPT_INTERRUPT_GATE);
    
    // Load drivers from disk as needed
    tty_print_string("Loading drivers\n");

    tar_header_t* file = (tar_header_t*)initrd;

    tty_print_string("Found file [");
    tty_print_string((char*)&file->filename[0]);

    tty_print_string("]\n");

    tty_print_string("Testing exception handlers\n");

    volatile int i = 0 / 0;
    MMapEnt* mmap_ent = &bootboot.mmap; 
    mmap_ent++;

    // Loop to prevent the kernel from returning to nothing and crashing
    // The OS should run tasks instead of this
    while(1);
}

__attribute__ ((interrupt))
void double_fault(struct interrupt_frame *frame, uint64_t error_code) {

    tty_print_string("UNKOWN CRITICAL ERROR. HALTING KERNEL.\n");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

__attribute__ ((interrupt))
void gp_fault(struct interrupt_frame *frame, uint64_t error_code) {

    tty_print_string("CRITICAL PROTECTION ERROR. HALTING KERNEL.");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

__attribute__ ((interrupt))
void div_0_fault(struct interrupt_frame *frame) {

    tty_print_string("DIV BY 0 ERROR. HALTING KERNEL.");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}


