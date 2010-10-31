/* This is x86 linker script. */
/* Ensure everything goes in the right order. */
ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

/*!
 * FIXME: This is incoherent, all processes are relocated to different start addresses in reality!
 * 
Components are linked with classical layout:

         4GB +-------------------------+ MEM_SPACE_END (Usually 4GB)
             |                         | 0xFFFFFFFF (end of virtual mem)
             |    Page directory       | Pagedir mapped into every process image so kernel doesn't have to reload caches.
             |                         |
             |_________________________| K_PAGEDIR_START (4Mb below virtual memory end) == 0xFFC00000
             |                         |
             |    ................     |
             |                         | 0xC0001000 kernel vsyscall page (drifting, linked from interface page)
         3GB |_________________________| 0xC0000000 kernel info/interface page (L4-like)
             |                         | U_PROC_END (Usually 3GB) end of process space
             | Guarded userspace stacks| U_STACK_START
             |    (n pages)            |
             |_________________________| U_STACK_END
             | Libraries mapped from   | U_LIBS_START
             | here downwards.         |
             |                         |
             |_________________________| Unmapped heap watermark page at heap end for growing
             | Heap guard page         | process heap a-la brk()/sbrk() with system oom checks.
             |_________________________|
             |                         | U_HEAP_END
             | User heap here          |
             | growing upwards.        |
             |_________________________| U_HEAP_START
             | Code, data, bss         |
         1MB |_________________________| 1 MB 0x100000 U_CODE_START start of code in process image
             |                         |
             | Usually unmapped space  |
             | or used by device       |
             | drivers for mapping DMA |
             | and I/O resources in    |
             | low memory (1-1 phys    |
             | mapping?), VM86 area.   |
             |_________________________|
             | Unmapped 4K page.       |            (first page reserved)
             | may be mapped for VM86  |
             | tasks to access BDA     |
         0x0 +_________________________+ 0x00000000 (invalid address)

*/
SECTIONS
{
    PAGE_SIZE = 4K;

    . =  0; /* This is relocatable executable, it will be relocated to appropriate position in memory. */
    .text ALIGN (4) :
    {
        _kickstart_begin = .; /* FIXME: all this begin-end symbols stuff is fragile. */
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

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note* .eh_frame) } /* FIXME: eh_frame is needed for dwarf debug info! */
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/
