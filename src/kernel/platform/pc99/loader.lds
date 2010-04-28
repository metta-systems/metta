SECTIONS
{
    . =  0x100000;
    .text ALIGN(4) : {
        _kickstart_begin = .;
        *(.multiboot_info)
        *(.text*)
        *(.*data*)
        /* global static initializers */
        . = ALIGN(4);
        ctors_GLOBAL = .;
        *(.ctor*)
        QUAD(0)
        _data_end = .;
        *(COMMON*)
        *(.bss*)
        _bss_end = .;
    }
    _end = .;
    _kickstart_end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) }
}

/*
TODO: use ld script to assemble and relink all the used modules to right addresses:
should be possible with a bit of LD voodoo.
*/

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/