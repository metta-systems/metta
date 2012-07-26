//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "algorithm"
#include "default_console.h"
#include "bootinfo.h"
#include "infopage.h"
#include "mmu_module_v1_interface.h"
#include "mmu_module_v1_impl.h"
#include "mmu_v1_interface.h"
#include "mmu_v1_impl.h"
#include "ramtab_v1_interface.h"
#include "ramtab_v1_impl.h"
#include "page_directory.h"
#include "system_frame_allocator_v1_interface.h"
#include "heap_v1_interface.h"
#include "stretch_allocator_v1_interface.h"
#include "nucleus.h"
#include "cpu.h"
#include "domain.h"
#include "stretch_v1_state.h"

//======================================================================================================================
// mmu_v1 state
//======================================================================================================================

static const size_t N_L1_TABLES = 1024;
static const size_t N_L2_ENTRIES = 1024;

struct ramtab_entry_t
{
    address_t owner;        /* PHYSICAL address of owning domain's DCB   */
    uint16_t frame_width;   /* Logical width of the frame                */
    uint16_t state;         /* Misc bits, e.g. is_mapped, is_nailed, etc */
} PACKED;

struct pdom_st
{
    uint16_t               refcnt;  /* reference count on this pdom    */
    uint16_t               gen;     /* current generation of this pdom */
    stretch_v1::closure_t* stretch; /* handle on stretch (for destroy) */
};

struct pdom_t
{
    uint8_t rights[SID_MAX/2];
};

struct shadow_t
{
    sid_t sid;
    uint16_t flags;
} PACKED;

#define SHADOW(_va)  reinterpret_cast<shadow_t*>(reinterpret_cast<char*>(_va) + 4*KiB)

typedef uint8_t     l2_info;    /* free or used info for 1K L2 page tables */
#define L2FREE      (l2_info)0x12
#define L2USED      (l2_info)0x99

#define PDIDX(_pdid)   ((_pdid) & 0xffff)
#define PDIDX_MAX       0x80   /* Allow up to 128 protection domains */

struct mmu_v1::state_t
{
    page_t                l1_mapping[N_L1_TABLES]; /**< Level 1 page directory      */
    shadow_t              l1_shadows[N_L1_TABLES]; /**< Level 1 shadows (4Mb pages) */
    page_t                l1_virt[N_L1_TABLES];    /**< Virtual addresses of L2s    */

    mmu_v1::closure_t     mmu_closure;
    ramtab_v1::closure_t  ramtab_closure;

    uint32_t              next_pdidx;          /* Next free pdom idx (hint) */
    pdom_t*               pdom_tbl[PDIDX_MAX]; /* Map pdom idx to pdom_t's  */
    pdom_st               pdominfo[PDIDX_MAX]; /* Map pdom idx to pdom_st's */

    bool                  use_global_pages;    /* Set iff we can use PGE    */

    /*system_*/frame_allocator_v1::closure_t*  system_frame_allocator;
    heap_v1::closure_t*                        heap;
    stretch_allocator_v1::closure_t*           stretch_allocator;

//    uint32_t              n_frames;//FIXME: UNUSED!!?!

    address_t             l1_mapping_virt; /* virtual  address of l1 page table */
    address_t             l1_mapping_phys; /* physical address of l1 page table */

    address_t             l2_virt;   /* virtual  address of l2 array base */
    address_t             l2_phys;   /* physical address of l2 array base */

    address_t             l1_virt_virt; /* virtual address of l2 PtoV table  */

    ramtab_entry_t*       ramtab;      /* base of ram table            */
    size_t                ramtab_size; /* size of ram table            */

    uint32_t              l2_max;   /* index of the last available chunk   */
    uint32_t              l2_next;  /* index of first potential free chunk */
    l2_info               info[0];  /* free/used L2 info; actually l2_max entries   */
};

//======================================================================================================================
// helper methods
//======================================================================================================================

#define L2SIZE          (8*KiB)                // 4K for L2 pagetable + 4K for shadow(?)

inline bool alloc_l2table(mmu_v1::state_t* state, address_t *l2va, address_t *l2pa)
{
    size_t i;

    for (i = state->l2_next; i < state->l2_max; i++)
        if (state->info[i] == L2FREE)
            break;

    if (i == state->l2_max)
    {
        // XXX go get some more mem from frames/salloc
        kconsole << "alloc_l2table: out of memory for tables!" << endl;
        return false;
    }

    state->info[i] = L2USED;
    state->l2_next = i+1;

    *l2va = state->l2_virt + (L2SIZE * i);
    memutils::clear_memory(reinterpret_cast<void*>(*l2va), L2SIZE);
    *l2pa = state->l2_phys + (L2SIZE * i);

    kconsole << "alloc_l2table: new L2 table at va=" << *l2va << ", pa=" << *l2pa << ", shadow va=" << SHADOW(*l2va) << endl;
    return true;
}

