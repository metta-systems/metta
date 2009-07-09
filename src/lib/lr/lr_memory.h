#ifndef __LR_MEMORY_H
#define __LR_MEMORY_H

/// \todo External dependency on <string.h>.
#include <string.h>

namespace lr {

/// Sets memory region to specified value.
inline void lr_memset(void * dest,int value,size_t size) {
	::memset(dest,value,size);
}

/// Copies memory region to specified destination - memory regions must not overlap.
inline void * lr_memcpy(void * dest,const void * src,size_t size) {
	return ::memcpy(dest,src,size);
}

/// Copies memory region to specified destination.
inline void * lr_memmove(void * dest,const void * src,size_t size) {
	return ::memmove(dest,src,size);
}

}

#endif
