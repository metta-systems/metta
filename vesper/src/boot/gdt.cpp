#include "gdt.h"

extern "C" void activate_gdt(uint32_t base);

GdtEntry gdt_entries[5];

/* Singleton? */
void GlobalDescriptorTable::init()
{
	static GlobalDescriptorTable gdt;
}

GlobalDescriptorTable::GlobalDescriptorTable()
{
	limit = sizeof(gdt_entries)-1;
	base = (uint32_t)&gdt_entries;

	memset(&gdt_entries, 0, sizeof(gdt_entries));

	gdt_entries[1].set_seg(0, 0xFFFFFFFF, GdtEntry::code, 0); // ring0 CS
	gdt_entries[2].set_seg(0, 0xFFFFFFFF, GdtEntry::data, 0); // ring0 DS
	gdt_entries[3].set_seg(0, 0xFFFFFFFF, GdtEntry::code, 3); // ring3 CS
	gdt_entries[4].set_seg(0, 0xFFFFFFFF, GdtEntry::data, 3); // ring3 DS

	activate_gdt((uint32_t)this);
}
