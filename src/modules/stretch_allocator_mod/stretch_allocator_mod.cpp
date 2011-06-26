#include "stretch_allocator_module_v1_interface.h"
#include "stretch_allocator_module_v1_impl.h"
#include "system_stretch_allocator_v1_interface.h"
#include "system_stretch_allocator_v1_impl.h"
#include "stretch_allocator_v1_interface.h"
#include "stretch_allocator_v1_impl.h"
#include "stretch_v1_interface.h"
#include "memory_v1_interface.h"
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
    memory_v1_virtmem_desc desc;
};

//! Shared state.
struct server_state_t
{
    virtual_address_space_region* regions;

    frame_allocator_v1_closure* frames;    //!< Only in nailed sallocs.
    heap_v1_closure* heap;
    mmu_v1_closure*  mmu;

    uint32_t*            sids;             //!< Pointer to table of SIDs in use.
    stretch_v1_closure** stretch_tab;      //!< SID -> Stretch_clp mapping.
    dl_link_t<system_stretch_allocator_v1_state> clients;          //!< list of all client states.
};

// HMMM
struct stretch_list_t : public dl_link_t<stretch_list_t>
{
    stretch_v1_closure* stretch;
};

// Dummy placeholder.
struct stretch_allocator_v1_state {};

//! Client state.
struct system_stretch_allocator_v1_state : public stretch_allocator_v1_state, public dl_link_t<system_stretch_allocator_v1_state>
{
    server_state_t*          shared_state; //!< Server (shared) state.
    protection_domain_v1_id  pdid;         //!< Protection domain of client.
    protection_domain_v1_id  parent;       //!< Protection domain of client's parent.
    vcpu_v1_closure*         vcpu;         //!< VCPU of client.

    stretch_allocator_v1_closure closure;  //!< Storage for the returned closure.

    stretch_list_t stretches;              //!< We hang all of the allocated stretches here.
}; 

//======================================================================================================================
// stretch_allocator_v1 methods
//======================================================================================================================

//======================================================================================================================
// nailed version
//======================================================================================================================

static const stretch_allocator_v1_ops stretch_allocator_v1_nailed_methods = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

//======================================================================================================================
// system_stretch_allocator_v1 methods
//======================================================================================================================

#define SYSALLOC_VA_BASE ANY_ADDRESS
#define SYSALLOC_VA_SIZE (256*MiB)

static bool vm_alloc(server_state_t* state, memory_v1_size size, memory_v1_address start, memory_v1_address* virt_addr, size_t* n_pages, size_t* page_width)
{
    kconsole << __PRETTY_FUNCTION__ << " size " << size << ", start " << start << endl;

    size_t npages = (size + PAGE_SIZE - 1) >> PAGE_WIDTH; 
    virtual_address_space_region* region;

    kconsole << "Check1" << endl;

    if (unaligned(start))
    {
        kconsole << "Check2" << endl;
        // no start address requested, allocate at start of any suitable region.
        for (region = state->regions->next; region != state->regions; region = region->next)
        {
            kconsole << "region:" << endl
                     << "  start: " << region->desc.start_addr << endl
                     << "  pages: " << region->desc.n_pages << endl
                     << "  width: " << region->desc.page_width << endl
                     << "  attrs: " << region->desc.attr << endl;

            if (npages <= region->desc.n_pages)
                break;
        }

        kconsole << "found region " << region << endl;

        if (region == state->regions)
        {
            kconsole << __FUNCTION__ << ": no appropriate region found!" << endl;
            return false;
        }

        *virt_addr  = region->desc.start_addr;
        *n_pages    = npages;
        *page_width = region->desc.page_width;

        if (region->desc.n_pages > npages)
        {
            region->desc.start_addr += align_to_frame_width(size, region->desc.page_width);
            region->desc.n_pages -= npages;
        }
        else
        {
            region->remove();
            delete region;
        }
    }
    else
    {
        // have a requested start address; compute start page
        size_t start_page = (start + PAGE_SIZE - 1) >> PAGE_WIDTH;
        size_t region_start_page = 0, region_last_page = 0, region_page_offset = 0;

        for (region = state->regions->next; region != state->regions; region = region->next)
        {
            // get the region start and end pages
            region_start_page = (region->desc.start_addr + PAGE_SIZE - 1) >> PAGE_WIDTH;
            region_last_page = region_start_page + region->desc.n_pages;
            
            // check if we're within one region
            if (start_page >= region_start_page && (start_page + npages) <= region_last_page)
                break;
        }

        if (region == state->regions)
        {
            kconsole << __FUNCTION__ << ": no appropriate region found!" << endl;
            return false;
        }

        if ((start & ((1UL << region->desc.page_width) - 1)) != 0) // FIXME: check start_page alignment instead?
        {
            kconsole << __FUNCTION__ << ": requested address " << start << " not aligned to region's page width " << region->desc.page_width << endl;
            nucleus::debug_stop();
        }

        region_page_offset = start_page - region_start_page;

        *virt_addr  = start_page << PAGE_WIDTH; // FIXME: use region page_width instead?
        *n_pages    = npages;
        *page_width = region->desc.page_width;

        // Now take out the allocated region.
        if (region_page_offset == 0)
        {
            // allocating from the start of the region
            if (region->desc.n_pages > npages)
            {
                region->desc.start_addr += align_to_frame_width(size, region->desc.page_width);
                region->desc.n_pages -= npages;
            }
            else
            {
                region->remove();
                delete region; // FIXME: check that the right operator delete is called!
            }
        }
        else
        {
            // allocating from the end of the region
            if ((region_page_offset + npages) == region->desc.n_pages)
            {
                region->desc.n_pages -= npages;
            }
            else
            {
                // allocating from the middle of the region
                auto new_region = new(state->heap) virtual_address_space_region;
                new_region->desc.start_addr = *virt_addr + align_to_frame_width(size, region->desc.page_width);
                new_region->desc.n_pages = region->desc.n_pages - (npages + region_page_offset);
                new_region->desc.page_width = region->desc.page_width;
                new_region->desc.attr = region->desc.attr;
                region->desc.n_pages = region_page_offset;
                region->insert_after(new_region);
            }
        }
    }

    return true;
}

