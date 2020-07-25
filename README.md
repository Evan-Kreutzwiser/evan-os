# evan-os
Evan OS is my custom operating system project writen in c for x86_64 uefi systems.

Roadmap and features are listed on the trello page (https://trello.com/b/62egCB1G/evan-os-devlopment-board) 

## Building And Testing

Building Evan OS requires an x86_64-elf compiler. On x86_64 linux computers, you can use the regular gcc compiler, but on othere architectures a cross compiler is needed. 

Evan OS boots using the bootboot protocol, but due to build problems the files are provided precompiled. Testing with qemu requires the OVMF EFI.

Running `make` will compile the kernel, `make install` will install the OS onto an gpt disk image with an efi partition, and `make emu` starts qemu with the disk image containing the OS. The font can be replaced with another psf font to change the OS's tty font, and `make INITRD` will create the tarball without installing it to the img file.
