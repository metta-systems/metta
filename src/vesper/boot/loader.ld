/* Ensure everything goes in the right order. */
/* This is x86 linker script. */
ENTRY (loader)
OUTPUT_FORMAT(binary)

SECTIONS
{
    KERNEL_BASE = 0x100000;
    KERNEL_VIRTUAL_BASE = 0xc0000000;
    PAGE_SIZE = 0x1000;

    . = 0x100000;
    .text ALIGN (PAGE_SIZE) : AT(ADDR(.text) - KERNEL_BASE)
    {
        *(.loader.text)
        *(.text)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN (PAGE_SIZE) : AT(ADDR(.data) - KERNEL_BASE)
    {
        *(.data)
        *(.rodata*)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (PAGE_SIZE) :  AT(ADDR(.bss) - KERNEL_BASE)
    {
        *(COMMON*)
        *(.bss*)
    }
    bss_end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame .ctors .dtors) }
}
