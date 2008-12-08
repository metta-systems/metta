/* oskit/config.h.  Generated automatically by configure.  */
/*
 * Copyright (c) 1996,1998,1999,2000 University of Utah and the Flux Group.
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
#ifndef _OSKIT_CONFIG_H_
#define _OSKIT_CONFIG_H_

/* Define to compile the OSKit for use with Knit.  */
/* #undef KNIT */

/* Define if your assembler supports the %cr4 register. */
#define HAVE_CR4 1

/* Define if your assembler supports the debug registers. */
#define HAVE_DEBUG_REGS 1

/* Define if your assembler supports the .p2align pseudo-op. */
#define HAVE_P2ALIGN 1

/* Define if your assembler supports the .code16 pseudo-op. */
#define HAVE_CODE16 1

/* Define if your assembler supports the .code16gcc pseudo-op. */
#define HAVE_CODE16GCC 1

/* Define to the prefix your assembler requires in .code16 mode
   before an instruction that uses a 32-bit address. */
#define ADDR32 addr32

/* Define to the prefix your assembler requires in .code16 mode
   before an instruction that uses a 32-bit datum.  */
#define DATA32 data32

/* Define if your assembler allows .space within .bss segments. */
#define HAVE_WORKING_BSS 1

/* Define if your compiler groks __attribute__((packed)) on structs. */
#define HAVE_PACKED_STRUCTS 1

/* Define if your compiler groks __attribute__((pure)). */
#define HAVE_PURE 1

/* Define if your compiler groks __attribute__((noreturn)). */
#define HAVE_NORETURN 1

/* Define if your compiler groks __attribute__((stdcall)). */
#define HAVE_STDCALL 1

/* Define if your compiler groks __attribute__((constructor)). */
#define HAVE_CONSTRUCTOR 1

#endif /* _OSKIT_CONFIG_H_ */
