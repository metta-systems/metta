#pragma once

#include "types.h"

#define LINKSYM(LS) ((address_t)&LS)

// defined by linker
extern address_t image_end;

extern address_t K_SPACE_START;
extern address_t K_HEAP_START;
extern address_t K_HEAP_END;
extern address_t K_STACK_START;
extern address_t K_STACK_END;