static bool add4k_page(mmu_v1::state_t* state, address_t va, page_t pte, sid_t sid)
{
    int l1idx, l2idx;
    address_t  l2va, l2pa;

    l1idx  = pde_entry(va);

    if (!state->l1_mapping[l1idx].is_present())
    {
        kconsole << "mapping va=" << va << " requires new L2 table" << endl;
        if (!alloc_l2table(state, &l2va, &l2pa)) {
            kconsole << "!!! intel_mmu:add4k_page - cannot alloc l2 table." << endl;
            return false;
        }
        state->l1_mapping[l1idx].set_frame(l2pa);
        state->l1_mapping[l1idx].set_flags(page_t::writable|page_t::write_through);
        state->l1_virt[l1idx].set_frame(l2va);
    }

    if (state->l1_mapping[l1idx].is_4mb())
    {
        kconsole << "URK! mapping va=" << va << " would use a 4MB page!" << endl;
        return false;
    }

    l2pa = state->l1_mapping[l1idx].frame();
    l2va = state->l2_virt + (l2pa - state->l2_phys);
    // XXX PARANOIA
    if (l2va != state->l1_virt[l1idx].frame())
        kconsole << "virtual addresses out of sync: l2va=" << l2va << ", not " << state->l1_virt[l1idx].frame() << endl;

    // Ok, once here, we have a pointer to our l2 table in "l2va"
    l2idx = pte_entry(va);

    // Set pte into real ptab
    reinterpret_cast<page_t*>(l2va)[l2idx] = pte;

    // Setup shadow pte (holds sid + original global rights)
    SHADOW(l2va)[l2idx].sid = sid;  // store sid in shadow
    SHADOW(l2va)[l2idx].flags = pte.flags();
    return true;
}

/*
** update4k_pages is used to modify the information in the
** page table about a particular contiguous range of pages.
** It returns the number of pages (from 1 upto npages)
** successfully updated, or zero on failure.
*/
static size_t update4k_pages(mmu_v1::state_t* state, address_t va, size_t n_pages, page_t pte, sid_t sid)
{
    int l1idx, l2idx;
    address_t  l2va, l2pa;

    l1idx  = pde_entry(va);

    if (!state->l1_mapping[l1idx].is_present())
    {
        kconsole << __FUNCTION__ << ": page at " << va << " not present, cannot update" << endl;
        return 0;
    }

    if (state->l1_mapping[l1idx].is_4mb())
    {
        kconsole << __FUNCTION__ << ": address " << va << " is mapped using a 4MB page!" << endl;
        return 0;
    }

    l2pa = state->l1_mapping[l1idx].frame();
    l2va = state->l2_virt + (l2pa - state->l2_phys);

    // XXX PARANOIA
    if (l2va != state->l1_virt[l1idx].frame())
        kconsole << "virtual addresses out of sync: l2va=" << l2va << ", not " << state->l1_virt[l1idx].frame() << endl;

    // Ok, once here, we have a pointer to our l2 table in "l2va"
    l2idx = pte_entry(va);

    // Update only flags and sid.
    flags_t flags = pte.flags();
    size_t i;

    for (i = 0; (i < n_pages) && ((i + l2idx) < N_L2_ENTRIES); ++i)
    {
        reinterpret_cast<page_t*>(l2va)[l2idx + i].set_flags(flags);

        // Setup shadow pte (holds sid + original global rights)
        SHADOW(l2va)[l2idx + i].sid = sid;  // store sid in shadow
        SHADOW(l2va)[l2idx + i].flags = flags; //??
    }

    return i;
}

/**
 * Add a page mapping.
 * va describes the corresponding virtual address.
 * pte contains appropriate physical address (if any) and flags.
 * FIXME: page_width is redundant and should be derived from pte!
 */
static bool add_page(mmu_v1::state_t* state, size_t page_width, address_t va, page_t pte, sid_t sid)
{
    bool result = false;
    switch (page_width)
    {
        case page_t::width_4kib:
            result = add4k_page(state, va, pte, sid);
            break;
        case page_t::width_4mib:
            // result = add4m_page(state, va, pte, sid);
            break;
        default:
            kconsole << __FUNCTION__ << ": unsupported page width " << page_width << endl;
    }
    return result;
}

