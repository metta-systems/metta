#pragma once

class root_domain_t : public component_t
{
public:
    root_domain_t(bootimage_t& img);
    address_t entry();
};
