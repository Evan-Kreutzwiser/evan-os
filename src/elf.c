/* 
 * evan-os/src/elf.c
 * 
 * Functions for reading and loading Executable Linker Files (ELF)
 * 
 */

#include <elf.h>

#include <stdint.h>

uint32_t elf_signature = 0x464c457f; // 4 Bytes, 0x7f + ELF in ascii as an integer

// Checks if a file in loaded in memory has the 4 byte signature of an ELF file
// A return value of 1 indicates that the file is an elf file  
uint8_t elf_check_signature(void* file_start) {
    // Compare the first 4 bytes of the file with the elf signature
    if ( ((uint32_t*)file_start)[0] == elf_signature) {
        return 1; // If the file is an ELF file
    }
    // If the signature did not match
    return 0; 
}
