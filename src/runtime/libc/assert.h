/* Fake C runtime */
#include "panic.h"

#define assert(b) ((b) ? (void)0 : panic_assert(#b, __FILE__, __LINE__))
