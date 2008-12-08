/*
 * Copyright (c) 1994, 1998, 1999 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

/*
 * This defines a lower-level memory allocator interface. It is intended
 * to be used by the malloc library and the osenv_mem functions, as well
 * as anyone else with special memory allocation requirements.
 */
#ifndef _OSKIT_COM_MEM_H_
#define _OSKIT_COM_MEM_H_

#include <oskit/types.h>
#include <oskit/com.h>
#include <oskit/error.h>
#include <oskit/dev/dev.h>

/*
 * The set of flags is currently identical to the those accepted by
 * the osenv_mem interface. This is probably unnecessary, as the
 * low level allocator would not be interested in some of these.
 * I am going to leave this as is for now, since it is easy to change.
 */
#define OSKIT_MEM_AUTO_SIZE	OSENV_AUTO_SIZE
#define OSKIT_MEM_PHYS_WIRED	OSENV_PHYS_WIRED
#define OSKIT_MEM_VIRT_EQ_PHYS  OSENV_VIRT_EQ_PHYS
#define OSKIT_MEM_PHYS_CONTIG   OSENV_PHYS_CONTIG
#define OSKIT_MEM_NONBLOCKING   OSENV_NONBLOCKING
#define OSKIT_MEM_ISADMA_MEM    OSENV_ISADMA_MEM
#define OSKIT_MEM_X861MB_MEM    OSENV_X861MB_MEM

/*
 * Lower-level (or common) memory allocator interface.
 * IID 4AA7DFE8-7C74-11CF-B500-08000953ADC2
 */
struct oskit_mem {
	struct oskit_mem_ops *ops;
};
typedef struct oskit_mem oskit_mem_t;

struct oskit_mem_ops {

	/*** COM-specified IUnknown interface operations ***/
	OSKIT_COMDECL_IUNKNOWN(oskit_mem_t)

	/*
	 * Allocate a chunk of memory of at least 'size' bytes, and return
         * * a pointer to the first byte in the chunk.  The allocation must
         * * satisfy any constraints specified by 'flags', or fail if the *
         * constraints cannot be satisfied. Returns null if there is not *
         * enough memory available. The initial contents of the memory
         * chunk are undefined.
	 */
	void *
	OSKIT_COMCALL	(*alloc)(oskit_mem_t *m, oskit_u32_t size,
	  			oskit_u32_t flags);

	/*
	 * Realloc a region. Flags must include OSKIT_MEM_AUTO_SIZE if the
	 * original allocation did. In that case, oldsize is ignored.
	 */
	void *
	OSKIT_COMCALL	(*realloc)(oskit_mem_t *m, void *ptr,
				oskit_u32_t oldsize, oskit_u32_t newsize,
				oskit_u32_t flags);

	/*
	 * Like alloc. Additionally, if 'align' is nonzero, it must be a
         * power of two, which indicates the required alignment for the
         * memory chunk.
         */
	void *
	OSKIT_COMCALL	(*alloc_aligned)(oskit_mem_t *m, oskit_u32_t size,
				oskit_u32_t flags, oskit_u32_t align);

	/*
	 * Deallocates the memory block pointed to by 'ptr'.
	 * If 'ptr' is NULL, this method harmlessly does nothing.
	 * The 'flags' parameter must include the OSKIT_MEM_AUTO_SIZE flag
	 * if and only if that flag was specified on the original allocation;
	 * if it is set, then the 'oldsize' parameter is ignored
	 * All other bits in the 'flags' parameter are ignored.
	 */
	void
	OSKIT_COMCALL	(*free)(oskit_mem_t *m, void *ptr,
	  			oskit_u32_t size, oskit_u32_t flags);

	/*
	 * Find the size of a previously allocated memory block
	 * that was allocated with the OSKIT_MEM_AUTO_SIZE option flag.
	 * The returned size may be greater than the original size requested,
	 * because of rounding done to satisfy alignment constraints.
	 * If 'ptr' is NULL, returns (oskit_u32_t)-1.
	 */
	oskit_u32_t
	OSKIT_COMCALL	(*getsize)(oskit_mem_t *m, void *ptr);