static size_t update_pages(mmu_v1::state_t* state, size_t page_width, address_t va, size_t n_pages, page_t pte, sid_t sid)
{
    size_t result = 0;
    switch (page_width)
    {
        case page_t::width_4kib:
            result = update4k_pages(state, va, n_pages, pte, sid);
            break;
        case page_t::width_4mib:
            // result = update4m_pages(state, va, n_pages, pte, sid);
            break;
        default:
            kconsole << __FUNCTION__ << ": unsupported page width " << page_width << endl;
    }
    return result;
}

inline uint16_t alloc_pdidx(mmu_v1::state_t* state)
{
    uint32_t i = state->next_pdidx;
    kconsole << __FUNCTION__ << ": next_pdidx " << i << endl;
    do {
        if (state->pdom_tbl[i] == NULL)
        {
            state->next_pdidx = (i + 1) % PDIDX_MAX;
            kconsole << __FUNCTION__ << ": allocate next_pdidx " << i << endl;
            return i;
        }
        i = (i + 1) % PDIDX_MAX;
        kconsole << __FUNCTION__ << ": next_pdidx " << i << endl;
    } while(i != state->next_pdidx);

    kconsole << "alloc_pdidx: out of identifiers!" << endl;
    nucleus::debug_stop();
    return 0xdead;
}

static flags_t control_bits(mmu_v1::state_t* state, stretch_v1::rights rights, memory_v1::attr_flags attr, bool valid)
{
    flags_t flags = 0;

    if (!valid)
        flags |= page_t::swapped;
    if (!rights.has(stretch_v1::right_read))
        flags |= page_t::kernel_mode;
    if (rights.has(stretch_v1::right_execute))
        flags |= page_t::executable;
    if (rights.has(stretch_v1::right_write))
        flags |= page_t::writable;
    if (state->use_global_pages && rights.has(stretch_v1::right_global))
        flags |= page_t::global;

    /**
     * We use the combination of the bits 'no_cache' and 'non_memory' to
     * determine our caching policy as follows:
     *     if no_cache is set, then set the Cache Disable bit.
     *     if non_memory is set, but *not* no_cache, then set the Write Through bit.
     * This is a bit vile since the 'attr' things are all a bit hotch
     * potch. Essentially it just means that things like, e.g. frame
     * buffers, are updated correctly as long as their pmem attrs
     * include non_memory (but not no_cache).
     */
    if (attr.has(memory_v1::attrs_no_cache))
    {
        flags |= page_t::cache_disable;
    }
    else if (attr.has(memory_v1::attrs_non_memory))
    {
        flags |= page_t::write_through;
    }

    return flags;
}

inline bool valid_width(uint32_t width)
{
    return width == page_t::width_4kib || width == page_t::width_4mib;
}

//======================================================================================================================
// mmu_v1 methods
//======================================================================================================================

static void mmu_v1_start(mmu_v1::closure_t* self, protection_domain_v1::id root_domain)
{
    // nucleus::flush_tlb();
    // nucleus::wrpdom(base);
}

static void mmu_v1_add_range(mmu_v1::closure_t* self, stretch_v1::closure_t* str, memory_v1::virtmem_desc mem_range, stretch_v1::rights global_rights)
{
    page_t pte;
    flags_t flags = control_bits(self->d_state, global_rights, 0, /*valid:*/false);
    pte.set_flags(flags);

    size_t page_width = mem_range.page_width;

    if (!valid_width(page_width))
    {
        kconsole << __FUNCTION__ << ": unsupported page width " << page_width << endl;
        return;
    }

    address_t virt = mem_range.start_addr;
    size_t page_size = 1UL << page_width;

    for (size_t n_pages = 0; n_pages < mem_range.n_pages; ++n_pages)
    {
        if (!add_page(self->d_state, page_width, virt, pte, str->d_state->sid))
        {
            kconsole << __FUNCTION__ << ": failed to add page at " << virt << endl;
            return;
        }
        virt += page_size;
    }

    kconsole << __FUNCTION__ << ": added range [" << mem_range.start_addr << ".." << mem_range.start_addr + (mem_range.n_pages << page_width) << "), sid=" << str->d_state->sid << endl;
}

