#include "component.h"

// How to call constructors from external entity?
// use this silly C wrapper? void boot();

class portal_manager : public component
{
public:
    portal_manager(com_iparent *p) : component(p) {}
};
