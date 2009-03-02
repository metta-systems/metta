/* Ensure everything goes in the right order. */
/* This is x86 linker script. */
ENTRY (loader)

SECTIONS
{
    KERNEL_BASE = 0xc0000000;
    PAGE_SIZE = 0x1000;

    . = 0x100000;
    .setup : /* Mapped at 0x100000 */
    {
        *(.setup)
    }

    . += KERNEL_BASE;

    .text ALIGN (PAGE_SIZE) : AT(ADDR(.text) - KERNEL_BASE)
    {
        code = .;
        *(.loader.text)
        *(.text)
        FILL(0x90)
    }

    .data ALIGN (PAGE_SIZE) : AT(ADDR(.data) - KERNEL_BASE)
    {
        start_ctors = .;
        *(.ctors)  /* or *(SORT(.ctors)) */
        end_ctors = .;

        data = .;
        *(.data)
        *(.rodata*)
        FILL(0x00)
    }
    edata = .;

    .bss ALIGN (PAGE_SIZE) :  AT(ADDR(.bss) - KERNEL_BASE)
    {
        _sbss = .;
        *(COMMON*)
        *(.bss*)
        _ebss = .;
        . = ALIGN(PAGE_SIZE);
    }

    end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame .dtors) }
}