static void mmu_v1_add_mapped_range(mmu_v1::closure_t* self, stretch_v1::closure_t* str, memory_v1::virtmem_desc mem_range, memory_v1::physmem_desc pmem, stretch_v1::rights global_rights)
{
    size_t page_width = mem_range.page_width;

    if (!valid_width(page_width))
    {
        kconsole << __FUNCTION__ << ": unsupported page width " << page_width << endl;
        return;
    }

    size_t frame_width = pmem.frame_width;

    if (!valid_width(frame_width))
    {
        kconsole << __FUNCTION__ << ": unsupported frame width " << frame_width << endl;
        return;
    }

    // If page width differs from frame width, need to homogenise.
    // In reality, we should be adjusting to fixed page width - as this is what supported by MMU,
    // but since both widths have been verified for validity above, changing either should work.
    size_t n_pages, n_frames;

    if (frame_width > page_width)
    {
        page_width = frame_width; // use larger size
        n_pages = (mem_range.n_pages << mem_range.page_width) >> page_width;
        n_frames = pmem.n_frames;
    }
    else if (frame_width < page_width)
    {
        frame_width = page_width; // use larger size
        n_pages = mem_range.n_pages;
        n_frames = (pmem.n_frames << pmem.frame_width) >> frame_width;
    }
    else
    {
        n_frames = pmem.n_frames;
        n_pages = mem_range.n_pages;
    }

    if (n_frames != n_pages)
    {
        kconsole << __FUNCTION__ << ": number of pages " << n_pages << " and frames " << n_frames << " do not match!" << endl;
        nucleus::debug_stop();
        return;
    }

    address_t phys = pmem.start_addr;
    address_t virt = mem_range.start_addr;
    flags_t flags = control_bits(self->d_state, global_rights, pmem.attr, /*valid:*/true);

    size_t page_size = 1UL << page_width;
    page_t pte;
    pte.set_flags(flags);

    while (n_pages--)
    {
        size_t frame = phys >> FRAME_WIDTH;
        pte.set_frame(phys);

        uint32_t owner = OWNER_NONE;

        // Sanity check the ramtab
        if (frame < self->d_state->ramtab_size)
        {
            ramtab_v1::state state;

            owner = self->d_state->ramtab_closure.get(frame, &frame_width, &state);
            if (owner == OWNER_NONE)
            {
                kconsole << __FUNCTION__ << ": physical address " << phys << " not owned!" << endl;
                nucleus::debug_stop();
            }

            if (state == ramtab_v1::state_nailed)
            {
                kconsole << __FUNCTION__ << ": physical address " << phys << " is nailed!" << endl;
                nucleus::debug_stop();
            }
        }

        if (!add_page(self->d_state, page_width, virt, pte, str->d_state->sid))
        {
            kconsole << __FUNCTION__ << ": failed to add page at " << virt << endl;
            return;
        }

        // Update the ramtab
        if (frame < self->d_state->ramtab_size)
        {
            self->d_state->ramtab_closure.put(frame, owner, frame_width, ramtab_v1::state_mapped);
        }

        virt += page_size;
        phys += page_size;
    }

    kconsole << __FUNCTION__ << ": added mapped range [" << mem_range.start_addr << ".." << mem_range.start_addr + (mem_range.n_pages << mem_range.page_width) << ")=>[" << pmem.start_addr << ".." << pmem.start_addr + (pmem.n_frames << pmem.frame_width) << "), sid=" << str->d_state->sid << endl;
}

/**
 * Note: update cannot currently modify mappings, and expects that the virtual range contains valid PFNs already.
 */
static void mmu_v1_update_range(mmu_v1::closure_t* self, stretch_v1::closure_t* str, memory_v1::virtmem_desc mem_range, stretch_v1::rights global_rights)
{
    page_t pte;
    flags_t flags = control_bits(self->d_state, global_rights, 0, /*valid:*/true);
    pte.set_flags(flags);

    size_t page_width = mem_range.page_width;

    if (!valid_width(page_width))
    {
        kconsole << __FUNCTION__ << ": unsupported page width " << page_width << endl;
        return;
    }

    size_t page_size = 1UL << page_width;
    address_t virt = mem_range.start_addr;
    size_t n_pages = mem_range.n_pages;

    while (n_pages > 0)
    {
        size_t updated = update_pages(self->d_state, page_width, virt, n_pages, pte, str->d_state->sid);
        if (updated == 0)
        {
            kconsole << __FUNCTION__ << ": failed to update pages at " << virt << endl;
            nucleus::debug_stop();
            return;
        }
        virt += updated * page_size;
        n_pages -= updated;
    }

    kconsole << __FUNCTION__ << ": updated range [" << mem_range.start_addr << ".." << mem_range.start_addr + (mem_range.n_pages << page_width) << "), sid=" << str->d_state->sid << endl;
}

static void mmu_v1_free_range(mmu_v1::closure_t* self, memory_v1::virtmem_desc mem_range)
{

}

