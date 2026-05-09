ENTRY(start)

SECTIONS {
    . = 1M;

    .boot : {
        KEEP(*(.multiboot_header))
    }

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    . = ALIGN(4096);
    _kernel_end = .;

    /* Assert that the kernel never grows into the LSE load region */
    ASSERT(_kernel_end < 0x400000, "Kernel too large, overlaps LSE load address!")
}