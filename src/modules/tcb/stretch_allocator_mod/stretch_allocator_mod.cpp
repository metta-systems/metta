//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stretch_allocator_module_v1_interface.h"
#include "stretch_allocator_module_v1_impl.h"
#include "system_stretch_allocator_v1_interface.h"
#include "system_stretch_allocator_v1_impl.h"
#include "stretch_allocator_v1_interface.h"
#include "stretch_allocator_v1_impl.h"
#include "frame_allocator_v1_interface.h"
#include "stretch_v1_interface.h"
#include "stretch_v1_state.h"
#include "stretch_v1_impl.h"
#include "memory_v1_interface.h"
#include "mmu_v1_interface.h"
#include "default_console.h"
#include "heap_new.h"
#include "bootinfo.h"
#include "macros.h"
#include "domain.h"
#include "algorithm"
#include "debugger.h"
#include "nucleus.h"
#include "infopage.h"

//======================================================================================================================
// state structures
//======================================================================================================================

// How many uint32_t's are needed to cover all SIDs
#define SID_ARRAY_SZ (SID_MAX/32)

struct virtual_address_space_region : public dl_link_t<virtual_address_space_region>
{
    memory_v1::virtmem_desc desc;

    // This doubly-linked list is very messy...
    virtual_address_space_region() : dl_link_t<virtual_address_space_region>() {
        init(this);
    }
};

//! Shared state.
struct server_state_t
{
    virtual_address_space_region*                    regions;

    frame_allocator_v1::closure_t*                   frames;       //!< Only in nailed sallocs.
    heap_v1::closure_t*                              heap;
    mmu_v1::closure_t*                               mmu;

    uint32_t*                                        sids;         //!< Pointer to table of SIDs in use.
    stretch_v1::closure_t**                          stretch_tab;  //!< SID -> Stretch_clp mapping.
    dl_link_t<system_stretch_allocator_v1::state_t>  clients;      //!< list of all client states.
};

// HMMM
struct stretch_list_t : public dl_link_t<stretch_list_t>
{
    stretch_v1::closure_t* stretch;

    // This doubly-linked list is very messy...
    stretch_list_t() : dl_link_t<stretch_list_t>() {
        init(this);
    }
};

// Dummy placeholder.
struct stretch_allocator_v1::state_t {};

//! Client state.
struct system_stretch_allocator_v1::state_t : public stretch_allocator_v1::state_t, public dl_link_t<system_stretch_allocator_v1::state_t>
{
    server_state_t*                  shared_state; //!< Server (shared) state.
    protection_domain_v1::id         pdid;         //!< Protection domain of client.
    protection_domain_v1::id         parent;       //!< Protection domain of client's parent.
    vcpu_v1::closure_t*              vcpu;         //!< VCPU of client.

    stretch_allocator_v1::closure_t  closure;      //!< Storage for the returned closure.

    stretch_list_t                   stretches;    //!< We hang all of the allocated stretches here.

    // This doubly-linked list is very messy...
    state_t() : dl_link_t<state_t>() {
        init(this);
    }
};

//======================================================================================================================
// helper functions
//======================================================================================================================

static sid_t alloc_sid(server_state_t* state)
{
    for (size_t i = 0; i < SID_ARRAY_SZ; ++i)
    {
        uint32_t sid = state->sids[i];
        if (sid != ~0U)
        {
            size_t k;
            for (k = 0; (k < 32) && (sid & (1 << k)); ++k) {}
            state->sids[i] = sid | (1 << k);
            kconsole << __FUNCTION__ << ": allocated sid " << i * 32 + k << endl;
            return i * 32 + k;
        }
    }
    kconsole << __FUNCTION__ << ": sid allocation FAILED" << endl;
    return NULL_SID;
}

static void register_sid(server_state_t* state, sid_t sid, stretch_v1::closure_t* stretch)
{
    state->stretch_tab[sid] = stretch;
}

// static void free_sid(server_state_t* state, sid_t sid)
// {
    // kconsole << __FUNCTION__ << ": deallocated sid " << sid << endl;
    // state->sids[sid / 32] &= ~(1 << (sid % 32));
// }

