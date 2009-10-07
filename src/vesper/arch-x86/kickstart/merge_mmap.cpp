#include "memory.h"
#include "multiboot.h"
#include "bootinfo.h"
#include "memory/new.h"
#include "vector_base.h"
#include "memblock_base.h"
#include "obj_copier.h"
#include "obj_allocator.h"
#include "obj_type_behavior.h"
#include "default_console.h"

struct area_t
{
    enum intersect { none, left_edge, within, right_edge };
    address_t start, end;

    area_t() : start(0), end(0) {}
    area_t(address_t start, address_t end) : start(start), end(end) {}
    area_t(multiboot_t::mmap_entry_t *ent);

    intersect intersects(const area_t& a2);
};

area_t::area_t(multiboot_t::mmap_entry_t *ent)
{
    start = page_align_up<address_t>(ent->address());
    end = page_align_down<address_t>(ent->address() + ent->size()) - 1;
}

// How passed area is layed out compared to this.
//UNITTEST:
//(1,2).intersect(-1,0) == none
//(1,2).intersect(3,4) == none
//(1,2).intersect(0,1) == left_edge
//(1,2).intersect(2,3) == right_edge
//(1,3).intersect(1,2) == left_edge
//(1,3).intersect(2,3) == right_edge
//(1,4).intersect(2,3) == within
area_t::intersect area_t::intersects(const area_t& a2)
{
    if (a2.end < start || a2.start > end)
        return none;
    if (a2.start > start && a2.end < end)
        return within;

    if (a2.start <= end && a2.end > end)
        return right_edge;

    return left_edge;
}

template <int N, class T>
class inplace_allocator
{
    char buf[N * sizeof(T)];
public:
    typedef T value_type;

    T* allocate(T* /*old_mem*/, size_t /*old_size*/, size_t /*new_size*/)
    {
        return reinterpret_cast<T*>(buf);
    }
    void deallocate(T* /*mem*/, size_t /*size*/) {}
};

typedef vector_base<memblock_base<area_t, obj_allocator<inplace_allocator<32, area_t>>, obj_copier<area_t>>, obj_type_behavior<area_t>, tight_allocation_policy> area_vector_t;

// Given a multiboot_t::mmap_t create a sorted memory regions map.
void mmap_prepare(multiboot_t::mmap_t* mmap, bootinfo_t& bi_page)
{
    area_vector_t areas;
    areas.reserve(32);

    // Start with 1 empty area.
    // Use memory map to add available areas and subtract unavailable ones.
    areas.push_back(area_t(0, 0));

    multiboot_t::mmap_entry_t* mmi = mmap->first_entry();
    while (mmi)
    {
        area_t introduced(mmi);
        kconsole << "next mmap entry " << introduced.start << " to " << introduced.end << " is " << (mmi->is_free() ? "free" : "occupied") << endl;
        if (mmi->is_free())
        {
            for (uint32_t i = 0; i < areas.size(); i++)
            {
                area_t existing = areas[i];
                switch (existing.intersects(introduced))
                {
                    case area_t::none:
                        kconsole << "add new area " << introduced.start << " " << introduced.end << endl;
                        areas.insert(i+1, introduced);
                        i++;
                        break;
                    case area_t::left_edge:
                        kconsole << "join on left with existing area" << endl;
                        areas[i] = area_t(introduced.start, existing.end);
                        break;
                    case area_t::right_edge:
                        kconsole << "join on right with existing area" << endl;
                        areas[i] = area_t(existing.start, introduced.end);
                        break;
                    case area_t::within: // do nothing
                        break;
                }
            }
        }
        else // occupied
        {
            for (uint32_t i = 0; i < areas.size(); i++)
            {
                area_t existing = areas[i];
                // occupied areas take precedence over free areas
                switch (existing.intersects(introduced))
                {
                    case area_t::none: // do nothing
                        break;
                    case area_t::left_edge:
                        kconsole << "sub from existing area on left" << endl;
                        areas[i] = area_t(introduced.end + 1, existing.end);
                        break;
                    case area_t::right_edge:
                        kconsole << "sub from existing area on right" << endl;
                        areas[i] = area_t(existing.start, introduced.start - 1);
                        break;
                    case area_t::within:
                        kconsole << "split area in 2:";
                        area_t new_a1(existing.start, introduced.start - 1);
                        area_t new_a2(introduced.end + 1, existing.end);
                        kconsole << " (" << existing.start << ", " << existing.end << ") => (" << new_a1.start << ", " << new_a1.end << ") (" << new_a2.start << ", " << new_a2.end << ")" << endl;
                        areas.erase(i);
                        areas.insert(i+1, new_a1);
                        areas.insert(i+1, new_a2);
                        i++;
                        break;
                }
            }
        }

        mmi = mmap->next_entry(mmi);
    }

    kconsole << "finished. final free mmap:" << endl;
    multiboot_t mb(bi_page.multiboot_header());
    bi_page.decrease_size(mb.memory_map()->size());
    mb.memory_map()->set_size(0);
    for (uint32_t i = 0; i < areas.size(); i++)
    {
        area_t a = areas[i];
        multiboot_t::mmap_entry_t entry;
        entry.set_region(a.start, a.end - a.start + 1, entry.free);
        entry.set_entry_size(sizeof(multiboot_t::mmap_entry_t));
        kconsole << "  " << a.start << " to " << a.end << endl;
        bi_page.append_mmap_entry(&entry);
    }
}