static protection_domain_v1::id mmu_v1_create_domain(mmu_v1::closure_t* self)
{
    auto state = self->d_state;

    uint16_t idx = alloc_pdidx(state);

    state->pdominfo[idx].refcnt = 0;
    state->pdominfo[idx].stretch = state->stretch_allocator->create(sizeof(pdom_t), stretch_v1::right_none);
    state->pdominfo[idx].gen++;

    memory_v1::size sz;
    pdom_t* base = reinterpret_cast<pdom_t*>(state->pdominfo[idx].stretch->info(&sz));
    memutils::clear_memory(base, sz);

    state->pdom_tbl[idx] = base;

    // Construct the pdid from the generation and the index.
    protection_domain_v1::id pdid = (uint32_t(state->pdominfo[idx].gen) << 16) | idx;
    kconsole << __FUNCTION__ << ": generated new pdid " << pdid << endl;
    return pdid;
}

static void mmu_v1_retain_domain(mmu_v1::closure_t* self, protection_domain_v1::id dom_id)
{
    auto state = self->d_state;
    uint16_t idx = PDIDX(dom_id);

    if ((idx >= PDIDX_MAX) || (state->pdom_tbl[idx] == NULL))
    {
        kconsole << __FUNCTION__ << ": bogus pdom id " << dom_id << endl;
        nucleus::debug_stop();
        return;
    }

    state->pdominfo[idx].refcnt++;
}

static void mmu_v1_release_domain(mmu_v1::closure_t* self, protection_domain_v1::id dom_id)
{
    auto state = self->d_state;
    uint16_t idx = PDIDX(dom_id);

    if ((idx >= PDIDX_MAX) || (state->pdom_tbl[idx] == NULL))
    {
        kconsole << __FUNCTION__ << ": bogus pdom id " << dom_id << endl;
        nucleus::debug_stop();
        return;
    }

    if (state->pdominfo[idx].refcnt)
        state->pdominfo[idx].refcnt--;

    if (state->pdominfo[idx].refcnt == 0)
    {
        state->stretch_allocator->destroy_stretch(state->pdominfo[idx].stretch);
        state->pdom_tbl[idx] = NULL;
    }
}

static void mmu_v1_set_rights(mmu_v1::closure_t* self, protection_domain_v1::id dom_id, stretch_v1::closure_t* str, stretch_v1::rights rights)
{
    auto state = self->d_state;
    uint16_t idx = PDIDX(dom_id);

    if ((idx >= PDIDX_MAX) || (state->pdom_tbl[idx] == NULL))
    {
        kconsole << __FUNCTION__ << ": bogus pdom id " << dom_id << endl;
        nucleus::debug_stop();
        return;
    }

    pdom_t* pdom = state->pdom_tbl[idx];
    sid_t sid = str->d_state->sid;

    kconsole << __FUNCTION__ << ": pdom " << pdom << ", sid " << sid << " " << rights << endl;

    uint8_t mask = sid & 1 ? 0xf0 : 0x0f;
    uint32_t val = rights;
    if (sid & 1) val <<= 4;
    pdom->rights[sid>>1] &= ~mask;
    pdom->rights[sid>>1] |= val;

    // Want to invalidate all non-global TB entries, but we can't
    // do that on Intel so just blow away the whole thing.
    // nucleus::flush_tlb();
}

static stretch_v1::rights mmu_v1_query_rights(mmu_v1::closure_t* self, protection_domain_v1::id dom_id, stretch_v1::closure_t* str)
{
    return stretch_v1::rights();
}

// No ASN supported on x86.
static int32_t mmu_v1_query_asn(mmu_v1::closure_t* self, protection_domain_v1::id dom_id)
{
    return 0x666;
}

static stretch_v1::rights mmu_v1_query_global_rights(mmu_v1::closure_t* self, stretch_v1::closure_t* str)
{
    return stretch_v1::rights();
}

static void mmu_v1_clone_rights(mmu_v1::closure_t* self, stretch_v1::closure_t* tmpl, stretch_v1::closure_t* str)
{

}

static const mmu_v1::ops_t mmu_v1_methods =
{
    mmu_v1_start,
    mmu_v1_add_range,
    mmu_v1_add_mapped_range,
    mmu_v1_update_range,
    mmu_v1_free_range,
    mmu_v1_create_domain,
    mmu_v1_retain_domain,
    mmu_v1_release_domain,
    mmu_v1_set_rights,
    mmu_v1_query_rights,
    mmu_v1_query_asn,
    mmu_v1_query_global_rights,
    mmu_v1_clone_rights
};

//======================================================================================================================
// ramtab_v1 methods
//======================================================================================================================

#if RAMTAB_DEBUG
#define RD(s) s
#else
#define RD(s)
#endif

static memory_v1::size ramtab_v1_size(ramtab_v1::closure_t* self)
{
    mmu_v1::state_t* st = reinterpret_cast<mmu_v1::state_t*>(self->d_state);
    RD(kconsole << __FUNCTION__ << ": ramtab state at " << st << ", returning size " << st->ramtab_size << endl);
    return st->ramtab_size;
}

