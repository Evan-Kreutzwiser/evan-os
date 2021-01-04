/*
 * evan-os/src/asm.h
 * 
 * Defines functions that represent single assembly commands used by the OS and drivers
 * 
 */

#ifndef ASM_H
#define ASM_H

#include <stdint.h>

// Interrupts

void sti(void); // Enable interrupts
void cli(void); // Disable interrupts

void hlt(void); // Stop the computer

// Model Specific Register manupulation

void     wrmsr(uint32_t msr_id, uint32_t low, uint32_t high);
uint64_t rdmsr(uint32_t msr_id);
uint32_t rdmsr_low(uint32_t msr_id);
uint32_t rdmsr_high(uint32_t msr_id);

// Output to io ports

void outportb(uint16_t port, uint8_t data);
void outportw(uint16_t port, uint16_t data);
void outportl(uint16_t port, uint32_t data);

// Input through io ports

uint8_t inportb(uint16_t port);
uint16_t inportw(uint16_t port);
uint32_t inportl(uint16_t port);

#endif