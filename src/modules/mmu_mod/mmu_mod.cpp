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
#include "cpu.h"
#include "page_directory.h"
#include "system_frame_allocator_v1_interface.h"
#include "heap_v1_interface.h"

//======================================================================================================================
// ramtab_v1 methods
//======================================================================================================================

static void ramtab_v1_put(ramtab_v1_closure* self, uint32_t pfn, uint32_t owner, uint32_t fwidth, ramtab_v1_state_e st)
{
    //kconsole << " +-ramtab_v1: put " << pfn << " with owner " << owner << " and frame width " << fwidth << " in state " << st << endl;
}

static const ramtab_v1_ops ramtab_v1_method_table = {
    NULL,
    NULL,
    ramtab_v1_put,
    NULL
};

//======================================================================================================================
// mmu_v1 methods
//======================================================================================================================

static const mmu_v1_ops mmu_v1_method_table = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const size_t N_L1_TABLES = 1024;

/* definitions for owner field of ramtab */
#define OWNER_NONE    0x0     /* pfn is unused by anyone        */
#define OWNER_SYSTEM  0x1     /* pfn is owned by us (mmgmt etc) */

struct ramtab_entry_t
{
    address_t owner;        /* PHYSICAL address of owning domain's DCB   */
    uint16_t frame_width;   /* Logical width of the frame                */
    uint16_t state;         /* Misc bits, e.g. is_mapped, is_nailed, etc */
} PACKED;

struct pdom_st 
{
    uint16_t      refcnt;  /* reference count on this pdom    */
    uint16_t      gen;     /* current generation of this pdom */
    stretch_v1_closure*   stretch; /* handle on stretch (for destroy) */ 
};

/*
** Protection domains are implemented as arrays of 4-bit elements, 
** indexed by stretch id.
*/
typedef uint16_t  sid_t;
#define SID_NULL  0xFFFF
#define SID_MAX   16384

struct pdom_t
{
    uint8_t rights[SID_MAX/2];
};

struct shadow_t
{
    sid_t sid;
    uint16_t ctl;
} PACKED;

#define SHADOW(_va)  reinterpret_cast<shadow_t*>(reinterpret_cast<char*>(_va) + 4*KiB)


typedef uint8_t     l2_info;    /* free or used info for 1K L2 page tables */
#define L2FREE      (l2_info)0x12
#define L2USED      (l2_info)0x99

#define PDIDX_MAX       0x80   /* Allow up to 128 protection domains */

struct mmu_v1_state
{
    page_t                l1_mapping[N_L1_TABLES]; /*!< Level 1 page directory      */
    shadow_t              l1_shadows[N_L1_TABLES]; /*!< Level 1 shadows (4Mb pages) */
    page_t                l1_virt[N_L1_TABLES];    /*!< Virtual addresses of L2s    */

    mmu_v1_closure        mmu_closure;
    ramtab_v1_closure     ramtab_closure;

    uint32_t              next_pdidx;          /* Next free pdom idx (hint) */
    pdom_t*               pdom_tbl[PDIDX_MAX]; /* Map pdom idx to pdom_t's  */
    pdom_st               pdominfo[PDIDX_MAX]; /* Map pdom idx to pdom_st's */

    bool                  use_global_pages;    /* Set iff we can use PGE    */

    system_frame_allocator_v1_closure*  system_frame_allocator;
    heap_v1_closure*                    heap;
//  stretch_allocator_v1_closure*       stretch_allocator;