#define SYSALLOC_VA_BASE ANY_ADDRESS
// #define SYSALLOC_VA_BASE (256*MiB)
#define SYSALLOC_VA_SIZE (256*MiB)

inline console_t& operator << (console_t& con, virtual_address_space_region* region)
{
    con << "region:" << endl
        << "  START: " << region->desc.start_addr << "; start page: " << (region->desc.start_addr >> PAGE_WIDTH) << endl
        << "  bytes: " << (region->desc.n_pages << region->desc.page_width) << "; PAGES: " << region->desc.n_pages << endl
        << "  width bits: " << int(region->desc.page_width) << endl
        << "  attrs: " << region->desc.attr << endl;
    return con;
}

static bool vm_alloc(server_state_t* state, memory_v1::size size, memory_v1::address start, memory_v1::address* virt_addr, size_t* n_pages, size_t* page_width)
{
    kconsole << __FUNCTION__ << " size " << size << ", start " << start << endl;

    size_t npages = (size + PAGE_SIZE - 1) >> PAGE_WIDTH;
    dl_link_t<virtual_address_space_region>* region;

    if (unaligned(start))
    {
        // no start address requested, allocate at start of any suitable region.
        for (region = state->regions->next(); region != state->regions; region = region->next())
        {
            kconsole << *region;

            if (npages <= (*region)->desc.n_pages)
                break;
        }

        kconsole << "found " << *region << endl;

        if (region == state->regions)
        {
            kconsole << __FUNCTION__ << ": no appropriate region found!" << endl;
            return false;
        }

        *virt_addr  = (*region)->desc.start_addr;
        *n_pages    = npages;
        *page_width = (*region)->desc.page_width;

        if ((*region)->desc.n_pages > npages)
        {
            (*region)->desc.start_addr += align_to_frame_width(size, (*region)->desc.page_width);
            (*region)->desc.n_pages -= npages;
        }
        else
        {
            region->remove();
            delete *region;
        }
    }
    else // aligned(start)
    {
        // have a requested start address; compute start page
        size_t start_page = (start + PAGE_SIZE - 1) >> PAGE_WIDTH;
        size_t region_start_page{0}, region_last_page{0}, region_page_offset{0};

        for (region = state->regions->next(); region != state->regions; region = region->next())
        {
            // get the region start and end pages
            region_start_page = ((*region)->desc.start_addr + PAGE_SIZE - 1) >> PAGE_WIDTH;
            region_last_page = region_start_page + (*region)->desc.n_pages;

            // check if we're within one region
            if (start_page >= region_start_page && (start_page + npages) <= region_last_page)
                break;
        }

        if (region == state->regions)
        {
            kconsole << __FUNCTION__ << ": no appropriate region found!" << endl;
            return false;
        }

        if ((start & ((1UL << (*region)->desc.page_width) - 1)) != 0) // FIXME: check start_page alignment instead?
        {
            kconsole << __FUNCTION__ << ": requested address " << start << " not aligned to region's page width " << (*region)->desc.page_width << endl;
            nucleus::debug_stop();
        }

        region_page_offset = start_page - region_start_page;

        *virt_addr  = start_page << PAGE_WIDTH; // FIXME: use region page_width instead?
        *n_pages    = npages;
        *page_width = (*region)->desc.page_width;

        // Now take out the allocated region.
        if (region_page_offset == 0)
        {
            // allocating from the start of the region
            if ((*region)->desc.n_pages > npages)
            {
                (*region)->desc.start_addr += align_to_frame_width(size, (*region)->desc.page_width);
                (*region)->desc.n_pages -= npages;
            }
            else
            {
                region->remove();
                delete *region; // FIXME: check that the right operator delete is called!
            }
        }
        else
        {
            // allocating from the end of the region
            if ((region_page_offset + npages) == (*region)->desc.n_pages)
            {
                (*region)->desc.n_pages -= npages;
            }
            else
            {
                // allocating from the middle of the region
                auto new_region = new(state->heap) virtual_address_space_region;
                new_region->desc.start_addr = *virt_addr + align_to_frame_width(size, (*region)->desc.page_width);
                new_region->desc.n_pages = (*region)->desc.n_pages - (npages + region_page_offset);
                new_region->desc.page_width = (*region)->desc.page_width;
                new_region->desc.attr = (*region)->desc.attr;
                (*region)->desc.n_pages = region_page_offset;
                region->insert_after(*new_region);
            }
        }
    }

    kconsole << __FUNCTION__ << ": allocated [" << *virt_addr << ".." << *virt_addr + (*n_pages << *page_width) << ")" << endl;
    return true;
}

