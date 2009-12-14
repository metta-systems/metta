
class physical_memory_manager_t
{
public:
    virtual size_t page_size() const;
    virtual address_t page_mask() const;
};

class x86_pmm_t : public physical_memory_manager_t
{
public:
    virtual size_t page_size() const { return 4096; }
    virtual address_t page_mask() const { return ~(page_size()-1); }
};
