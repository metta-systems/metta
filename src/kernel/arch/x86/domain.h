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
struct flink_t
{
    flink_t* next;
    flink_t* prev;
};

struct dcb_rw_t;
struct ramtab_entry_t; // defined by mmu_mod
     
struct dcb_ro_t
{
    dcb_rw_t* rw;
    uint32_t min_phys_frame_count;
    uint32_t max_phys_frame_count;
    ramtab_entry_t* ramtab;
    flink_t memory_region_list;
};

struct dcb_rw_t
{
    dcb_ro_t* ro;
};

