/* This is x86 linker script. */
/* Ensure everything goes in the right order. */
ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    PAGE_SIZE = 4096;

    . =  0; /* This is relocatable executable, it will be relocated to appropriate position in memory. */
    .text ALIGN (4) :
    {
        *(.text*)
        *(.rodata*)
        /* global static initializers */
        . = ALIGN(4);
        ctors_GLOBAL = .;
        *(SORT(.ctor*))
        QUAD(0)
        dtors_GLOBAL = .;
        *(SORT(.dtor*))
        QUAD(0)
        FILL(0x90)
    }

    .data ALIGN (4) :
    {
        *(.data*)
        FILL(0x00)
    }

    .bss ALIGN (4) :
    {
        *(COMMON*)
        *(.bss*)
    }

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame) } /* FIXME: eh_frame is needed for dwarf debug info! */
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/
