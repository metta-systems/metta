/*
DCB is taken mostly verbatim from Nemesis, here's the original diagram:

 Structure of a Nemesis DCB:

---->+------------------------------------------------+	0x0000
     |                                                |
     |  dcb_ro: read-only to the user domain.         |
     |                                                |
     +------------------------------------------------+
     |  VP closure                                    |
     +------------------------------------------------+
     |            <padding>                           |
     +------------------------------------------------+ 0x2000 (page) XXX
     |                                                |
     |  dcb_rw: read/writeable by user domain         |
     |                                                |
     +------------------------------------------------+
     |                                                |
     |  Array of context slots                        |
     |                                                |
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     |                                                |
     +------------------------------------------------+
*/

// Just a doubly linked list, plain and simple.
struct dl_link_t
{
    dl_link_t* next;
    dl_link_t* prev;
    
    inline void init()
    {
        next = prev = this;
    }
    
    inline bool is_empty()
    {
        return next == this;
    }
    
    inline void remove()
    {
        next->prev = prev;
        prev->next = next;
    }
    
    /*!
     * Insert another before this.
     */
    inline void insert_before(dl_link_t* another)
    {
        another->next = this;
        another->prev = prev;
        prev->next = another;
        prev = another;
    }
    
    /*!
     * Insert another after this.
     */
    inline void insert_after(dl_link_t* another)
    {
        another->next = next;
        another->prev = this;
        next->prev = another;
        next = another;
    }
/*    
    inline dl_link_t* dequeue()
    {
        if (is_empty())
            return NULL;
        next->remove();
        return next;
    }
*/
};

struct region_list_t : public dl_link_t
{
    address_t  start;         /* Start of physical memory region         */
    size_t     n_phys_frames; /* No of *physical* frames it extends      */
    size_t     frame_width;   /* Logical frame width within region       */
};

struct dcb_rw_t;
struct ramtab_entry_t; // defined by mmu_mod
     
struct dcb_ro_t
{
    dcb_rw_t* rw;
    uint32_t min_phys_frame_count;
    uint32_t max_phys_frame_count;
    ramtab_entry_t* ramtab;
    dl_link_t memory_region_list;
};

struct dcb_rw_t
{
    dcb_ro_t* ro;
};

