#ifndef __INCLUDED_GDT_H
#define __INCLUDED_GDT_H

#include "common.h"

class GdtEntry
{
	public:
		enum segtype_e
		{
			code = 0xb,
			data = 0x3,
			tss  = 0x9
		};

		void set_seg(uint32_t base, uint32_t limit, segtype_e type, int dpl);
		void set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl);

	private:
		union {
			uint32_t raw[2];
			struct {
				uint32_t limit_low  : 16;
				uint32_t base_low   : 24 __attribute__((packed));
				uint32_t type       :  4;
				uint32_t s          :  1;
				uint32_t dpl        :  2;
				uint32_t present    :  1;
				uint32_t limit_high :  4;
				uint32_t avl        :  2;
				uint32_t datasize   :  1;
				uint32_t granularity:  1;
				uint32_t base_high  :  8;
			} d __attribute__((packed));
		} x;
};

INLINE void GdtEntry::set_seg(uint32_t base, uint32_t limit, segtype_e type, int dpl)
{
	if (limit > (1 << 20))
	{
		x.d.limit_low  = (limit >> 12) & 0xFFFF;
		x.d.limit_high = (limit >> 28) & 0xF;
		x.d.granularity = 1; /* 4K granularity	*/
	}
	else
	{
		x.d.limit_low  =  limit        & 0xFFFF;
		x.d.limit_high = (limit >> 16) & 0xFF;
		x.d.granularity = 0; /* 1B granularity	*/
	}

	x.d.base_low   = base & 0xFFFFFF;
	x.d.base_high  = (base >> 24) & 0xFF;
	x.d.type = type;
	x.d.dpl = dpl;

	/* default fields */
	x.d.present = 1;
	x.d.datasize = 1; /* 32-bit segment */
	x.d.s = 1;	/* non-system segment	*/

	/* unused fields */
	x.d.avl = 0;
}

INLINE void GdtEntry::set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl)
{
    x.d.limit_low  =  limit        &   0xFFFF;
    x.d.limit_high = (limit >> 16) &     0xFF;
    x.d.base_low   =  base         & 0xFFFFFF;
    x.d.base_high  = (base >> 24)  &     0xFF;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.present = 1;	/* present		*/
    x.d.granularity = 0;	/* byte granularity	*/
    x.d.datasize = 0;	/* 32-bit segment	FIXME WTF*/
    x.d.s = 0;	/* non-system segment	FIXME WTF*/

    /* unused fields */
    x.d.avl = 0;
}

class GlobalDescriptorTable
{
	public:
		static void init();

	private:
		GlobalDescriptorTable();
		uint16_t limit;
		uint32_t base;
} __attribute__((packed));

#endif /* !__INCLUDED_GDT_H */