static void set_default_rights(system_stretch_allocator_v1::state_t* state, stretch_v1::closure_t* stretch)
{
    server_state_t* ss = state->shared_state;
    if (state->pdid != NULL_PDID)
    {
        ss->mmu->set_rights(state->pdid, stretch, stretch_v1::rights(stretch_v1::right_read).add(stretch_v1::right_write).add(stretch_v1::right_meta));
        if (state->parent != NULL_PDID)
        {
            ss->mmu->set_rights(state->parent, stretch, stretch_v1::rights(stretch_v1::right_meta));
        }
    }
}

//======================================================================================================================
// stretch_v1 methods
//======================================================================================================================

static memory_v1::address stretch_v1_info(stretch_v1::closure_t* self, memory_v1::size* s)
{
    kconsole << __FUNCTION__ << endl;
    *s = self->d_state->size;
    return self->d_state->base;
}

static void stretch_v1_set_rights(stretch_v1::closure_t* self, protection_domain_v1::id dom_id, stretch_v1::rights access)
{
    kconsole << __FUNCTION__ << ": pdom " << dom_id << ", sid " << self->d_state->sid << " " << access << endl;
    address_t start_page = self->d_state->base >> PAGE_WIDTH;
    size_t n_pages = self->d_state->size >> PAGE_WIDTH;
    if (nucleus::protect(dom_id, start_page, n_pages, access))
    {
        kconsole << ERROR << __FUNCTION__ << ": nucleus returned error" << endl;
        nucleus::debug_stop();
    }
}

static void stretch_v1_remove_rights(stretch_v1::closure_t* self, protection_domain_v1::id dom_id)
{

}

static void stretch_v1_set_global_rights(stretch_v1::closure_t* self, stretch_v1::rights access)
{

}

static stretch_v1::rights stretch_v1_query_rights(stretch_v1::closure_t* self, protection_domain_v1::id dom_id)
{
    return stretch_v1::rights();
}

static const stretch_v1::ops_t stretch_v1_methods =
{
    stretch_v1_info,
    stretch_v1_set_rights,
    stretch_v1_remove_rights,
    stretch_v1_set_global_rights,
    stretch_v1_query_rights
};

//======================================================================================================================
// helper functions that depend on stretch_v1_ops
//======================================================================================================================

static stretch_v1::state_t* create_stretch(server_state_t* state, address_t base, size_t n_pages)
{
    auto stretch = new(state->heap) stretch_v1::state_t;
    if (!stretch)
    {
        kconsole << __FUNCTION__ << ": stretch alloc failed!" << endl;
        return nullptr;
    }

    closure_init(&stretch->closure, &stretch_v1_methods, stretch);
    stretch->base = base;
    stretch->size = n_pages << PAGE_WIDTH;
    stretch->sid = alloc_sid(state);
    stretch->mmu = state->mmu;

    register_sid(state, stretch->sid, &stretch->closure);

    return stretch;
}

//======================================================================================================================
// stretch_allocator_v1 methods
//======================================================================================================================

//======================================================================================================================
// nailed version
//======================================================================================================================

