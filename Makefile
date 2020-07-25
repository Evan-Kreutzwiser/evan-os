
# Must be set to x86_64-elf tools
CC := gcc
LD := ld

CFLAGS := -Wall -Wextra -m64 -fpic -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -Iinclude -O0
LDFLAGS := -nostdlib -nostartfiles -T linker.ld

EMUFLAGS := -L /usr/share/edk2-ovmf/x64 -bios OVMF.fd \
 -net none \
 -drive id=disk,file=cdimage.iso,if=none,format=raw \
 -device ahci,id=ahci \
 -device ide-hd,drive=disk,bus=ahci.0 \
 -serial stdio \


KERNEL := kernel.sys

SRCDIR := ./src 
BINDIR := ./bin

OBJS := \
	bin/kernel.o \
	bin/asm.o \
	bin/interrupt.o \
	bin/tty.o \
	bin/serial.o \
	

all: $(KERNEL)

%.o: ../src/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	$(LD) -r -b binary -o $(BINDIR)/font.o font.psf
	$(LD) $(LDFLAGS) $(OBJS) $(BINDIR)/font.o -o $(KERNEL)
	
INITRD: $(KERNEL)
	@tar -cvf INITRD $(KERNEL)
	@echo Created bootbootinitrd
	
clean:
	-rm *.o bin/*.o src/*.o kernel.sys INITRD fat.img cdimage.iso
	
install: INITRD
	@echo Installation to img file not implemented yet 

	dd if=/dev/zero of=fat.img bs=1k count=1440
	mformat -i fat.img -f 1440 ::
	mmd -i fat.img ::/EFI
	mmd -i fat.img ::/EFI/BOOT
	mmd -i fat.img ::/BOOTBOOT
	mcopy -i fat.img INITRD ::/BOOTBOOT
	mcopy -i fat.img CONFIG ::/BOOTBOOT
	mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT

	-mkdir iso
	cp fat.img iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso

	@echo Done

loop:
	@echo Setting up loop devices for the image file
	sudo modprobe loop
	sudo losetup -o 1048576 --sizelimit 53476864 /dev/loop1 uefi.img
	sudo losetup -o 53476864 --sizelimit 104840192 /dev/loop2 uefi.img
	
emu:
	qemu-system-x86_64 $(EMUFLAGS)