# evan-os
My custom operating system project writen in c.

Roadmap and features are listed on the trello page (https://trello.com/b/62egCB1G/evan-os-devlopment-board) 

## Building And Testing

Building Evan OS requires an existing x86_64-elf cross compiler, as the build script will not make one for you.

Running `make` will compile the kernel and link in the font file (font.psf) to create kernel.sys, and `make INITRD` creates the tarball that bootboot uses to boot the OS. `make install` will install the OS onto an gpt disk image with an efi partition. Finally, running `make emu` starts qemu with the disk image containing the OS.
