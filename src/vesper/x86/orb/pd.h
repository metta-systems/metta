#pragma once

namespace nucleus
{

//! Protection Domain
/*!
Protection domain governs an address space and threads that execute in this space.
*/
class pd_t
{
    pd_t();
private:
    portal_table portals;
    mapping_t mappings;
    region_t regions;
    security_id_t sec_id;
};

} // namespace nucleus
