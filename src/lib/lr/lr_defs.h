#ifndef __LR_DEFS_H
#define __LR_DEFS_H

/// \todo External dependency on <cassert>, if lrDEBUG is defined.
#ifdef lrDEBUG
#	include <cassert>
#endif

namespace lr {

/// Run-time assertion routine - is used only when lrDEBUG is defined.
#ifdef lrDEBUG
#	define lr_assert assert
#else
#	define lr_assert(C)
#endif

/// Disables copying of object of type specified by parameter C.
#define lrNO_COPY_CLASS(C) \
	private : C(const C &); C & operator=(const C&);

}

#endif
