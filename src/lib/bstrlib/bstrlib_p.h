#include <string.h>
// #include "globals.h"
// #include "memutils.h"
// #include "memory_manager.h"
// using namespace metta::common;
// using namespace metta::kernel;
// #define strlen  memutils::strlen
// #define strcmp  memutils::strcmp
// 
// #define bstr__alloc   malloc
// #define bstr__free    free
// #define bstr__realloc realloc
// 
// #define bstr__memcpy  memutils::copy_memory
// #define bstr__memmove memutils::move_memory
// #define bstr__memset  memutils::fill_memory
// #define bstr__memcmp  memutils::memcmp
// #define bstr__memchr  memutils::memchr

#define bstr__memcpy  memcpy
#define bstr__memmove memmove
#define bstr__memset  memset

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
