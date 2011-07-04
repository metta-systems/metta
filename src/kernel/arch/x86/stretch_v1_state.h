#pragma once

#include "memory_v1_interface.h"
#include "stretch_v1_interface.h"
#include "doubly_linked_list.h"
#include "domain.h"

struct stretch_allocator_v1_closure;
struct mmu_v1_closure;

/*!
 * State record for a stretch
 * We expose it for the benefit of the MMU code.
 * Would prefer not to, but reckon it's better than exposing SIDs in IDL...
 */
struct stretch_v1_state : public dl_link_t<stretch_v1_state>
{
    stretch_allocator_v1_closure* allocator;
    mmu_v1_closure*               mmu;
    stretch_v1_closure            closure;
    sid_t sid;
    
    memory_v1_address base;
    memory_v1_size    size;
    
    stretch_v1_rights global_rights;
};

const uint32_t stretch_v1_right_none = 0;
