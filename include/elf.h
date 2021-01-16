/* 
 * evan-os/include/elf.h
 * 
 * Defines structures and functions for reading Executable Linker Files (ELF)
 * 
 */

#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ELF_ARCHITECTURE_X86_64 0x3e

// The header at the begining of every ELF file containing platform information and pointers to / the size of segment tables
typedef struct elf_header_t {

    char     magic_number[4]; // Must contain 0x7f and 'ELF' in ASCII
    uint8_t  bit_number;      // A value of 2 means 64 bit. Don't load 32 bit
    uint8_t  endian;          // 1 = Little endian, 2 = Big endian
    uint8_t  header_version;
    uint8_t  abi;             // 0 for System V
    
    uint8_t  unused[7];
    
    uint16_t type;         // 1 = relocatable, 2 = executable, 3 = shared, 4 = core(?)
    uint16_t architecture; // Must be x86_64. The OS does not run on other platforms
    uint32_t elf_version;  // 1 for the original version of ELF

    uint64_t program_entry_offset;  // Points to the code entry point
    uint64_t prgoram_header_offset; // Points to the start of the prograsm header table
    uint64_t section_header_offset; // Points to the start of the section header array

    uint32_t flags;                     // Platform Specific
    uint16_t header_size;               // The size of this header. Should be 64 bytes
    uint16_t program_header_entry_size; // The size of a program header
    uint16_t program_header_entries;    // The number of entries in the program header table
    uint16_t section_header_entry_size; // The size of a section header
    uint16_t section_header_entries;    // The number of entries in the section header table
    uint16_t section_names_index;       // The index of the section containing the section names

} __attribute__((packed)) elf_header_t;

// The layout of entries in ELF file's program headers table. Describes how the system should set up memory for the process
typedef struct elf_program_header_t {

    uint32_t type;  // The segment's type
    uint32_t flags; // Segment type dependant flags

    uint64_t offset_in_file;   // The segments location in the file
    uint64_t virtual_address;  // Where the segment should be in the virtual address space
    uint64_t physcial_address; // Unused by this OS. On platforms where it matters, this is the physical location in memory 
    uint64_t size_in_file;     // How large the segment is in the file
    uint64_t size_in_memory;   // Size of the segment in memory
    uint64_t alignment;        // A power of 2 representing the segment's memory alignment. 0 or 1 Indicates no requested alignment.

} __attribute__((packed)) elf_program_header_t;

typedef struct elf_section_header_t {

} __attribute__((packed)) elf_section_header_t;


// Checks if a file in loaded in memory has the 4 byte signature of an ELF file
// A return value of 1 indicates that the file is an elf file
uint8_t elf_check_signature(void* file_start);

#endif // ELF_H
