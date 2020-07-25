# evan-os
Evan OS is my custom operating system project writen in c for x86_64 uefi systems.

Roadmap and features are listed on the trello page (https://trello.com/b/62egCB1G/evan-os-devlopment-board) 

## Building And Testing

Building Evan OS requires an x86_64-elf compiler. On x86_64 linux computers, you can use the regular gcc compiler, but on othere architectures a cross compiler is needed. 

Running `make` will compile the kernel and link in the font file (font.psf) to create kernel.sys, and `make INITRD` creates the tarball that bootboot uses to boot the OS. `make install` will install the OS onto an gpt disk image with an efi partition. Finally, running `make emu` starts qemu with the disk image containing the OS.
