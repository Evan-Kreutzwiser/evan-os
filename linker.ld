/*
 * evanos/linker.ld
 *
 * Linker file obtained from the bootboot sample kernel
 * 
 * Copyright (C) 2017 - 2020 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 *
 */

mmio  = 0xffffffff80000000;                    /* Configurable for level 2 loaders like this one */
fb    = 0xffffffffc0000000;
PHDRS
{
  boot PT_LOAD;                                /* One single loadable segment */
}
SECTIONS
{
    . = 0xffffffffe0000000;                    /* Kernel base address in upper memory */
    bootboot    = .; . += 4096;
    environment = .; . += 4096;
    .text : {
        KEEP(*(.text.boot)) *(.text .text.*)   /* Code */
        *(.rodata .rodata.*)                   /* Data */
        *(.data .data.*)
    } :boot
    .bss (NOLOAD) : {                          /* Bss */
        . = ALIGN(16);
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4096);
        heap_location = .;
    } :boot

    /DISCARD/ : { *(.eh_frame) *(.comment) }
}
