#ifndef __LR_ALLOCATOR_H
#define __LR_ALLOCATOR_H

/// \todo External dependency on <stdlib.h>.
#include <stdlib.h>
/// \todo External dependency on <new>.
#include <new>

#include "types.h"
#include "memutils.h"

namespace lr {

/// Takes care of simple memory allocation and deallocation.
/**
 Uses realloc() and free() functions for memory allocation and deallocation.
 Does not invoke constructors for allocated objects.
 Does not invoke destructors when deallocating objects.
 Uses inplace new operator for placing object into allocated memory.
 Template parameter T - specifies type of object allocator operates on.
*/
template < class T >
struct lrAllocatorUnsafe {
	
	/// Allocates or reallocates memory region to specified size.
	static T * reallocate(T * memory,uint32 size) {
		return static_cast < T * > (::realloc(memory,static_cast < size_t > (size) * sizeof(T)));
	}
	
	/// Deallocates specified memory region.
	static void deallocate(T * begin,T * end) {
		::free(begin);
	}

	/// Copy-constructs object in specified memory initialiazing it to specified value.
	static void construct(T * memory,const T & value) {
		new(static_cast < void * > (memory))T(value);
	}

	/// Copy-constructs range of objects in specified memory initialiazing them to specified value.
	static void construct(T * begin,T * end,const T & value) {
		for (T * it=begin; it != end; it++)
			new(static_cast < void * > (it))T(value);
	}
	
	/// Copy-constructs range of objects in specified memory initialiazing them to specified range of values.
	static void construct(T * dstBegin,T * dstEnd,const T * srcBegin,const T * srcEnd) {
		lr_assert(srcBegin < srcEnd);
		lr_assert((srcEnd < dstBegin) || (srcBegin > dstEnd));
		lr_memcpy(dstBegin,srcBegin,(dstEnd - dstBegin) * sizeof(T));
	}
	
	/// Does nothing for lrAllocatorUnsafe.
	static void destruct(T *) {}
	
	/// Does nothing for lrAllocatorUnsafe.
	static void destruct(T *,T *) {}
};

/// Takes care of memory allocation and deallocation.
/**
 Uses realloc() and free() functions for memory allocation and deallocation.
 Does invoke constructors for allocated objects.
 Does invoke destructors when deallocating objects.
 Uses inplace new operator for placing object into allocated memory.
 Template parameter T - specifies type of object allocator operates on.
*/
template < class T >
struct lrAllocator {
	
	/// Allocates or reallocates memory region to specified size.
	static T * reallocate(T * memory,uint32 size) {
		return static_cast < T * > (::realloc(memory,static_cast < size_t > (size) * sizeof(T)));
	}
	
	/// Deallocates specified memory region invoking object destructors.
	static void deallocate(T * begin,T * end) {
		for (T * it=begin; it != end; it++)
			it->~T();
		::free(begin);
	}

	/// Copy-constructs object in specified memory initialiazing it to specified value.
	static void construct(T * memory,const T & value) {
		new(static_cast < void * > (memory))T(value);
	}

	/// Copy-constructs range of objects in specified memory initialiazing them to specified value.
	static void construct(T * begin,T * end,const T & value) {
		for (T * it=begin; it != end; it++)
			new(static_cast < void * > (it))T(value);
	}
	
	/// Copy-constructs range of objects in specified memory initialiazing them to specified range of values.
	static void construct(T * dstBegin,T * dstEnd,const T * srcBegin,const T * srcEnd) {
		for (T * it=dstBegin; it != dstEnd; it++) {
			lr_assert(srcBegin != srcEnd);
			new(static_cast < void * > (it))T(*srcBegin++);
			}
	}
	
	/// Invokes destructor for specified object.
	static void destruct(T * object) {
		object->~T();
	}
	
	/// Invokes destroctor for specified object sequence in memory.
	static void destruct(T * begin,T * end) {
		for (T * it=begin; it != end; it++)
			it->~T();
	}
};

}

#endif
