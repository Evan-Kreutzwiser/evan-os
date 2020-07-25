/*
 * evanos/src/asm.c
 * 
 * Defines functions that represent single assembly commands used by the OS and drivers
 * 
 */

#ifndef ASM_H
#define ASM_H

#include <stdint.h>

// Interupts

void sti(void); // Enable interrupts
void cli(void); // Disable interrupts

void hlt(void); // Stop the computer

// Output to io ports

void outportb(uint16_t port, uint8_t data);
void outportw(uint16_t port, uint16_t data);
void outportl(uint16_t port, uint32_t data);

// Input through io ports

uint8_t inportb(uint16_t port);
uint16_t inportw(uint16_t port);
uint32_t inportl(uint16_t port);

#endif