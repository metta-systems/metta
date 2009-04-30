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
 * Modified for Metta by Berkus, 2008.12.08.
 * Basic COM (Component Object Model) definitions.
 * These definitions are not really binary-level compatible with Microsoft's,
 * since we don't use the GUIDs but use java-style interface specifications.
 */

#ifndef _OSKIT_COM_H_
#define _OSKIT_COM_H_

#include "types.h"
#include "string.h"
#include <oskit/types.h>
#include <oskit/compiler.h>
#include <oskit/error.h>

using metta::kernel::string;

/* COM interface identifiers */
typedef string oskit_iid_t;

/* COM interface return status code */
typedef int32_t status_t;

/* COM standard result code macros */
/* The type to use is status_t. */

/* Standard success codes */
#define OSKIT_S_OK      0

#define OSKIT_SUCCEEDED(err)        ((status_t)(err) >= OSKIT_S_OK)
#define OSKIT_FAILED(err)           ((status_t)(err) < OSKIT_S_OK)

#define OSKIT_ERROR_CODE(err)       ((err) & 0xffff)
#define OSKIT_ERROR_FACILITY(err)   (((err) >> 16) & 0x1fff) //bug
#define OSKIT_ERROR_SEVERITY(err)   (((err) >> 31) & 1)

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
#define OSKIT_COMCALL                       OSKIT_STDCALL
#define OSKIT_COMDECL       status_t        OSKIT_COMCALL
#define OSKIT_COMDECL_U     uint32_t        OSKIT_COMCALL
#define OSKIT_COMDECL_V     void            OSKIT_COMCALL

#define DECLARE_COMINTERFACE(name) struct name##_ops; struct name : public com_interface<name##_ops>


template <class ops_type>
struct com_interface
{
    ops_type *ops;

    inline OSKIT_COMDECL   query(const oskit_iid_t iid, void **out_ihandle)
    {
        return ops->query(this, iid, out_ihandle);
    }

    inline OSKIT_COMDECL_U ref()
    {
        return ops->ref(this);
    }

    inline OSKIT_COMDECL_U unref()
    {
        return ops->unref(this);
    }
};

/*
 * Definition of the com_iunknown interface (IUnknown in Microsofteze).
 */
DECLARE_COMINTERFACE(com_iunknown)
{
};

struct com_iunknown_ops
{
    /**
    * Query this object to find if it supports a particular interface,
    * and if so, obtain a reference to that interface.
    **/
    OSKIT_COMDECL   (*query)(com_iunknown *obj, const oskit_iid_t iid,
                             void **out_ihandle);

    /**
    * Increment the reference count for this interface.
    **/
    OSKIT_COMDECL_U (*ref)(com_iunknown *obj);

    /**
    * Decrement the reference count for this interface,
    * potentially freeing the interface/object if it drops to zero.
    **/
    OSKIT_COMDECL_U (*unref)(com_iunknown *obj);
};



// Parent interface
// struct Parent_v1 : public Trader_v1, public Environment_v1
// {
// }


///// TODO clean these up (reg/unreg/lookup goes to itrader interface of parent com object)

/*
 * Generic COM interface/object registration and lookup.
 * This does not itself need to be a COM object since there's only one.
 */
// oskit_error_t oskit_register(const oskit_iid_t iid, void *interface);
// oskit_error_t oskit_unregister(const oskit_iid_t iid, void *interface);
// oskit_error_t oskit_lookup(const oskit_iid_t iid, void ***out_interface_array);

/*
 * Call context query/manipulation functions,
 * primarily used to identify the principal for permission checking purposes.
 * Each thread can have an associated call context object;
 * these routines simply get and set the current thread's context object.
 * What interfaces the object supports (besides IUnknown) is undefined.
 */
// oskit_error_t oskit_get_call_context(const oskit_iid_t iid, void **out_if);
// oskit_error_t oskit_set_call_context(com_iunknown *context);

#endif /* _OSKIT_COM_H_ */

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
