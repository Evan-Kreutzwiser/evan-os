/* 
 * evan-os/src/elf.c
 * 
 * Functions for reading and loading Executable Linker Files (ELF)
 * 
 */

#include <elf.h>

#include <memory/paging.h>

#include <stdint.h>

uint32_t elf_signature = 0x464c457f; // 4 Bytes, 0x7f + ELF in ascii as an integer

// Checks if a file in loaded in memory has the 4 byte signature of an ELF file
// A return value of 1 indicates that the file is an elf file  
uint8_t elf_check_signature(elf_header_t* file_header) {
    // Compare the first 4 bytes of the file with the elf signature
    if ( ((uint32_t*)file_header)[0] == elf_signature) {
        return 1; // If the file is an ELF file
    }
    // If the signature did not match
    return 0; 
}


uint8_t elf_check_platform_info(elf_header_t* file_header) {

    // If the target architecture or endian direction doesnt match, throw an error
    // The file must be for x86_64 (AMD64), be 64 bit, and little endian
    if (file_header->architecture != ELF_ARCHITECTURE_X86_64 || file_header->endian != 1 || file_header->bit_number != 2) {
        return 1; // If the platform described in the file doesnt match the OS, then error
    }    

    // The file should work on this platform
    return 0;
}

uint8_t elf_allocate_memory(elf_header_t* file_header, uint64_t* address_space) {

    // Get the segment table using the offset and the header location
    elf_program_header_t* program_header_table = (elf_program_header_t*)(file_header->prgoram_header_offset+(uint64_t)file_header);
    uint16_t segment_count = file_header->program_header_entries;

    uint8_t page_flags = 0b111; // User, Writeable, and Present
    // TODO: This method offers not security at all

    for (uint16_t i = 0; i < segment_count; i++) {
        // TODO: Add address checking
        // Map the memory pages to the process's address space
        paging_allocate_user_pages((void*)program_header_table[i].virtual_address, program_header_table[i].size_in_memory / 4096, page_flags, address_space);
    }


    return 0;
}
