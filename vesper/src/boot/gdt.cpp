#include "gdt.h"

extern "C" void activate_gdt(uint32 base);

GdtEntry gdt_entries[5];

/* Singleton? */
void GlobalDescriptorTable::init()
{
	static GlobalDescriptorTable gdt;
}

GlobalDescriptorTable::GlobalDescriptorTable()
{
	limit = sizeof(gdt_entries)-1;
	base = (uint32)&gdt_entries;

	gdt_entries[1].set_seg(0, 0xFFFFFFFF, 0, GdtEntry::code); // ring0 CS
	gdt_entries[2].set_seg(0, 0xFFFFFFFF, 0, GdtEntry::data); // ring0 DS
	gdt_entries[3].set_seg(0, 0xFFFFFFFF, 3, GdtEntry::code); // ring3 CS
	gdt_entries[4].set_seg(0, 0xFFFFFFFF, 3, GdtEntry::data); // ring3 DS

	activate_gdt((uint32)this);
}
