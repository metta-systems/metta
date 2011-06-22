#pragma once

#include "heap_v1_interface.h"

void* operator new(size_t size, heap_v1_closure* heap) throw();
void* operator new[](size_t size, heap_v1_closure* heap) throw();
void operator delete(void* p, heap_v1_closure* heap) throw();
void operator delete[](void* p, heap_v1_closure* heap) throw();
