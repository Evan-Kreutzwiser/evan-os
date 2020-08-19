# evan-os
Evan OS is my custom operating system project writen in c for x86_64 uefi systems.

Roadmap and features are listed on the [trello page](https://trello.com/b/62egCB1G/evan-os-devlopment-board) 

## Building And Testing

Building Evan OS requires an x86_64-elf compiler. On x86_64 linux computers, you can use the regular gcc compiler, but on othere architectures a cross compiler is needed. 

Evan OS boots using the bootboot protocol, but due to build problems the files are provided precompiled. Testing with qemu requires the OVMF EFI.

Running `make` will compile the kernel, `make install` will install the OS onto an gpt disk image with an efi partition, and `make emu` starts qemu with the disk image containing the OS. The font can be replaced with another psf font to change the OS's tty font, and `make INITRD` will create the tarball without installing it to the img file (Useful for installing Evan OS on real hardware).

## Booting And Installation On Real Hardware

Evan OS's bootloader is a reference implementation of the [BOOTBOOT](https://gitlab.com/bztsrc/bootboot) protocol; a multiplatform bootloader that provides all operating systems the same startup environment. The Evan OS source code comes with a precompiled BOOTBOOT refernece implementation for the x86_64 efi platform. 

The kernel binary resides on the efi partition inside of the INITRD tar file, and the rest of the operating system is installed elsewhere. The INITRD tar also contains the drivers needed to boot up the operating system, such as disk drivers, filesystem drivers, and low level hardware drivers needed like timers needed to operate the computer.

To use Evan OS on real hardware, create a folder named "BOOTBOOT" in the root of your computer's EFI partition, place the CONFIG file and INITRD inside that new folder, and copy "BOOTX64.EFI" into EFI/BOOT in the EFI partition. If your uefi bios lets you set boot paths, you can change the name of "BOOTX64.EFI" to whatever you want before placing it in the EFI partition.
WARNING: copying BOOTX64.EFI into EFI/BOOT may overwrite your exsisting OS's bootloader and prevent you from using your exsisting OS. Only do this if you know exactly what you are doing.