static stretch_v1::closure_t* stretch_allocator_v1_nailed_create(stretch_allocator_v1::closure_t* self, memory_v1::size size, stretch_v1::rights global_rights)
{
    kconsole << __FUNCTION__ << ": size " << size << endl;
    memory_v1::virtmem_desc virt;
    memory_v1::physmem_desc phys;
    auto state = reinterpret_cast<system_stretch_allocator_v1::state_t*>(self->d_state);
    server_state_t* ss = state->shared_state;

    phys.start_addr = ss->frames->allocate(size, FRAME_WIDTH);
    if (phys.start_addr == NO_ADDRESS)
    {
        kconsole << __FUNCTION__ << ": Failed to get physmem" << endl;
        //raise(memory_v1_falure);
        return nullptr;
    }
    phys.frame_width = FRAME_WIDTH;
    phys.n_frames = size_in_whole_frames(size, FRAME_WIDTH);

    if (!vm_alloc(ss, size, ANY_ADDRESS /*SYSALLOC_VA_BASE + phys.start_addr*/, &virt.start_addr, &virt.n_pages, &virt.page_width))
    {
        kconsole << __FUNCTION__ << ": Failed to get virtmem" << endl;
        ss->frames->free(phys.start_addr, size);
        //raise(memory_v1_falure);
        return nullptr;
    }

    auto s = create_stretch(ss, virt.start_addr, virt.n_pages);

    if (!s)
    {
        kconsole << __FUNCTION__ << ": Failed to create_stretch" << endl;
        ss->frames->free(phys.start_addr, size);
        //raise(memory_v1_falure);
        return nullptr;
    }

    s->allocator = self;
    ss->mmu->add_mapped_range(&s->closure, virt, phys, global_rights);

    set_default_rights(state, &s->closure);

    //TODO: need locking here! at least lightweight
    //lock();
    stretch_list_t* link = new(ss->heap) stretch_list_t;
    link->stretch = &s->closure;
    state->stretches.add_to_tail(*link);
    //unlock();

    kconsole << __FUNCTION__ << ": returning stretch at " << &s->closure << endl;
    return &s->closure;
}

static stretch_allocator_v1::stretch_seq stretch_allocator_v1_nailed_create_list(stretch_allocator_v1::closure_t* self, stretch_allocator_v1::size_seq sizes, stretch_v1::rights access)
{
    kconsole << __FUNCTION__ << endl;
    return stretch_allocator_v1::stretch_seq();
}

static stretch_v1::closure_t* stretch_allocator_v1_nailed_create_at(stretch_allocator_v1::closure_t* self, memory_v1::size size, stretch_v1::rights access, memory_v1::address start, memory_v1::attrs attr, memory_v1::physmem_desc region)
{
    kconsole << __FUNCTION__ << endl;
    return 0;
}

static stretch_v1::closure_t* stretch_allocator_v1_nailed_clone(stretch_allocator_v1::closure_t* self, stretch_v1::closure_t* template_stretch, memory_v1::size size)
{
    kconsole << __FUNCTION__ << endl;
    return 0;
}

static void stretch_allocator_v1_nailed_destroy_stretch(stretch_allocator_v1::closure_t* self, stretch_v1::closure_t* stretch)
{
    kconsole << __FUNCTION__ << endl;

}

static void stretch_allocator_v1_nailed_destroy(stretch_allocator_v1::closure_t* self)
{
    kconsole << __FUNCTION__ << endl;

}

static const stretch_allocator_v1::ops_t stretch_allocator_v1_nailed_methods =
{
    stretch_allocator_v1_nailed_create,
    stretch_allocator_v1_nailed_create_list,
    stretch_allocator_v1_nailed_create_at,
    stretch_allocator_v1_nailed_clone,
    stretch_allocator_v1_nailed_destroy_stretch,
    stretch_allocator_v1_nailed_destroy
};

//======================================================================================================================
// system_stretch_allocator_v1 methods
//======================================================================================================================

/**
 * create_nailed() is used to create a 'special' stretch allocator for use by the system code for page tables,
 * protection domains, dcbs, etc.
 * On intel machines, all that 'special' means is that the stretches are backed by physical memory on creation.
 */
