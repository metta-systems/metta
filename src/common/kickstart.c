/*

bootloader
+---kickstart
	+---setup initial page mappings
	+---map/move glue and boot components to their designated addresses
	+---enable paging
	+---glue_init
		+---initialize syscalls interface page
	+---kernel_init
	    +---initialize exception and interrupt handlers
        +---find bootimage PCBs
        +---run PCB initialization upcalls
        +---enter them into schedule
        +---run scheduler

*/

//TODO: copy kickstart almost verbatim from old code?

//perhaps, convert to C