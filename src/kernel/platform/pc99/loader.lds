SECTIONS
{
    .text : {
        _kickstart_begin = .;
        *(.multiboot_header)
        *(.text)
        *(.*data*)
        _data_end = .;
    }
    _bss_end = .;
    _end = .;
    _kickstart_end = .;
}
