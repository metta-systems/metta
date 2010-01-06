#pragma once

#include "types.h"

/*! Memory allocation resource record, per-domain. */
struct memory_resrec_t
{
    size_t allocated_frames;
    size_t guaranteed_frames;
    size_t optimistic_frames;
};

/*!
 * Frame allocator is a system domain privileged keeper of physical memory frames.
 */
class frame_allocator_t
{
public:
    enum /*page_constraints*/ {
        continuous = 1 << 0,
        non_ram    = 1 << 1,

        below_1Mb  = 1 << 8,
        below_16Mb = 2 << 9,
    };

    static frame_allocator_t& instance();

    inline static const size_t page_size() { return PAGE_SIZE; }
    inline static const size_t page_mask() { return ~(PAGE_SIZE-1); }

    /*!
     * Allocate a non-constrained page frame. These kinds of pages can be used to map
     * normal memory into a virtual address space.
     * @return physical address of the page or 0 if no page available.
     */
    virtual physical_address_t allocate_frame() = 0;

    /*!
     * Free a page allocated with the allocate_frame() function.
     * @param[in] frame physical address of the page frame.
     */
    virtual void free_frame(physical_address_t frame) = 0;

    /*! Allocate a memory range with specific constraints the pages need to fulfill.
     * @param[in] range reference to the MemoryRegion object
     * @param[in] num_frames the number of pages to allocate for the MemoryRegion object
     * @param[in] page_constraints the constraints the pages have to fullfill
     * @param[in] flags flags from the VirtualAddressSpace class namespace
     * @param[in] start the physical address of the beginning of the region (optional)
     * @return true, if a valid MemoryRegion object is created, false otherwise */
    virtual bool allocate_range(memory_range_t& range,
                                size_t num_frames,
                                flags_t page_constraints,
                                flags_t flags,
                                physical_address_t start = -1) = 0;

    virtual bool free_range(memory_range_t& range) = 0;

    /*! Structure containing information about one memory range. */
    struct memory_range_t
    {
        /*!
         * @param[in] virtual_address virtual address of the beginning of the memory region
         * @param[in] physical_address physical address of the beginning of the memory region (or 0)
         * @param[in] size_ size (in bytes) of the memory region
         * @param[in] name_ user-visible name of the memory region
         */
        inline memory_range_t(void* virtual_address,
                              physical_address_t physical_address,
                              size_t size_,
                              const char* name_)
            : virtual_address(virtual_address)
            , physical_address(physical_address)
            , size(size_)
            , name(name_)
        {}

        /*! Virtual address of the memory region */
        void* virtual_address;
        /*! Physical address of the memory region (or 0) */
        physical_address_t physical_address;
        /*! Size (in bytes) of the memory region */
        size_t size;
        /*! Pointer to the user-visible name of the memory region */
        const char* name;
    };

    // Questionable API items:
    /*!
     * Frees the previously allocated frame 'frame' and returns it to the pool.
     * If @e virt is non-zero, remove mapping associated with address @e virt in frame allocator's pagedir.
     */
//     virtual void free_frame(address_t frame, address_t virt = 0) = 0;

    /*!
     * Finds a free frame (swaps out if necessary) and allocates it to p.
     */
//     void alloc_frame(page_t* p, bool is_kernel = true, bool is_writeable = true);

    /*!
     * Removes the frame under p's control and returns it to the pool.
     */
//     void free_frame(page_t* p);

protected:
    /** The constructor */
    inline frame_allocator_t() {}
    /** The destructor */
    inline virtual ~frame_allocator_t() {}

private:
    /*!
     * Disable the copy constructor.
     * @note NOT implemented
     */
    frame_allocator_t(const frame_allocator_t&);
    /*!
     * Disable assignment operator.
     * @note Not implemented
     */
    frame_allocator_t& operator =(const frame_allocator_t&);

    /*! Unmaps a memory region - called ONLY from MemoryRegion's destructor. */
//     virtual void unmap_range(memory_range_t* range) = 0;
};
