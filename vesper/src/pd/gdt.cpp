#include "gdt.h"

extern "C" void activate_gdt(Address base); // in activate.s

GdtEntry gdt_entries[5];

/* Singleton instead? */
void GlobalDescriptorTable::init()
{
	static GlobalDescriptorTable gdt;
}

GlobalDescriptorTable::GlobalDescriptorTable()
{
	limit = sizeof(gdt_entries)-1;
	base = (Address)&gdt_entries;

	gdt_entries[0].set_null();                                // null
	gdt_entries[1].set_seg(0, 0xFFFFFFFF, GdtEntry::code, 0); // ring0 CS
	gdt_entries[2].set_seg(0, 0xFFFFFFFF, GdtEntry::data, 0); // ring0 DS
	gdt_entries[3].set_seg(0, 0xFFFFFFFF, GdtEntry::code, 3); // ring3 CS
	gdt_entries[4].set_seg(0, 0xFFFFFFFF, GdtEntry::data, 3); // ring3 DS

	activate_gdt((Address)this);
}
