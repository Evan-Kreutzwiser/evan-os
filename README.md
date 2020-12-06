# evan-os
Evan OS is my custom operating system project writen in c for x86_64 uefi systems.

![Build: Master](https://img.shields.io/github/workflow/status/Evan-Kreutzwiser/evan-os/Compile/master?label=Build%3A%20Master&style=flat-square)
![Build: Development](https://img.shields.io/github/workflow/status/Evan-Kreutzwiser/evan-os/Compile/development?label=Build%3A%20Development&style=flat-square)
![Open issues](https://img.shields.io/github/issues/Evan-Kreutzwiser/evan-os?label=Issues&style=flat-square)

Evan OS's bootloader is a reference implementation of the [BOOTBOOT](https://gitlab.com/bztsrc/bootboot) protocol; a multiplatform bootloader that provides all operating systems the same startup environment.

Roadmap and features are listed on the [trello page](https://trello.com/b/62egCB1G/evan-os-devlopment-board) 

## Building And Testing

Building Evan OS requires an x86_64-elf-gcc compiler and gnu make. On x86_64 linux computers, the built in gcc compiler can be used. Evan-OS is only designed for x86_64 systems. 

Evan OS boots using the bootboot protocol, but due to build problems the files are provided precompiled. Testing with qemu requires the OVMF EFI.

Make sure all dependancies are installed,  then build the OS:

1. Compile the kernel with make
``` 
make 
```

2. Create a bootable ISO
```
make install
```

3. Run the OS in qemu
```
make emu
```

The font can be replaced with another psf font to change the OS's tty font, and `make INITRD` will create the tarball without installing it to the img file (Useful for installing Evan OS on real hardware).

To debug Evan OS with gdb, use `make emudebug` to make qemu start paused and wait for a gdb connection.

## Dependencies

Evan-OS can only by built on x86_64 linux systems with gcc-10 or above and gnu make, and all dependencies can be install through the distribution package manager.

ISO creation with `make install`:

- mtools
- xorriso

Qemu emulation with `make emu`:

- qemu (For x86_64 with kvm)
- ovmf

## Booting And Installation On Real Hardware

The Evan OS source code comes with a precompiled BOOTBOOT refernece implementation for the x86_64 efi platform. 

The kernel binary resides on the efi partition inside of the INITRD tar file, and the rest of the operating system is installed elsewhere. The INITRD tar also contains the drivers needed to boot up the operating system, such as disk drivers, filesystem drivers, and low level hardware drivers like timers needed to operate the computer.

To use Evan OS on real hardware:

1. Create the INITRD (Also automatically made during ISO creation)
```
make INITRD
```

2. Create a folder called BOOTBOOT at the root of your EFI partition

3. Copy the INITRD and CONFIG files (./CONFIG and ./INITRD) into the BOOTBOOT folder

4. Copy BOOTX64.EFI into the EFI/BOOT folder of the EFI partition

WARNING: copying BOOTX64.EFI into EFI/BOOT may overwrite your exsisting OS's bootloader and prevent you from using your exsisting OS. 

Only do this if you know exactly what you are doing.