static memory_v1::address ramtab_v1_base(ramtab_v1::closure_t* self)
{
    mmu_v1::state_t* st = reinterpret_cast<mmu_v1::state_t*>(self->d_state);
    RD(kconsole << __FUNCTION__ << ": ramtab state at " << st << ", returning base " << st->ramtab << endl);
    return reinterpret_cast<memory_v1::address>(st->ramtab);
}

static void ramtab_v1_put(ramtab_v1::closure_t* self, uint32_t frame, uint32_t owner, uint32_t frame_width, ramtab_v1::state state)
{
    mmu_v1::state_t* st = reinterpret_cast<mmu_v1::state_t*>(self->d_state);
    RD(kconsole << __FUNCTION__ << ": frame " << frame << " with owner " << owner << " and frame width " << int(frame_width) << " in state " << state << endl);
    if (frame >= st->ramtab_size)
    {
        kconsole << __FUNCTION__ << ": out of range frame " << frame << ", max is " << st->ramtab_size << endl;
        nucleus::debug_stop();
        return;
    }

    st->ramtab[frame].owner = owner;
    st->ramtab[frame].frame_width = frame_width;
    st->ramtab[frame].state = state;
}

static uint32_t ramtab_v1_get(ramtab_v1::closure_t* self, uint32_t frame, uint32_t* frame_width, ramtab_v1::state* state)
{
    mmu_v1::state_t* st = reinterpret_cast<mmu_v1::state_t*>(self->d_state);
    if (frame >= st->ramtab_size)
    {
        kconsole << __FUNCTION__ << ": out of range frame " << frame << ", max is " << st->ramtab_size << endl;
        nucleus::debug_stop();
        return 0xdeadd00d;
    }

    *frame_width = st->ramtab[frame].frame_width;
    *state = ramtab_v1::state(st->ramtab[frame].state);
    RD(kconsole << __FUNCTION__ << ": frame " << frame << " with owner " << st->ramtab[frame].owner << " and frame width " << int(*frame_width) << " in state " << *state << endl);
    return st->ramtab[frame].owner;
}

static const ramtab_v1::ops_t ramtab_v1_methods =
{
    ramtab_v1_size,
    ramtab_v1_base,
    ramtab_v1_put,
    ramtab_v1_get
};

#undef RD

//======================================================================================================================
// mmu_module_v1 methods
//======================================================================================================================

/*
** Compute how much space is required initially for page tables:
** We do this currently by cycling through all initial mappings
** and setting a bit in a 1024-bit bitmap if a particular 4Mb
** chunk of the virtual address space requires a L2 page table.
** We then add a fixed number to this to allow for L2 allocation
** before we get frames and stretch allocators.
*/
static int bitmap_bit(address_t va)
{
    return (va >> 22) & 0x1f; // Each bit in each word represents 4Mb => 5 bits at 22 bit offset.
}

static int bitmap_index(address_t va)
{
    return va >> 27; // Each array word represents 32*4Mb = 128Mb => 27 bits.
}

#define N_EXTRA_L2S     32                     // We require an additional margin of L2 ptabs.

static size_t memory_required(bootinfo_t* bi, size_t& n_l2_tables)
{
    uint32_t bitmap[32] = { 0 };
    size_t nptabs = 0;

    std::for_each(bi->vmap_begin(), bi->vmap_end(), [&bitmap](const memory_v1::mapping* e)
    {
        kconsole << "Virtual mapping [" << e->virt << ", " << e->virt + (e->nframes << FRAME_WIDTH) << ") -> [" << e->phys << ", " << e->phys + (e->nframes << FRAME_WIDTH) << ")" << endl;
        for (size_t j = 0; j < e->nframes; ++j)
        {
            address_t va = e->virt + (j << FRAME_WIDTH);
            bitmap[bitmap_index(va)] |= 1 << bitmap_bit(va);
        }
    });

    /* Now scan through the bitmap to determine the number of L2s reqd */
    for (int i = 0; i < 32; ++i)
    {
        while (bitmap[i])
        {
            if (bitmap[i] & 1)
                nptabs++;
            bitmap[i] >>= 1;
        }
    }

    nptabs += N_EXTRA_L2S;
    n_l2_tables = nptabs;

    size_t res = sizeof(mmu_v1::state_t);   /* state includes the level 1 page table */

    // Account for L2 infos
    res += n_l2_tables * sizeof(l2_info);

    kconsole << " +--Got nptabs " << nptabs << endl;

    return res;
}

