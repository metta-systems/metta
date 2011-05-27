#include "default_console.h"
#include "bootinfo.h"
#include "mmu_module_v1_interface.h"
#include "mmu_module_v1_impl.h"
#include "mmu_v1_interface.h"
#include "algorithm"
#include "mmu_v1_impl.h"

// TODO: We need to abstract frames module from the format of bootinfo page,
// so we add a type for memory_map and make it hide the fact that it uses the bootinfo_page
// we pass the memory_map type to frames_mod.

/// use bootinfo_t::mmap_begin/end for now, but probably bootinfo_t should return memory_map_t in request for memmap?

/*class memory_map_t
{
public:
	memory_map_t();
	memory_map_t(bootinfo_t* bi); // this hides bootinfo behind mmap type

	// memory item returned by the iterator
	class entry_t
	{
	public:
		bool is_free();
		physical_address_t start();
		size_t size();
	};

	class memory_map_iterator_t : public std::iterator<std::forward_iterator_tag, memory_map_t::memory_map_entry_t>
	{
	public:
		memory_map_iterator_t();
		memory_map_t::entry_t operator *();
    	void operator ++();
    	void operator ++(int);
		inline bool operator == (const memory_map_iterator_t& other) { return ptr == other.ptr; }
    	inline bool operator != (const memory_map_iterator_t& other) { return ptr != other.ptr; }
	};

	typedef memory_map_iterator_t iterator;
	iterator begin(); // see bootinfo_t::mmap_begin()
	iterator begin() const;
	iterator end();
	iterator end() const;
	iterator rbegin();
	iterator rbegin() const;
	iterator rend();
	iterator rend() const;
};*/

//
// mmu methods
//

static const mmu_v1_ops mmu_v1_method_table = {
    NULL
};

static const size_t N_L1_TABLES = 1024;

struct mmu_v1_state
{
/*	page_t l1_mapping[N_L1_TABLES];
*/
	mmu_v1_closure mmu_closure;
/*	ramtab_v1_closure ramtab_closure;

	system_frames_allocator_v1_closure* frames_allocator;
	heap_v1_closure* heap;
	stretch_allocator_v1_closure* stretch_allocator;

	ramtab_t* ramtab;
	size_t ramtab_size;*/
};

//
// mmu_mod methods
//

/* 
** Compute how much space is required initially for page tables: 
** We do this currently by cycling through all initial mappings
** and setting a bit in a 1024-bit bitmap if a particular 4Mb 
** chunk of the virtual address space requires a L2 page table. 
** We then add a fixed number to this to allow for L2 allocation 
** before we get frames and stretch allocators. 
*/
static size_t memory_required(bootinfo_t* bi, size_t& n_l2_tables)
{
	#if 0
    word_t bitmap[32];
    word_t curva, res, nptabs; 
    int i, j;

    res = sizeof(MMU_st);   /* state includes the level 1 page table */
    for(i = 0; i < 32; i++) 
	bitmap[i] = 0;

    for(i = 0; mmap[i].nframes; i++) {
		MTRC(eprintf("mem_reqd: got memmap [%p,%p) -> [%p,%p), %d frames\n", 
			mmap[i].vaddr, 
			mmap[0].vaddr + (mmap[0].nframes << FRAME_WIDTH),//[0]oops?
			mmap[i].paddr, 
			mmap[0].paddr + (mmap[0].nframes << FRAME_WIDTH),//[0]oops?
			mmap[i].nframes));
		for(j=0; j < mmap[i].nframes; j++) {
	    	curva = (word_t)(mmap[i].vaddr + (j << FRAME_WIDTH));
	    	bitmap[BITMAP_IDX(curva)] |= 1 << BITMAP_BIT(curva);
		}
    }

    /* Now scan through the bitmap to determine the number of L2s reqd */
    nptabs = 0;
    for(i=0; i < 32; i++) {
		while(bitmap[i]) {
	    	if(bitmap[i] & 1) 
				nptabs++;
	    	bitmap[i] >>= 1; 
		}
    }

    MTRC(eprintf("Got ntpabs=%d, extra=%d => total is %d\n", 
	    nptabs, N_EXTRA_L2S, nptabs + N_EXTRA_L2S));
    nptabs += N_EXTRA_L2S;
    *nl2tables = nptabs; 
    return (res + (nptabs * L2SIZE));
#endif
	return 0;
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

static mmu_v1_closure* mmu_mod_create(mmu_module_v1_closure* self, int initial_reservation/*, ramtab& ramtab, address_t& free*/)
{
    UNUSED(self);
    UNUSED(initial_reservation);

    kconsole << "MMU mod : create" << endl;

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
	kconsole << "mmu_mod: state allocated at " << first_range << endl;

	if (!bi->use_memory(first_range, first_range + mmu_memory_needed_bytes))
	{
		
	}

	mmu_v1_state *state = reinterpret_cast<mmu_v1_state*>(first_range);
	mmu_v1_closure *cl = &state->mmu_closure;
	cl->methods = &mmu_v1_method_table;
	cl->state = state;

    return cl;
}

static const mmu_module_v1_ops ops = {
    mmu_mod_create
};

static const mmu_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(mmu_module_v1, mmu_module, clos);
