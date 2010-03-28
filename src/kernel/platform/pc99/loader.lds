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
