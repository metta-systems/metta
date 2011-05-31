#include "algorithm"
#include "default_console.h"
#include "bootinfo.h"
#include "infopage.h"
#include "mmu_module_v1_interface.h"
#include "mmu_module_v1_impl.h"
#include "mmu_v1_interface.h"
#include "mmu_v1_impl.h"
#include "ramtab_v1_interface.h"
#include "cpu.h"
#include "page_directory.h"

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

struct mmu_v1_state
{
    page_t                l1_mapping[N_L1_TABLES]; /*!< Level 1 page directory      */
//    swpte_t               stab[N_L1ENTS];          /*!< Level 1 shadows (4Mb pages) */
    page_t                l1_virt[N_L1_TABLES];    /*!< Virtual addresses of L2s    */

    mmu_v1_closure        mmu_closure;
	ramtab_v1_closure     ramtab_closure;

    uint32_t              next_pdidx;          /* Next free pdom idx (hint) */
//    pdom_t               *pdom_tbl[PDIDX_MAX]; /* Map pdom idx to pdom_t's  */
//    pdom_st               pdominfo[PDIDX_MAX]; /* Map pdom idx to pdom_st's */

    bool                  use_global_pages;    /* Set iff we can use PGE    */

//	system_frames_allocator_v1_closure* frames_allocator;
//	heap_v1_closure*                    heap;
//	stretch_allocator_v1_closure*       stretch_allocator;

    uint32_t              nframes;

    address_t             l1_mapping_virt; /* virtual  address of l1 page table */
    address_t             l1_mapping_phys; /* physical address of l1 page table */

    address_t             va_l2;   /* virtual  address of l2 array base */
    address_t             pa_l2;   /* physical address of l2 array base */

    address_t             vtab_va; /* virtual address of l2 PtoV table  */

//	ramtab_t*             ramtab;      /* base of ram table            */
	size_t                ramtab_size; /* size of ram table            */

    uint32_t              maxl2;   /* index of the last available chunk   */
    uint32_t              nextl2;  /* index of first potential free chunk */
//    l2_info               info[1]; /* free/used l2 info; actually maxl2   */
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

    size_t res = sizeof(mmu_v1_state);   /* state includes the level 1 page table */

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

    kconsole << " +--Got nptabs " << nptabs << endl;

    return res + (nptabs * L2SIZE);
}

struct ramtab_entry_t
{};
	
/* 
** Compute how much space is required for the ram table; this is a 
** system wide table which contains ownership information (and other 
** misc. stuff) for each frame of 'real' physical memory.
*/
static size_t ramtab_required(bootinfo_t* bi, size_t& max_ramtab_entries)
{
	max_ramtab_entries = bi->find_top_memory_address() / PAGE_SIZE;
	return max_ramtab_entries * sizeof(ramtab_entry_t);
}

static mmu_v1_closure* mmu_module_v1_create(mmu_module_v1_closure* self, uint32_t initial_reservation, ramtab_v1_closure** ramtab, memory_v1_address* free)
{
    kconsole << " +-mmu_module_v1.create" << endl;

    // read the memory map from bootinfo page
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

	size_t mmu_memory_needed_bytes = 0;
	
    // Calculate how much space is needed for the MMU structures.
    //    mmu_state,
    //    pagetables
    //    and ramtab
	size_t n_l2_tables = 0;
	size_t maxpfn = 0;
	mmu_memory_needed_bytes = memory_required(bi, n_l2_tables);
	mmu_memory_needed_bytes = page_align_up(mmu_memory_needed_bytes);
	mmu_memory_needed_bytes += ramtab_required(bi, maxpfn);
	mmu_memory_needed_bytes += initial_reservation;

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
    
    state->ramtab_size = maxpfn;
//    state->ramtab_closure.methods = &ramtab_v1_method_table;
    state->ramtab_closure.state = reinterpret_cast<ramtab_v1_state*>(first_range);
    *ramtab = &state->ramtab_closure;

    state->use_global_pages = (INFO_PAGE.cpu_features & X86_32_FEAT_PGE) != 0;

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