/*
** Compute how much space is required for the ram table; this is a
** system wide table which contains ownership information (and other
** misc. stuff) for each frame of 'real' physical memory.
*/
static size_t ramtab_required(bootinfo_t* bi, size_t& max_ramtab_entries)
{
    max_ramtab_entries = bi->find_usable_physical_memory_top() / PAGE_SIZE;
    return max_ramtab_entries * sizeof(ramtab_entry_t);
}

static inline bool is_non_cacheable(uint32_t type)
{
    return type == multiboot_t::mmap_entry_t::reserved
        || type == multiboot_t::mmap_entry_t::acpi_reclaimable
        || type == multiboot_t::mmap_entry_t::acpi_nvs
        || type == multiboot_t::mmap_entry_t::bad_memory;
}

static void enter_mappings(mmu_v1::state_t* state)
{
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    std::for_each(bi->vmap_begin(), bi->vmap_end(), [bi, state](const memory_v1::mapping* e)
    {
        kconsole << "Virtual mapping [" << e->virt << ", " << e->virt + (e->nframes << FRAME_WIDTH) << ") -> [" << e->phys << ", " << e->phys + (e->nframes << FRAME_WIDTH) << ")" << endl;
        for (size_t j = 0; j < e->nframes; ++j)
        {
            address_t virt = e->virt + (j << FRAME_WIDTH);
            address_t phys = e->phys + (j << FRAME_WIDTH);

            uint32_t flags = page_t::writable;

            /* We assume the frames used by the established mappings
               are part of the image unless otherwise specified */

            /*
            ** Generally we want to cache things, but in the case
            ** of IO space we prefer not to.
            */
            std::for_each(bi->mmap_begin(), bi->mmap_end(), [phys, virt, &flags](const multiboot_t::mmap_entry_t* e)
            {
                if (is_non_cacheable(e->type()) && (e->address() <= phys) && (e->address() + e->size() > phys))
                {
                    V(kconsole << "Disabling cache for va=" << virt << endl);
                    flags |= page_t::cache_disable;
                }
            });

            /* We lock the L1 page table into the TLB */
            if(state->use_global_pages && (virt == reinterpret_cast<address_t>(&(state->l1_mapping))))
                flags |= page_t::global;

            page_t pte;
            pte = 0;
            pte.set_frame(phys);
            pte.set_flags(flags);

            if (!add4k_page(state, virt, pte, SID_NULL))
            {
                kconsole << "enter_mappings: failed to add mapping " << virt << "->" << phys << endl;
                PANIC("enter_mappings failed!");
            }

            state->ramtab_closure.put(phys_frame_number(phys), OWNER_SYSTEM, FRAME_WIDTH, ramtab_v1::state_mapped);
        }
    });

    kconsole << " +-mmu_module_v1: enter_mappings required total of " << state->l2_next << " new l2 tables." << endl;
}

