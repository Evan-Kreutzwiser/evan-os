
// Bootboot header
#include <bootboot.h>

// File header
#include <kernel.h>

// OS headers
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
extern uint8_t fb;                  // Linear framebuffer mapped here
extern volatile unsigned char _binary_font_psf_start; // Font file


// Entry point
// NOTE: This code runs on all cores in parallel 
void _start(void) {

    uint32_t screen_width = bootboot.fb_width;
    uint32_t screen_height = bootboot.fb_height;
    uint32_t scanline = bootboot.fb_scanline;

    // Make a pointer the the screen's framebuffer
    uint32_t* framebuffer = (uint32_t*)&fb;

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
    


    volatile int r = 3 / 0;

    // Loop to prevent the kernel from returning (to nothing)
    while(1);
}

void double_fault(void) {

    tty_print_string("CRITICAL ERROR. HALTING KERNEL.");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

void gp_fault(void) {

    tty_print_string("CRITICAL GP ERROR. HALTING KERNEL.");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

void div_0_fault(void) {

    tty_print_string("CRITICAL DIV BY 0 ERROR. HALTING KERNEL.");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}