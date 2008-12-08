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
 * Definitions of heavily compiler-specific things.
 */
#ifndef _OSKIT_COMPILER_H_
#define _OSKIT_COMPILER_H_

#include <oskit/config.h>

/*
 * All function prototypes should be surrounded by these macros,
 * so that a C++ compiler will know that they're C functions.
 */
#ifdef __cplusplus
#define OSKIT_BEGIN_DECLS extern "C" {
#define OSKIT_END_DECLS }
#else
#define OSKIT_BEGIN_DECLS
#define OSKIT_END_DECLS
#endif

/*
 * XXX this is a GCC-ism that can't easily be worked around at the moment.
 * Would probably be best just to avoid using this if possible.
 */
#ifndef OSKIT_INLINE
#define OSKIT_INLINE	extern __inline
#endif

/*
 * Other handy attributes that are specific to GCC,
 * and are only in _some_ versions of GCC.
 */
#ifdef HAVE_PURE
#define OSKIT_PURE	__attribute__((__const__))
#else
#define OSKIT_PURE
#endif

#ifdef HAVE_NORETURN
#define OSKIT_NORETURN	__attribute__((__noreturn__))
#else
#define OSKIT_NORETURN
#endif

#ifdef HAVE_STDCALL
#define OSKIT_STDCALL	__attribute__((__stdcall__))
#else
#define OSKIT_STDCALL
#endif

#endif /* _OSKIT_COMPILER_H_ */
