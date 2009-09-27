#include "memory.h"
#include "multiboot.h"
#include "memory/new.h"
#include "vector_base.h"
#include "memblock_base.h"
#include "obj_copier.h"
#include "obj_type_behavior.h"

struct area_t
{
    enum intersect { none, left_edge, within, right_edge };
    address_t start, end;

    area_t() : start(0), end(0) {}
    area_t(address_t start, address_t end) : start(start), end(end) {}
    area_t(multiboot_t::mmap_entry_t *ent);

    intersect intersects(const area_t& a2);

    size_t npages() // type_traits<address_type>::size_type
    {
        return (end - start) / PAGE_SIZE;
    }
};

area_t::area_t(multiboot_t::mmap_entry_t *ent)
{
    start = page_align_up<address_t>(ent->address());
    end = page_align_down<address_t>(ent->address() + ent->size());
}

//UNITTEST:
//(1,2).intersect(-1,0) == none
//(1,2).intersect(3,4) == none
//(1,2).intersect(0,1) == left_edge
//(1,2).intersect(2,3) == right_edge
//(1,3).intersect(1,2) == within
//(1,3).intersect(2,3) == within
area_t::intersect area_t::intersects(const area_t& a2)
{
    if (a2.end < start || a2.start > end)
        return none;
    if (a2.start >= start && a2.end <= end)
        return within;

    if (a2.start <= end && a2.end > end)
        return right_edge;

    return left_edge;
}

template <class T>
class inplace_allocator
{
    char buf[4096];
public:
    T* allocate(T* /*old_mem*/, size_t /*old_size*/, size_t /*new_size*/)
    {
        return reinterpret_cast<T*>(buf);
    }
    void deallocate(T* /*mem*/, size_t /*size*/) {}
};

// Given a multiboot_t::mmap_t create a sorted memory regions map.
void mmap_prepare(multiboot_t::mmap_t* mmap)
{
    vector_base<memblock_base<area_t, inplace_allocator<area_t>, obj_copier<area_t>>, obj_type_behavior<area_t>, tight_allocation_policy> areas;

    // Start with 1 region covering whole _available_ memory.
    // Use memory map to cut bits off of left or right edge of region
    // or split it up in two.
    areas.push_back(area_t(0, ~0)); //.append
    ASSERT(areas[0].npages() == 0xfffff);

    multiboot_t::mmap_entry_t* mmi = mmap->first_entry();
    while (mmi)
    {
        area_t a(mmi);
        if (mmi->is_free())
        {
            for (uint32_t i = 0; i < areas.size(); i++)
            {
                area_t a2 = areas[i];
                switch (a.intersects(a2))
                {
                    case area_t::none: // add new region
                    case area_t::left_edge: case area_t::right_edge: // meld into one region
                    case area_t::within: // do nothing
                        break;
                }
            }
        }
        else // occupied
        {
            for (uint32_t i = 0; i < areas.size(); i++)
            {
                area_t a2 = areas[i];
                // occupied regions take precedence over free regions
                switch (a.intersects(a2))
                {
                    case area_t::none: // do nothing
                    case area_t::left_edge: case area_t::right_edge: // subtract from existing region
                    case area_t::within: // split region in 2
                        break;
                }
            }
        }

        mmi = mmap->next_entry(mmi);
    }

}