    uint32_t              n_frames;

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

#define L2SIZE          (8*KiB)                // 4K for L2 pagetable + 4K for shadow(?)
#define N_EXTRA_L2S     32                     /* We require an additional margin of L2 ptabs */

static size_t memory_required(bootinfo_t* bi, size_t& n_l2_tables)
{
    uint32_t bitmap[32] = { 0 };
    size_t nptabs = 0;

    std::for_each(bi->vmap_begin(), bi->vmap_end(), [&bitmap](const memory_v1_mapping* e)
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

    size_t res = sizeof(mmu_v1_state);   /* state includes the level 1 page table */

    // Account for L2 infos
    res += n_l2_tables * sizeof(l2_info);

    kconsole << " +--Got nptabs " << nptabs << endl;

    return res;// + (n_l2_tables * L2SIZE);
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

inline bool alloc_l2table(mmu_v1_state* state, address_t *l2va, address_t *l2pa)
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

static bool add4k_page(mmu_v1_state* state, address_t va, page_t pte, sid_t sid)
{
    int l1idx, l2idx;
    address_t   l2va, l2pa;

    l1idx  = pde_entry(va);

    if (!state->l1_mapping[l1idx].is_present())
    {
        kconsole << "mapping va=" << va << " requires new L2 table" << endl;
	    if(!alloc_l2table(state, &l2va, &l2pa)) {
            kconsole << "!!! intel_mmu:add4k_page - cannot alloc l2 table." << endl;
	        return false;
	    }
        state->l1_mapping[l1idx].set_frame(l2pa);
        state->l1_mapping[l1idx].set_flags(page_t::writable|page_t::write_through);
	    state->l1_virt[l1idx].set_frame(l2va);
    }

    if(state->l1_mapping[l1idx].is_4mb()) 
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
//    swpte->ctl.all = ((hwpte_t *)&pte)->bits & 0xFF;
    return true;
}

static void enter_mappings(mmu_v1_state* state)
{
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
    std::for_each(bi->vmap_begin(), bi->vmap_end(), [bi, state](const memory_v1_mapping* e)
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
    	        if ((e->type() != multiboot_t::mmap_entry_t::free) && (e->address() <= phys) && (e->address() + e->size() > phys))
    		    {
    		        //kconsole << "Disabling cache for va=" << virt << endl;
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

            state->ramtab_closure.put(phys >> FRAME_WIDTH, OWNER_SYSTEM, FRAME_WIDTH, ramtab_v1_state_e_mapped);
		}
    });

    kconsole << " +-mmu_module_v1: enter_mappings required total of " << state->l2_next << " new l2 tables." << endl;
}

static mmu_v1_closure* mmu_module_v1_create(mmu_module_v1_closure* self, uint32_t initial_reservation, ramtab_v1_closure** ramtab, memory_v1_address* free)
{
    kconsole << " +-mmu_module_v1.create" << endl;
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

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

	if (!bi->use_memory(first_range, mmu_memory_needed_bytes))
	{
        PANIC("Unable to use memory for initial MMU setup!");
	}

	kconsole << " +-mmu_module_v1: state allocated at " << first_range << endl;

    // Recursion Unleashed!
	mmu_v1_state *state = reinterpret_cast<mmu_v1_state*>(first_range);
	mmu_v1_closure *cl = &state->mmu_closure;
	cl->methods = &mmu_v1_method_table;
	cl->state = state;
	
    state->l1_mapping_virt = state->l1_mapping_phys = first_range;
    state->l1_virt_virt = reinterpret_cast<address_t>(&state->l1_virt);

    kconsole << " +-mmu_module_v1: L1 phys table at va=" << state->l1_mapping_virt << ", pa=" << state->l1_mapping_phys << ", virt table at va=" << state->l1_virt_virt << endl;

    // Initialise the physical mapping to fault everything, & virtual to 'no trans'.
    for(i = 0; i < N_L1_TABLES; i++) 
    {
	    state->l1_mapping[i] = 0;
	    state->l1_virt[i] = 0;
	    state->l1_shadows[i].sid = SID_NULL;
	    state->l1_shadows[i].ctl = 0; 
    }

    // Initialise the ram table; it follows the state record immediately.
    state->ramtab_size = max_ramtab_entries;
    state->ramtab = reinterpret_cast<ramtab_entry_t*>(first_range + ramtab_offset);
    memutils::clear_memory(state->ramtab, state->ramtab_size * sizeof(ramtab_entry_t));

    state->ramtab_closure.methods = &ramtab_v1_method_table;
    state->ramtab_closure.state = reinterpret_cast<ramtab_v1_state*>(first_range);
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
    state->system_frame_allocator    = NULL;
    state->heap      = NULL;
    //state->stretch_allocator  = NULL;

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
//    MTRC(eprintf("MMUMod: setting new ptbr to va=%p, pa=%p\n", 
//	    st->va_l1, st->pa_l1));
    ia32_mmu_t::set_active_pagetable(state->l1_mapping_phys);
//    ntsc_wptbr(st->va_l1, st->pa_l1, st->vtab_va); //PDBR syscall
// nucleus_write_pdbr(); <<-- proposed syscalls format
//    MTRC(eprintf("+++ done new ptbr.\n"));

    // And store some useful pointers in the PIP for user-level translation.
//    INFO_PAGE.l1_va  = st->va_l1; 
//    INFO_PAGE.l2tab  = st->vtab_va; 
    INFO_PAGE.mmu_ok = true; 

    /* Sort out pointer to free space for caller */
    *free = first_range +  l2_tables_offset + n_l2_tables * L2SIZE;

    return cl;
}

static const mmu_module_v1_ops mmu_module_v1_method_table = {
    mmu_module_v1_create,
};

static const mmu_module_v1_closure clos = {
    &mmu_module_v1_method_table,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(mmu_module_v1, mmu_module, clos);
