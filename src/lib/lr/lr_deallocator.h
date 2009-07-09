#ifndef __LR_DEALLOCATOR_H
#define __LR_DEALLOCATOR_H

/// \todo External dependency on <cstdlib>.
#include <cstdlib>

namespace lr {

/// Releases allocated memory.
/**
 Delete operator is used for releasing allocated memory.
*/
struct lrDeallocator {

	/// Deallocates memory specified by pointer using delete operator.
	template < class Type >
	static void deallocate(Type * ptr) {
		delete ptr;
	}
};

/// Releases allocated memory.
/**
 Function free() is used for releasing allocated memory.
*/
struct lrDeallocatorUnsafe {

	/// Deallocates memory specified by pointer using free function.
	template < class Type >
	static void deallocate(Type * ptr) {
		::free(ptr);
	}
};

}

#endif