/*!
 * create_nailed() is used to create a 'special' stretch allocator for use by the system code for page tables,
 * protection domains, dcbs, etc.
 * On alphas, 'special' means that the stretches are all allocated from a 4GB region of memory;
 * this memory is backed by physical memory in a simple manner - the high bits are zapped to give the physical
 * address. The fact of the magic prefix also simplifies the task of sanity checking arguments for the palcode.
 * On intel machines, all that 'special' means is that the stretches are backed by physical memory on creation.
 */
// salloc_expt.c:1194
stretch_allocator_v1_closure* system_stretch_allocator_v1_create_nailed(system_stretch_allocator_v1_closure* self, frame_allocator_v1_closure* frames, heap_v1_closure* heap)
{
    kconsole << __FUNCTION__ << endl;
    server_state_t* orig_state = self->state->shared_state;

    memory_v1_address virt;
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
    first->desc.attr = memory_v1_attrs_regular;
    shared_state->regions->add_to_head(first);

    kconsole << __FUNCTION__ << ": creating client state" << endl;
    auto client_state = new(heap) system_stretch_allocator_v1_state;
    client_state->init();
    client_state->shared_state = shared_state;
    client_state->vcpu = NULL;
    client_state->pdid = NULL_PDID;
    client_state->parent = NULL_PDID;
    client_state->stretches.init();
    client_state->closure.methods = &stretch_allocator_v1_nailed_methods;
    client_state->closure.state = client_state;

    // Keep a record of this 'client'.
    shared_state->clients.add_to_head(client_state);

    kconsole << __FUNCTION__ << ": done creating" << endl;
    return &client_state->closure;
}

static const system_stretch_allocator_v1_ops system_stretch_allocator_v1_methods = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    system_stretch_allocator_v1_create_nailed,
    NULL
};

//======================================================================================================================
// stretch_allocator_module_v1 methods
//======================================================================================================================

static system_stretch_allocator_v1_closure* stretch_allocator_module_v1_create(stretch_allocator_module_v1_closure* self, heap_v1_closure* heap, mmu_v1_closure* mmu)
{
    kconsole << __FUNCTION__ << endl;
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

    server_state_t* shared_state = new(heap) server_state_t;
    shared_state->heap = heap;
    shared_state->mmu = mmu;
    shared_state->frames = NULL;
    shared_state->clients.init();
    shared_state->regions = new(heap) virtual_address_space_region;
    shared_state->regions->init();

    auto region = new(heap) virtual_address_space_region;
    region->desc.start_addr = 0;
    region->desc.n_pages = 0x100000; // 4GiB address space.
    region->desc.page_width = PAGE_WIDTH;
    region->desc.attr = memory_v1_attrs_regular;
    shared_state->regions->add_to_tail(region);

    // by this point allocated memory contains
    // @0x1000 PIP, 1 page
    // @0x8000 bootinfo page, 1 page
    // @0xb8000 video ram, 1 page
    // @0x100000.. allocated mmu state and heap (up to 0x1ad000 currently)
    // currently running code (entry.cpp) somewhere in between
    // @0x400000.. loaded modules, up to 0x42d000 currently.

    // Allocate already used space.
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [shared_state](const multiboot_t::mmap_entry_t* e)
    {
        if ((e->type() != multiboot_t::mmap_entry_t::non_free)
         && (e->type() != multiboot_t::mmap_entry_t::bootinfo))
            return;

        kconsole << "vm region @ " << e->start() << ", " << int(e->size()) << " bytes." << endl;

        memory_v1_address virt;
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
    shared_state->stretch_tab = new(heap) stretch_v1_closure* [SID_MAX];
    memutils::clear_memory(shared_state->stretch_tab, SID_MAX * sizeof(stretch_v1_closure*));

    // Poke it into the info page.
    INFO_PAGE.stretch_mapping = shared_state->stretch_tab;

    system_stretch_allocator_v1_state* client_state = new(heap) system_stretch_allocator_v1_state;
    client_state->init();
    client_state->shared_state = shared_state;
    client_state->vcpu = NULL;
    client_state->pdid = NULL_PDID;
    client_state->parent = NULL_PDID;
    client_state->stretches.init();

    // Oh, uglyness, oh, casting!
    client_state->closure.methods = reinterpret_cast<const stretch_allocator_v1_ops*>(&system_stretch_allocator_v1_methods);
    client_state->closure.state = client_state;

    shared_state->clients.add_to_head(client_state);

    return reinterpret_cast<system_stretch_allocator_v1_closure*>(&client_state->closure);
}

static void stretch_allocator_module_v1_finish_init(stretch_allocator_module_v1_closure* self, system_stretch_allocator_v1_closure* stretch_allocator, vcpu_v1_closure* vcpu, protection_domain_v1_id pdid)
{
    stretch_allocator->state->vcpu = vcpu;
    stretch_allocator->state->pdid = pdid;
}

static const stretch_allocator_module_v1_ops stretch_allocator_module_v1_methods = {
    stretch_allocator_module_v1_create,
    stretch_allocator_module_v1_finish_init
};

static const stretch_allocator_module_v1_closure clos = {
    &stretch_allocator_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_allocator_module, v1, clos);
