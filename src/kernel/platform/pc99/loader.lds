SECTIONS
{
    . =  0x100000;
    .text ALIGN(4) : {
        _kickstart_begin = .;
        *(.multiboot_info)
        *(.text)
        *(.*data*)
        _data_end = .;
    }
    _bss_end = .;
    _end = .;
    _kickstart_end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame .dtors) }
}
