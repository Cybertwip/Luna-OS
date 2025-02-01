/* Entry point of the kernel */
ENTRY(_start)


/* Define the memory layout */
SECTIONS
{
    /* Start at 2M to avoid conflicts with BIOS and UEFI */
    . = 2M;

    .note.Xen : {
        *(.note.Xen)
    }

	.text BLOCK(4K) : ALIGN(4K)
	{
		code = .; _code = .; __code = .;
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .; _data = .; __data = .;
		*(.data)
	}

    .bss BLOCK(4K) : ALIGN(4K)
    {
		bss = .; _bss = .; __bss = .;

        *(COMMON)          /* Common symbols */
        *(.bss)            /* Uninitialized data */
    }

    /* Define symbols for the end of the kernel image */
    . = ALIGN(4K);
	end = .; _end = .; __end = .;
}