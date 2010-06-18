#pragma once

class component_t
{
};

class root_domain_t : public component_t
{
public:
    root_domain_t(bootimage_t& img);
    address_t entry();

private:
    address_t start;
    size_t size;
};