	/*
	 * A lower level interface intended to provide the same 
	 * flexibility as the LMM alloc_gen interface. It might be
	 * that the underlying memory allocator is not as spiffy as the
	 * LMM, and will fall back to doing something dopey. 
	 */
	void *
	OSKIT_COMCALL	(*alloc_gen)(oskit_mem_t *m, oskit_u32_t size,
				oskit_u32_t flags, oskit_u32_t align_bits,
				oskit_u32_t align_ofs);

	/*
	 * Return the amount of free space in the memory object pool.
	 * If flags is non-zero, it should be a memory type flag, which
	 * indicates that the return value should be the amount of free
	 * space of that type.
	 */
	oskit_size_t
	OSKIT_COMCALL	(*avail)(oskit_mem_t *m, oskit_u32_t flags);

	/*
	 * A debugging hook. If appropriate, print out some information
	 * about the current state of the memory object.
	 */
	OSKIT_COMDECL_V (*dump)(oskit_mem_t *m);
	
};

#define oskit_mem_query(m, iid, out_ihandle) \
	((m)->ops->query((oskit_mem_t *)(m), (iid), (out_ihandle)))
#define oskit_mem_addref(m) \
	((m)->ops->addref((oskit_mem_t *)(m)))
#define oskit_mem_release(m) \
	((m)->ops->release((oskit_mem_t *)(m)))
#define oskit_mem_alloc(m, size, flags) \
	((m)->ops->alloc((oskit_mem_t *)(m), (size), (flags)))
#define oskit_mem_realloc(m, ptr, olds, news, flags) \
	((m)->ops->realloc((oskit_mem_t *)(m), (ptr), (olds), (news), (flags)))
#define oskit_mem_alloc_aligned(m, size, flags, align) \
	((m)->ops->alloc_aligned((oskit_mem_t *)(m), (size), (flags), (align)))
#define oskit_mem_free(m, ptr, size, flags) \
	((m)->ops->free((oskit_mem_t *)(m), (ptr), (size), (flags)))
#define oskit_mem_getsize(m, ptr) \
	((m)->ops->getsize((oskit_mem_t *)(m), (ptr)))
#define oskit_mem_alloc_gen(m, size, flags, bits, ofs) \
	((m)->ops->alloc_gen((oskit_mem_t *)(m), (size), (flags), \
			     (bits), (ofs)))
#define oskit_mem_avail(m, flags) \
	((m)->ops->avail((oskit_mem_t *)(m), (flags)))
#define oskit_mem_dump(m) \
	((m)->ops->dump((oskit_mem_t *)(m)))

/* GUID for oskit_mem interface */
extern const struct oskit_guid oskit_mem_iid;
#define OSKIT_MEM_IID OSKIT_GUID(0x4aa7dfe8, 0x7c74, 0x11cf, \
		0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

/*
 * Initialize the memory interface. The appmem initializer is a
 * convenience function that allows the default implementation of the
 * oskit memory object to be used in Fluke userland applications.
 * It pulls in an empty malloc_lmm, initializes it for simple memory
 * allocation, and the calls oskit_mem_init() (which operates on the
 * malloc_lmm). 
 */
oskit_mem_t *oskit_mem_init(void);
oskit_mem_t *oskit_appmem_init(void *base, oskit_size_t size);

/*
 * The memory object will call this function when there is a memory
 * shortage. It should try to scare up more memory, and return a
 * pointer to that memory so that the memory object can add it to its
 * pool. Returns zero if no more memory can be found.
 *
 * Flags are OSKIT_MEM flags above, and can be used to specialize the
 * required memory.
 */
void *oskit_mem_morecore(oskit_size_t size, int flags);

#endif /* _OSKIT_COM_MEM_H_ */
