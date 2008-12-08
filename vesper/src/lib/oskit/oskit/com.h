/*
 * Copyright (c) 1997-1998 University of Utah and the Flux Group.
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
 * Basic COM (Component Object Model) definitions.
 * These definitions are binary-level compatible with Microsoft's,
 * but not name-level compatible;
 * i.e., we do not use Microsoft's icky naming conventions.
 * Also, note that Microsoft's basic COM header file is called <objbase.h>,
 * whereas ours is called <oskit/com.h>.
 * If this ever becomes a problem,
 * we can easily define one to be a glue front-end to the other.
 * However, this should generally not be an issue,
 * because COM concentrates on binary-level compatibility, not source-level.
 */
/* XXX Check that these definitions are consistent with Microsoft's,
   particularly with respect to signedness of integer values. */
#ifndef _OSKIT_COM_H_
#define _OSKIT_COM_H_

#include <oskit/types.h>
#include <oskit/compiler.h>
#include <oskit/error.h>

OSKIT_BEGIN_DECLS

#if 0
/*
 * File time stamp structure; corresponds to FILETIME in Win32.
 * XXX document exactly the meaning of the fields.
 */
struct oskit_filetime {
	oskit_u32_t	low;
	oskit_u32_t	high;
};
typedef struct oskit_filetime oskit_filetime_t;
#endif


/* COM/DCE globally unique identifiers (GUIDs, or UUIDs in DCE-speak) */
struct oskit_guid {
	oskit_u32_t	data1;
	oskit_u16_t	data2;
	oskit_u16_t	data3;
	oskit_u8_t	data4[8];
};
typedef struct oskit_guid oskit_guid_t;

/* This macro produces a structure initializer for oskit_guid_t. */
#define OSKIT_GUID(l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
	{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }


/* COM interface identifiers (a particular application of GUIDs) */
typedef oskit_guid_t oskit_iid_t;


/* COM standard result code macros */
/* The type to use is oskit_error_t defined in <oskit/error.h>. */

#define OSKIT_SUCCEEDED(err)		((oskit_error_t)(err) >= 0)
#define OSKIT_FAILED(err)		((oskit_error_t)(err) < 0)

#define OSKIT_ERROR_CODE(err)		((err) & 0xffff)
#define OSKIT_ERROR_FACILITY(err)	(((err) >> 16) & 0x1fff)
#define OSKIT_ERROR_SEVERITY(err)	(((err) >> 31) & 1)

#define OSKIT_MAKE_ERROR(severity, facility, code)	\
	(((oskit_error_t)(severity) << 31) |		\
	 ((oskit_error_t)(facility) << 16) |		\
	 (oskit_error_t)(code))

#define OSKIT_SEVERITY_SUCCESS	0
#define OSKIT_SEVERITY_ERROR	1

#define OSKIT_FACILITY_NULL	0
#define OSKIT_FACILITY_RPC	1
#define OSKIT_FACILITY_ITF	4

/* Facility code we've stolen for global OSKIT error codes */
#define OSKIT_FACILITY_OSKIT	0x0f10

/* Standard success codes */
#define OSKIT_S_OK		0
#define OSKIT_S_TRUE		0
#define OSKIT_S_FALSE		1


/*
 * COM interfaces use the Pascal-like "stdcall" calling conventions,
 * in order to ensure binary compability across all compilers.
 * The OSKIT_COMCALL macro corresponds to Microsoft's WINAPI macro;
 * it is based on the compiler-specific OSKIT_STDCALL macro
 * defined in <oskit/compiler.h>.
 * Note that although GCC allows it to appear either
 * just after the return type or after the whole function prototype,
 * other x86 compilers such as Watcom require it after the return type,
 * so for compatibility we should put it there.
 */
#define OSKIT_COMCALL				OSKIT_STDCALL
#define OSKIT_COMDECL		oskit_error_t	OSKIT_COMCALL
#define OSKIT_COMDECL_U		oskit_u32_t	OSKIT_COMCALL
#define OSKIT_COMDECL_V		void		OSKIT_COMCALL

/*
 * Definition of the oskit_iunknown interface (IUnknown in Microsofteze).
 */
struct oskit_iunknown
{
	struct oskit_iunknown_ops *ops;
};
typedef struct oskit_iunknown oskit_iunknown_t;

struct oskit_iunknown_ops
{
	/*
	 * Query this object to find if it supports a particular interface,
	 * and if so, obtain a reference to that interface.
	 */
	OSKIT_COMDECL	(*query)(oskit_iunknown_t *obj, const oskit_iid_t *iid,
				 void **out_ihandle);

	/*
	 * Increment the reference count for this interface.
	 */
	OSKIT_COMDECL_U	(*addref)(oskit_iunknown_t *obj);

	/*
	 * Decrement the reference count for this interface,
	 * potentially freeing the interface/object if it drops to zero.
	 */
	OSKIT_COMDECL_U	(*release)(oskit_iunknown_t *obj);
};

/*
 * use this macro to insert declarations for the three methods 
 * common to all interfaces
 */
#define OSKIT_COMDECL_IUNKNOWN(type)					\
	OSKIT_COMDECL	(*query)(type *, const oskit_iid_t *, void **);	\
	OSKIT_COMDECL_U	(*addref)(type *);				\
	OSKIT_COMDECL_U	(*release)(type *);				

extern const struct oskit_guid oskit_iunknown_iid;
#define OSKIT_IUNKNOWN_IID OSKIT_GUID(0x00000000, 0x0000, 0x0000, \
		0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)

#define oskit_iunknown_query(obj, iid, out_ihandle) \
	((obj)->ops->query((oskit_iunknown_t *)(obj), (iid), (out_ihandle)))
#define oskit_iunknown_addref(obj) \
	((obj)->ops->addref((oskit_iunknown_t *)(obj)))
#define oskit_iunknown_release(obj) \
	((obj)->ops->release((oskit_iunknown_t *)(obj)))

/*
 * Generic COM interface/object registration and lookup.
 * This does not itself need to be a COM object since there's only one.
 */
oskit_error_t oskit_register(const struct oskit_guid *iid, void *interface);
oskit_error_t oskit_unregister(const struct oskit_guid *iid, void *interface);
oskit_error_t oskit_lookup(const oskit_guid_t *iid, void ***out_interface_array);

/*
 * Call context query/manipulation functions,
 * primarily used to identify the principal for permission checking purposes.
 * Each thread can have an associated call context object;
 * these routines simply get and set the current thread's context object.
 * What interfaces the object supports (besides IUnknown) is undefined.
 */
oskit_error_t oskit_get_call_context(const struct oskit_guid *iid, void **out_if);
oskit_error_t oskit_set_call_context(oskit_iunknown_t *context);

OSKIT_END_DECLS

#endif /* _OSKIT_COM_H_ */
