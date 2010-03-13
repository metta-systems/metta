#include "protection_domain.h"

//! Allocate a stretch of virtual addresses of size @c size with optional start address @c base.
// Stretch is created in privileged space (thus not editable by application).
stretch_t* stretch_t::create(size_t size, access_t access, address_t base)
{
    stretch_t* stretch = new stretch_t;
    if (protection_domain_t::privileged().allocate_stretch(stretch, size, access, base))
        return stretch;
    delete stretch;
    return 0;
}

stretch_t::stretch_t()
    : address(0)
    , size(0)
    , access_rights(0)
    , stretch_driver(0)
{
}
