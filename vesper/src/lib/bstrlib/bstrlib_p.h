#include "globals.h"
#include "memutils.h"
#include "memory_manager.h"
using namespace metta::common;
using namespace metta::kernel;
#define strlen  memutils::strlen
#define strcmp  memutils::strcmp

#define bstr__alloc   malloc
#define bstr__free    free
#define bstr__realloc realloc

#define bstr__memcpy  memutils::copy_memory
#define bstr__memmove memutils::move_memory
#define bstr__memset  memutils::fill_memory
#define bstr__memcmp  memutils::memcmp
#define bstr__memchr  memutils::memchr
