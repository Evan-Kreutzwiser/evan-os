
# Must be set to x86_64-elf tools
CC := gcc
LD := ld

CFLAGS := -Wall -Wextra \
	-m64 -fpic -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone -Iinclude -O0
LDFLAGS := -nostdlib -nostartfiles -T linker.ld

EMUFLAGS := -L /usr/share/edk2-ovmf/x64 -bios OVMF.fd \
 -net none \
 -drive id=disk,file=cdimage.iso,if=none,format=raw \
 -device ahci,id=ahci \
 -device ide-hd,drive=disk,bus=ahci.0 \
 -serial stdio \
 -smp 2 \


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

$(BINDIR):
	-@mkdir -p $(BINDIR)

bin/%.o: src/%.c $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	$(LD) -r -b binary -o $(BINDIR)/font.o font.psf
	$(LD) $(LDFLAGS) $(OBJS) $(BINDIR)/font.o -o $(KERNEL)
	@echo Compilation complete

INITRD:
	@tar -cvf INITRD linker.ld $(KERNEL) font.psf
	@echo Created bootbootinitrd
	
clean:
	-rm -f *.o bin/*.o src/*.o kernel.sys INITRD *.img cdimage.iso
	
install: INITRD
	@echo Creating Evan OS disk image
	@dd if=/dev/zero of=fat.img bs=1k count=1440
	@mformat -i fat.img -f 1440 ::
	@mmd -i fat.img ::/EFI
	@mmd -i fat.img ::/EFI/BOOT
	@mmd -i fat.img ::/BOOTBOOT
	@mcopy -i fat.img INITRD ::/BOOTBOOT
	@mcopy -i fat.img CONFIG ::/BOOTBOOT
	@mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT

	-@mkdir -p iso
	@cp fat.img iso
	@xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso

emu:
	qemu-system-x86_64 $(EMUFLAGS)