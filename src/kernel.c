
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
#include <syscall.h>
#include <memory/paging.h>
#include <memory/memory.h>
#include <memory/rangeallocator.h>

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

char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Entry point
// NOTE: This code runs on all cores in parallel 
__attribute__((section(".text.boot")))
void _start(void) {

    // Load the memory segment table
    gdt_init();

    // Now the kernel can run in its properly set memory segments
    kernel();
}

// The main kernel function responsible for inirializing Evan OS
void kernel(void) {

    uint32_t screen_width = bootboot.fb_width;
    uint32_t screen_height = bootboot.fb_height;
    uint32_t scanline = bootboot.fb_scanline;

    // Make a pointer the the screen's framebuffer
    volatile uint32_t* framebuffer = (uint32_t*)&fb;

    // Get the pointer to the ramdisk
    volatile void * initrd = (void *)bootboot.initrd_ptr;

    // Disable interrupts
    cli();

    // Initialize the tty to take screen dimensions into account
    tty_init();

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

    tty_print_string("Loading interrupt table\n");

    // Load our interrupt table so we can add exception handlers
    interrupt_load_table();

    // Fill the entries for driver assignable interrupts
    interrupt_init();

    // Add exception handlers
    tty_print_string("Adding exception handlers\n");

    // Set up the double fault handler (The most important one)
    interrupt_set_gate(0x8, (uint64_t)&double_fault, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
    // Set up the general protection fault handler
    interrupt_set_gate(0xd, (uint64_t)&gp_fault, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
    // Set up the page fault handler
    interrupt_set_gate(0xe, (uint64_t)&page_fault, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
    // Set up the div by 0 fault handler
    interrupt_set_gate(0x0, (uint64_t)&div_0_fault, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);
    // Set up the invalid instruction fault handler
    interrupt_set_gate(0x6, (uint64_t)&invalid_opcode_fault, INTERRUPT_PRESENT | INTERRUPT_INTERRUPT_GATE);

    // Load drivers from disk as needed
    tty_print_string("Loading drivers\n");

    tar_header_t* file = (tar_header_t*)initrd;

    tty_print_string("Found file [");
    tty_print_string((char*)&file->filename[0]);
    tty_print_string("]\nFile Size: ");
    print_hex(octal_string_to_int(file->size, 11));
    tty_print_string("\nSetting up syscalls\n");

    // Set up system calls
    syscall_init();

    // Set up the memory paging system
    paging_init();

    // Set up basic memory allocation
    memory_allocation_init();

    // Switch paging to malloc mode
    //paging_enable_memory_allocation();

    // Create and load a blank test address space
    void * address_space_pointer = paging_create_address_space(); // Create the address space's pml4
    address_space_pointer = (void*)paging_get_physical_address((uint64_t)address_space_pointer); // Get its phyiscal address
    paging_load_address_space(address_space_pointer); // Load the new pml4

    // Test memory allocation

    // Allocate 2 chunks of memory
    uint8_t* pointer = kmalloc(2048);
    uint8_t* pointer2 = kmalloc(2048);
    // Free one
    kfree(pointer2);
    // Allocate a 3rd
    uint8_t* pointer3 = kmalloc(1024);
    
    // Check if the freed space was reused
    if ((uint64_t)pointer2 == (uint64_t)pointer3) {
        tty_print_string("Memory allocation test passed\n");
    } 
    else {
        tty_print_string("ERROR: Memory allocation test failed\n");
    }

    // Free the pointers to reuse the memory
    kfree(pointer);
    kfree(pointer3);

    // Stop the computer
    // The OS should run tasks instead of this
    while(1) {
        cli();
        hlt();
    }
}



__attribute__ ((interrupt))
void double_fault(struct interrupt_frame *frame /*, uint64_t error_code */ ) {

    tty_print_string("UNKOWN CRITICAL ERROR. HALTING KERNEL.\n");

    // Print the address that caused the fault
    tty_print_string("Address: ");
    print_hex(frame->ip);
    tty_print_char('\n');

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

__attribute__ ((interrupt))
void gp_fault(struct interrupt_frame *frame, uint64_t error_code) {

    tty_print_string("CRITICAL PROTECTION ERROR: \nTable:");

    switch (error_code & 0b110) {
        case 0b000:
            tty_print_string(" GDT\n");
            break;
        case 0b010:
        case 0b110:
            tty_print_string(" IDT\n");
            break;
        case 0b100:
            tty_print_string(" LDT\n");
            break;
    }

    uint64_t index = (error_code >> 3) & 0x1fff;
    tty_print_string("Index: ");
    print_hex(index);
    
    tty_print_string("\n%RIP: ");
    print_hex(frame->ip);
    tty_print_string("\n%CS: ");
    print_hex(frame->cs);

    tty_print_string("\nHALTING KERNEL.\n");

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

__attribute__((interrupt))
void page_fault(struct interrupt_frame *frame, uint64_t error_code) {

    uint64_t faulting_address;

    // Get the virtual address of the fault causing instruction
    asm volatile ("mov %%cr2, %%rax; mov %%rax, %0;" : "=m" (faulting_address) :: "rax");

    tty_print_string("PAGE FAULT. Details:\n");

    // Check bit 0 to see if the error was caused by a non present page or a protection violation
    if ((error_code & 0b1) == 1) {
        tty_print_string("Page protection violation\n");
    }
    else {
        tty_print_string("Page not present\n"); 
    }

    if ((error_code & 0b10) == 2) {
        tty_print_string("Write access\n");
    }
    else {
        tty_print_string("Read access\n"); 
    }

    // Check the priviledge level of the error
    if ((error_code & 0b100) == 4) {
        tty_print_string("User code\n");
    }
    else {
        tty_print_string("Kernel code\n"); 
    }

    if ((error_code & 0b1000) == 8) {
        tty_print_string("Reserved bits inccorectly set\n");
    }

    if ((error_code & 0b10000) == 16) {
        tty_print_string("Instruction fetch\n");
    }

    // Print the faulting virtual address
    tty_print_string("V Address: ");
    print_hex(faulting_address);
    tty_print_string("Faulting Address: ");
    print_hex(frame->ip);
    tty_print_string("\nCode Segment: ");
    print_hex(frame->cs);
    tty_print_char('\n');
}

__attribute__ ((interrupt))
void div_0_fault(struct interrupt_frame *frame) {

    tty_print_string("DIV BY 0 ERROR. HALTING KERNEL.");

    // Print the address that divided by 0
    tty_print_string("Address: ");
    print_hex(frame->ip);
    tty_print_char('\n');

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

__attribute__ ((interrupt))
void invalid_opcode_fault(struct interrupt_frame *frame) {

    tty_print_string("INVALID INSTRUCTION ERROR. HALTING KERNEL.\nAddress: ");

    print_hex(frame->ip);

    // Stop the computer
    while (1) {
        cli();
        hlt();
    }
}

void print_val(uint64_t value, uint8_t bits) {
    tty_print_string("0b");
    char character[bits+1];
    character[bits] = '\0';
    uint64_t shifted_value;
    for (int i = bits-1; i >= 0; i--) {
        shifted_value = value >> i;
        character[i] = (shifted_value & 0b1) + '0';
        tty_print_string(character);
    }
}

void print_hex(uint64_t value) {

    tty_print_string("0x");
    uint64_t shifted_value;
    for (int i = 15; i >= 0; i--) {
        shifted_value = (value >> (4 * i)) & 0xf;
        
        tty_print_char(hex_digits[shifted_value]);
    }
}

// Convert an octal string to a sinlge integer that the computer can use
uint64_t octal_string_to_int(char* octal_string, uint64_t length) {
    
    uint64_t decoded_value = 0;
    uint64_t multiplier = 1;

    // Iterate over every octal string digit, starting from the lowest place value
    for (int64_t i = length-1; i >= 0; i--, multiplier *= 8) {
        // Every itteration/digit the multiplier is multiplied by 8 
        // to account for the digit's place value

        // Add the digit multiplied by its place value to the total decoded value
        decoded_value += (octal_string[i]-'0') * multiplier;
    }

    return decoded_value;
}