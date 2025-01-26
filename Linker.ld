ENTRY(_start)

SECTIONS {
    . = 0x100000;               /* Load at 1MB */

    .note.Xen : {
        *(.note.Xen)
    }

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ : {
        *(.comment)
    }
    
}