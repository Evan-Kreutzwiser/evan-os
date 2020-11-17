/*
 * evanos/drivers/driver-linker.ld
 * 
 * The linker file used to compile Evan OS drivers
 *
 */


SECTIONS 
{

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.driverinfo)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

}
