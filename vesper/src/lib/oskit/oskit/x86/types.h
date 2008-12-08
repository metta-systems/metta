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
 * Basic Flux OS Toolkit types for the x86 architecture.
 */
#ifndef	_OSKIT_X86_TYPES_H_
#define _OSKIT_X86_TYPES_H_

/*
 * On any anchitecture,
 * these types are _exactly_ as wide as indicated in their names.
 * The signed types essentially have the same meanings
 * as Java's byte, short, int, and long types, respectively.
 */
typedef signed char		oskit_s8_t;
typedef signed short int	oskit_s16_t;
typedef signed int		oskit_s32_t;
typedef signed long long	oskit_s64_t;
typedef unsigned char		oskit_u8_t;
typedef unsigned short int	oskit_u16_t;
typedef unsigned int		oskit_u32_t;
typedef unsigned long long	oskit_u64_t;
typedef float			oskit_f32_t;
typedef double			oskit_f64_t;

/*
 * Just yer ordinary boolean type.
 * On the x86, bytes are fast, so we use it;
 * on other architectures this will generally be an int.
 */
typedef unsigned char		oskit_bool_t;

/*
 * Type compatible with `va_list' from <stdarg.h>.
 */
typedef char *			oskit_va_list;

/*
 * A oskit_addr_t is a type-neutral pointer,
 * e.g. an offset into a virtual memory space.
 */
typedef	oskit_u32_t		oskit_addr_t;

/*
 * A oskit_size_t is the proper type for
 * expressing the difference between two
 * oskit_addr_t entities.
 */
typedef	oskit_u32_t		oskit_size_t;
typedef oskit_s32_t		oskit_ssize_t;

/*
 * Integer types the size of a general-purpose processor register.
 * Generally the same size as oskit_addr_t and oskit_size_t.
 */
typedef oskit_u32_t		oskit_reg_t;
typedef oskit_s32_t		oskit_sreg_t;


#endif	/* _OSKIT_X86_TYPES_H_ */
