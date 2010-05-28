SECTIONS
{
    PAGE_SIZE = 0x1000;

    . =  0; /* This is relocatable executable, it will be relocated to appropriate position in memory. */
    .text ALIGN (4) :
    {
        *(.text*)
        *(.rodata*)
        /* global static initializers */
        . = ALIGN(4);
        ctors_GLOBAL = .;
        *(.ctor*)
        QUAD(0)
        FILL(0x90)
    }
    text_end = .;

    .data ALIGN (4) :
    {
        *(.data*)
        FILL(0x00)
    }
    data_end = .;

    .bss ALIGN (4) :
    {
        *(COMMON*)
        *(.bss*)
    }
    bss_end = .;

    placement_address = ALIGN(PAGE_SIZE);

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) } /* FIXME: eh_frame is needed for dwarf debug info! */
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/