stretch_allocator_v1::closure_t*
system_stretch_allocator_v1_create_nailed(system_stretch_allocator_v1::closure_t* self, frame_allocator_v1::closure_t* frames, heap_v1::closure_t* heap)
{
    kconsole << __FUNCTION__ << endl;
    server_state_t* orig_state = self->d_state->shared_state;

    memory_v1::address virt;
    size_t n_pages, page_width;

    if (!vm_alloc(orig_state, SYSALLOC_VA_SIZE, SYSALLOC_VA_BASE, &virt, &n_pages, &page_width))
    {
        kconsole << __FUNCTION__ << ": couldn't allocate system virtual memory region." << endl;
        nucleus::debug_stop();
    }

    kconsole << __FUNCTION__ << ": creating shared state" << endl;
    auto shared_state = new(heap) server_state_t;
    shared_state->frames = frames;
    shared_state->heap = heap;

    // Copy the rest from our creator.
    shared_state->mmu = orig_state->mmu;
    shared_state->sids = orig_state->sids;
    shared_state->stretch_tab = orig_state->stretch_tab;

    kconsole << __FUNCTION__ << ": creating regions" << endl;
    shared_state->regions = new(heap) virtual_address_space_region;
    shared_state->regions->init();
    shared_state->clients.init();

    kconsole << __FUNCTION__ << ": creating first region" << endl;
    auto first = new(heap) virtual_address_space_region;
    first->desc.start_addr = virt;
    first->desc.n_pages = n_pages;
    first->desc.page_width = page_width;
    first->desc.attr = memory_v1::attrs_regular;
    shared_state->regions->add_to_head(*first);

    kconsole << __FUNCTION__ << ": creating client state" << endl;
    auto client_state = new(heap) system_stretch_allocator_v1::state_t;
    client_state->init();
    client_state->shared_state = shared_state;
    client_state->vcpu = nullptr;
    client_state->pdid = NULL_PDID;
    client_state->parent = NULL_PDID;
    client_state->stretches.init();
    closure_init(&client_state->closure, &stretch_allocator_v1_nailed_methods, client_state);

    // Keep a record of this 'client'.
    shared_state->clients.add_to_head(*client_state);

    kconsole << __FUNCTION__ << ": done creating" << endl;
    return &client_state->closure;
}

/**
 * create_over() is a special stretch creation method which currently is restricted to the system_stretch_allocator.
 * It provides the same functions as create_at() with the additional ability to deal with virtual memory regions which
 * have already been allocated: in this case a stretch is created over the existing region and the mappings updated
 * only with the new stretch ID / rights.
 * XXX SMH: This latter case could cause the breaking of the stretch model if called unscrupulously. This is the main
 * reason for the restriction of this method at the current time.
 */
static stretch_v1::closure_t*
system_stretch_allocator_v1_create_over(system_stretch_allocator_v1::closure_t* self, memory_v1::size size, stretch_v1::rights global_rights, memory_v1::address start, memory_v1::attrs attr, uint32_t page_width, memory_v1::physmem_desc pmem)
{
    memory_v1::virtmem_desc virtmem;
    bool update = false;
    server_state_t* state = self->d_state->shared_state;

    kconsole << __FUNCTION__ << ": start " << start << ", size " << size << endl;

    if (!vm_alloc(state, size, start, &virtmem.start_addr, &virtmem.n_pages, &virtmem.page_width))
    {
        /*
         * If we fail, we assume that the entire region is already allocated and that we are performing
         * a "map stretch over" type function.
         */
        virtmem.start_addr = page_align_down(start);
        virtmem.n_pages = size_in_whole_pages(size);
        virtmem.page_width = page_width;
        update = true;
    }

    auto s = create_stretch(state, virtmem.start_addr, virtmem.n_pages);

    if (!s)
    {
        kconsole << __FUNCTION__ << ": create_stretch failed!" << endl;
        // if (!update) vm_free();
        nucleus::debug_stop();
        return 0;
    }

    s->allocator = reinterpret_cast<stretch_allocator_v1::closure_t*>(self);//YIKES!

    kconsole << __FUNCTION__ << ": created [" << s->base << ".." << (s->base + s->size) << "), sid " << s->sid << endl;

    if (update)
        state->mmu->update_range(&s->closure, virtmem, global_rights);
    else
        state->mmu->add_range(&s->closure, virtmem, global_rights);

    set_default_rights(self->d_state, &s->closure);

    //TODO: need locking here! at least lightweight
    //lock();
    stretch_list_t* link = new(state->heap) stretch_list_t;
    link->stretch = &s->closure;
    self->d_state->stretches.add_to_tail(*link);
    //unlock();

    kconsole << __FUNCTION__ << ": returning stretch at " << &s->closure << endl;
    return &s->closure;
}

