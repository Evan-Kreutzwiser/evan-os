
# Must be set to x86_64-elf tools
CC := gcc
LD := ld

CFLAGS := -Wall -Wextra -Wpointer-arith -Wcast-align \
	-m64 -fpic -ffreestanding -fno-stack-protector -nostdlib -mno-red-zone \
	-Iinclude -O0 -mno-sse -mno-mmx -mno-80387
LDFLAGS := -nostdlib -nostartfiles -T linker.ld

EMUFLAGS := -L /usr/share/edk2-ovmf/x64 -bios OVMF.fd \
 -net none \
 -drive id=disk,file=cdimage.iso,if=none,format=raw \
 -device ahci,id=ahci \
 -device ide-hd,drive=disk,bus=ahci.0 \
 -serial stdio \
 -smp 1 -m 1G -vga qxl \
 -d int -enable-kvm

 EMUFLAGDEBUG := -s -S

KERNEL := kernel.sys

SRCDIR := ./src
BINDIR := ./bin

SRCS = $(shell find $(SRCDIR)/ -type f -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(SRCS))

.PHONY: all install clean emu emudebug
.SUFFIXES: .o .c .img .iso .EFI

all: $(KERNEL)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	@mkdir -p "$(@D)"
	@$(CC) $(CFLAGS) -c $< -o $@

bin/font.o: font.psf
	@echo Converting font to obj file
	@$(LD) -r -b binary -o $(BINDIR)/font.o font.psf	

$(KERNEL): $(OBJS) bin/font.o
	@echo Linking kernel
	@$(LD) $(LDFLAGS) $(OBJS) $(BINDIR)/font.o -o $(KERNEL)
	@echo
	@echo Compilation complete
	@echo

INITRD: $(KERNEL)
	@echo Creating INITRD
	@tar -cvf INITRD $(KERNEL)
	@echo BOOTBOOT INITRD complete
	
clean:
	-rm -rf $(BINDIR) $(KERNEL) INITRD ./iso cdimage.iso
	
install: cdimage.iso

cdimage.iso: INITRD
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
	
	@echo Evan OS ISO complete

emu:
	qemu-system-x86_64 $(EMUFLAGS)

emudebug:
	qemu-system-x86_64 $(EMUFLAGS) $(EMUFLAGDEBUG)