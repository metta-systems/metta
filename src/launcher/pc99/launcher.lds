OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

START_ADDRESS = 0x100000;

/* setting up the Multiboot header - see GRUB docs for details. */
MULTIBOOT_MAGIC = 0x1badb002;        /* 'magic number' lets bootloader find the header */
MULTIBOOT_MEMORY_INFO  = 2;          /* provide memory map */
MULTIBOOT_FLAGS = MULTIBOOT_MEMORY_INFO;
MULTIBOOT_CHECKSUM = -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS);

SECTIONS {
    . =  START_ADDRESS;

    .multiboot ALIGN(4) : {
        /* multiboot header should fit in the first 8KiB */
        LONG(MULTIBOOT_MAGIC);
        LONG(MULTIBOOT_FLAGS);
        LONG(MULTIBOOT_CHECKSUM);
    }

    .text ALIGN (4) : {
        *(.text*)
        *(.rodata*)
        /* global static initializers */
        . = ALIGN(4);
        ctors_GLOBAL = .;
        *(.ctor*)
        QUAD(0)
        FILL(0x90)
    }

    .data ALIGN (4) : {
        *(.data*)
        FILL(0x00)
    }

    .bss ALIGN (4) : {
        *(COMMON*)
        *(.bss*)
    }

    /* Strip unnecessary stuff */
    /DISCARD/ : {
        *(.comment .note* .eh_frame .dtors) /* FIXME: eh_frame is needed for dwarf debug info! */
    }
}

/*
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
*/
