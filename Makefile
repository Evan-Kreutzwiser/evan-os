
CC := x86_64-elf-gcc
LD := x86_64-elf-ld

CFLAGS := -ffreestanding -Wall -Wextra -Iinclude 
LDFLAGS := -nostdlibs -T linker.ld

KERNEL := kernel.sys


all: INITRD


%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL)

INITRD: $(KERNEL)
	@tar -cvf INITRD $(KERNEL)
	@echo Created bootbootinitrd
	
install: INITRD
	@echo Installation to img file not implemented yet 