static mmu_v1::closure_t*
mmu_module_v1_create(mmu_module_v1::closure_t* self, uint32_t initial_reservation, ramtab_v1::closure_t** ramtab, memory_v1::address* free)
{
    kconsole << "==========================" << endl
             << "   mmu_module_v1.create" << endl
             << "==========================" << endl;

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    size_t mmu_memory_needed_bytes = 0;

    // Calculate how much space is needed for the MMU structures.
    //    mmu_state,
    //    pagetables
    //    and ramtab
    size_t n_l2_tables = 0;
    size_t max_ramtab_entries = 0;
    size_t i;

    mmu_memory_needed_bytes = memory_required(bi, n_l2_tables);
    mmu_memory_needed_bytes = page_align_up(mmu_memory_needed_bytes);

    address_t ramtab_offset = mmu_memory_needed_bytes;

    mmu_memory_needed_bytes += ramtab_required(bi, max_ramtab_entries);
    mmu_memory_needed_bytes = page_align_up(mmu_memory_needed_bytes);

    address_t l2_tables_offset = mmu_memory_needed_bytes;

    mmu_memory_needed_bytes += n_l2_tables * L2SIZE; // page-aligned by definition

    mmu_memory_needed_bytes += initial_reservation;
    mmu_memory_needed_bytes = page_align_up(mmu_memory_needed_bytes);

    address_t first_range = bi->find_highmem_range_of_at_least(mmu_memory_needed_bytes);

    // Find proper location to start "allocating" from.
    first_range = page_align_up(first_range);

    if (!bi->use_memory(first_range, mmu_memory_needed_bytes, multiboot_t::mmap_entry_t::system_data))
    {
        PANIC("Unable to use memory for initial MMU setup!");
    }

    kconsole << " +-mmu_module_v1: state allocated at " << first_range << endl;

    mmu_v1::state_t *state = reinterpret_cast<mmu_v1::state_t*>(first_range);
    mmu_v1::closure_t *cl = &state->mmu_closure;
    closure_init(cl, &mmu_v1_methods, state);

    state->l1_mapping_virt = state->l1_mapping_phys = first_range;
    state->l1_virt_virt = reinterpret_cast<address_t>(&state->l1_virt);

    kconsole << " +-mmu_module_v1: L1 phys table at va=" << state->l1_mapping_virt << ", pa=" << state->l1_mapping_phys << ", virt table at va=" << state->l1_virt_virt << endl;

    // Initialise the physical mapping to fault everything, & virtual to 'no trans'.
    for(i = 0; i < N_L1_TABLES; i++)
    {
        state->l1_mapping[i] = 0;
        state->l1_virt[i] = 0;
        state->l1_shadows[i].sid = SID_NULL;
        state->l1_shadows[i].flags = 0;
    }

    // Initialise the ram table; it follows the state record immediately.
    state->ramtab_size = max_ramtab_entries;
    state->ramtab = reinterpret_cast<ramtab_entry_t*>(first_range + ramtab_offset);
    memutils::clear_memory(state->ramtab, state->ramtab_size * sizeof(ramtab_entry_t));

    closure_init(&state->ramtab_closure, &ramtab_v1_methods, reinterpret_cast<ramtab_v1::state_t*>(first_range));
    *ramtab = &state->ramtab_closure;

    kconsole << " +-mmu_module_v1: ramtab at " << state->ramtab << " with " << state->ramtab_size << " entries." << endl;

    // Initialise the protection domain tables
    state->next_pdidx = 0;
    for(i = 0; i < PDIDX_MAX; i++)
    {
        state->pdom_tbl[i] = NULL;
        state->pdominfo[i].refcnt  = 0;
        state->pdominfo[i].gen     = 0;
        state->pdominfo[i].stretch = NULL;
    }

    // And store a pointer to the pdom_tbl in the info page.
    INFO_PAGE.protection_domains = &(state->pdom_tbl);

    state->use_global_pages = (INFO_PAGE.cpu_features & X86_32_FEAT_PGE) != 0;

    // Intialise our closures, etc to NULL for now  // will be fixed by $Done later
    state->system_frame_allocator = NULL;
    state->heap = NULL;
    state->stretch_allocator = NULL;

    state->l2_virt  = page_align_up(first_range + l2_tables_offset);
    state->l2_phys  = page_align_up(state->l1_mapping_phys + l2_tables_offset);
    state->l2_max  = n_l2_tables;

    kconsole << " +-mmu_module_v1: " << static_cast<int>(state->l2_max) << " L2 tables at va=" << state->l2_virt << ", pa=" << state->l2_phys << endl;

    state->l2_next = 0;
    for(i = 0; i < state->l2_max; i++)
        state->info[i] = L2FREE;

    // Enter mappings for all the existing translations.
    enter_mappings(state); // this call uses mappings in bootinfo_page, so we need to set them up sooner or call enter_mappings() later, maybe in Done or Engage?

    // Swap over to our new page table!
    kconsole << " +-mmu_module_v1: setting pagetable to " << state->l1_mapping_virt << ", " << state->l1_mapping_phys << endl;
    nucleus::write_pdbr(state->l1_mapping_virt, state->l1_mapping_phys);
    kconsole << " +-mmu_module_v1: wrote new pdbr using syscall!" << endl;

    // And store some useful pointers in the PIP for user-level translation.
//    INFO_PAGE.l1_va  = st->va_l1;
//    INFO_PAGE.l2tab  = st->vtab_va;
    INFO_PAGE.mmu_ok = true;

    /* Sort out pointer to free space for caller */
    *free = first_range +  l2_tables_offset + n_l2_tables * L2SIZE;

    return cl;
}

static void mmu_module_v1_finish_init(mmu_module_v1::closure_t* self, mmu_v1::closure_t* mmu, frame_allocator_v1::closure_t* frames, heap_v1::closure_t* heap, stretch_allocator_v1::closure_t* sysalloc)
{
    /* We don't require much here; just squirrel away the closures */
    mmu->d_state->system_frame_allocator = frames;
    mmu->d_state->heap = heap;
    mmu->d_state->stretch_allocator = sysalloc;
}

static const mmu_module_v1::ops_t mmu_module_v1_methods =
{
    mmu_module_v1_create,
    mmu_module_v1_finish_init
};

static mmu_module_v1::closure_t clos =
{
    &mmu_module_v1_methods,
    NULL // no state
};

EXPORT_CLOSURE_TO_ROOTDOM(mmu_module, v1, clos);
