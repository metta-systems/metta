#pragma once

#if SYSTEM_DEBUG
#define D(s) s
#else
#define D(s)
#endif

#if SYSTEM_VERBOSE_DEBUG
#define V(s) s
#else
#define V(s)
#endif
