
# Must be set to x86_64-elf tools
CC := gcc
LD := ld

CFLAGS := -Wall -Wextra -m64 -fpic -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -Iinclude -O0
LDFLAGS := -nostdlib -nostartfiles -T linker.ld

EMUFLAGS := -L /usr/share/edk2-ovmf/x64 -bios OVMF.fd \
 -net none \
 -drive id=disk,file=uefi.img,if=none,format=raw \
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
	

all: $(KERNEL) INITRD bootboot.efi

%.o: ../src/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(KERNEL): $(OBJS)
	$(LD) -r -b binary -o $(BINDIR)/font.o font.psf
	$(LD) $(LDFLAGS) $(OBJS) $(BINDIR)/font.o -o $(KERNEL)
	readelf -hls $(KERNEL) >kernel-info.txt

INITRD: $(KERNEL)
	@tar -cvf INITRD $(KERNEL)
	@echo Created bootbootinitrd
	
clean:
	-rm INITRD
	-rm *.o bin/*.o src/*.o kernel.sys
	
install: INITRD
	@echo Installation to img file not implemented yet 
	@echo Mounting and installing OS to disk image
	sudo mount /dev/loop1 imgmnt
	# sudo mount /dev/loop2 fatmnt

	# -sudo mkdir -p efimnt/EFI
	@-sudo mkdir -p imgmnt/EFI/boot
	# -sudo mkdir -p efimnt/EFI/EvanOS
	@-sudo mkdir -p imgmnt/BOOTBOOT

	sudo cp bootboot.efi imgmnt/EFI/boot/BOOTX64.EFI
	sudo cp CONFIG imgmnt/BOOTBOOT/CONFIG
	@echo Bootloader files copied

	# sudo cp kernel.sys fatmnt/kernel2.sys
	# sudo cp kernel.sys efimnt/EFI/EvanOS/kernel2.sys
	sudo cp INITRD imgmnt/BOOTBOOT/INITRD
	
	sudo umount imgmnt
	# sudo umount fatmnt
	@echo Done

loop:
	@echo Setting up loop devices for the image file
	sudo modprobe loop
	sudo losetup -o 1048576 --sizelimit 53476864 /dev/loop1 uefi.img
	sudo losetup -o 53476864 --sizelimit 104840192 /dev/loop2 uefi.img
	
emu:
	qemu-system-x86_64 $(EMUFLAGS)