static const system_stretch_allocator_v1::ops_t system_stretch_allocator_v1_methods =
{
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    system_stretch_allocator_v1_create_nailed,
    system_stretch_allocator_v1_create_over
};

//======================================================================================================================
// stretch_allocator_module_v1 methods
//======================================================================================================================

static system_stretch_allocator_v1::closure_t*
stretch_allocator_module_v1_create(stretch_allocator_module_v1::closure_t* self, heap_v1::closure_t* heap, mmu_v1::closure_t* mmu)
{
    kconsole << __FUNCTION__ << endl;
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    server_state_t* shared_state = new(heap) server_state_t;
    shared_state->heap = heap;
    shared_state->mmu = mmu;
    shared_state->frames = nullptr;
    shared_state->clients.init();
    shared_state->regions = new(heap) virtual_address_space_region;

    auto region = new(heap) virtual_address_space_region;
    region->desc.start_addr = 0;
    region->desc.n_pages = 0x100000; // 4GiB address space.
    region->desc.page_width = PAGE_WIDTH;
    region->desc.attr = memory_v1::attrs_regular;
    shared_state->regions->add_to_tail(*region);

    // by this point allocated memory contains
    // @0x1000 PIP, 1 page
    // @0x8000 bootinfo page, 1 page
    // @0xb8000 video ram, 1 page
    // currently running code (entry.cpp) somewhere above 0x100000
    // then.. loaded modules,
    // then.. allocated mmu state and heap
    // then.. some more loaded modules?

    // Allocate already used space.
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [shared_state](const multiboot_t::mmap_entry_t* e)
    {
        if ((e->type() != multiboot_t::mmap_entry_t::non_free)
         && (e->type() != multiboot_t::mmap_entry_t::bootinfo))
            return;

        kconsole << "vm region @ " << e->start() << ", " << int(e->size()) << " bytes." << endl;

        memory_v1::address virt;
        size_t n_pages, page_width;

        if (!vm_alloc(shared_state, e->size(), e->start(), &virt, &n_pages, &page_width))
        {
            kconsole << __FUNCTION__ << ": cannot allocate already used VM region. FAIL!" << endl;
            nucleus::debug_stop();
        }
    });

    // Allocate space for SID allocation table.
    shared_state->sids = new(heap) uint32_t [SID_ARRAY_SZ];
    memutils::clear_memory(shared_state->sids, SID_ARRAY_SZ * sizeof(uint32_t));

    // Allocate space for SID->stretch mapping.
    shared_state->stretch_tab = new(heap) stretch_v1::closure_t* [SID_MAX];
    memutils::clear_memory(shared_state->stretch_tab, SID_MAX * sizeof(stretch_v1::closure_t*));

    // Poke it into the info page.
    INFO_PAGE.stretch_mapping = shared_state->stretch_tab;

    system_stretch_allocator_v1::state_t* client_state = new(heap) system_stretch_allocator_v1::state_t;
    client_state->init();
    client_state->shared_state = shared_state;
    client_state->vcpu = nullptr;
    client_state->pdid = NULL_PDID;
    client_state->parent = NULL_PDID;
    client_state->stretches.init();

    // Oh, uglyness, oh, casting!
    closure_init(&client_state->closure, reinterpret_cast<const stretch_allocator_v1::ops_t*>(&system_stretch_allocator_v1_methods), client_state);

    shared_state->clients.add_to_head(*client_state);

    return reinterpret_cast<system_stretch_allocator_v1::closure_t*>(&client_state->closure);
}

static void stretch_allocator_module_v1_finish_init(stretch_allocator_module_v1::closure_t* self, system_stretch_allocator_v1::closure_t* stretch_allocator, vcpu_v1::closure_t* vcpu, protection_domain_v1::id pdid)
{
    stretch_allocator->d_state->vcpu = vcpu;
    stretch_allocator->d_state->pdid = pdid;
}

static const stretch_allocator_module_v1::ops_t stretch_allocator_module_v1_methods =
{
    stretch_allocator_module_v1_create,
    stretch_allocator_module_v1_finish_init
};

static stretch_allocator_module_v1::closure_t clos =
{
    &stretch_allocator_module_v1_methods,
    nullptr
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_allocator_module, v1, clos);
