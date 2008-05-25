#ifndef __INCLUDED_GDT_H
#define __INCLUDED_GDT_H

#include "Types.h"

class GdtEntry
{
	public:
		enum segtype_e
		{
			code = 0xb,
			data = 0x3,
			tss  = 0x9
		};

		void set_seg(uint32 base, uint32 limit, int dpl, segtype_e type);
		void set_sys(uint32 base, uint32 limit, int dpl, segtype_e type);

	private:
		union {
			uint32 raw[2];
			struct {
				uint32 limit_low  : 16;
				uint32 base_low   : 24 __attribute__((packed));
				uint32 type       :  4;
				uint32 s          :  1;
				uint32 dpl        :  2;
				uint32 present    :  1;
				uint32 limit_high :  4;
				uint32 avl        :  2;
				uint32 datasize   :  1;
				uint32 granularity:  1;
				uint32 base_high  :  8;
			} d __attribute__((packed));
		} x;
};

INLINE void GdtEntry::set_seg(uint32 base, uint32 limit, int dpl, segtype_e type)
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

INLINE void GdtEntry::set_sys(uint32 base, uint32 limit, int dpl, segtype_e type)
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
		uint16 limit;
		uint32 base;
} __attribute__((packed));

#endif /* !__INCLUDED_GDT_H */
