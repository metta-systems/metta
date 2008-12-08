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
 * Basic type definitions used in the OSKit interfaces
 */
#ifndef	_OSKIT_TYPES_H_
#define _OSKIT_TYPES_H_


/*
 * This file defines those specific-width types for this machine architecture,
 * and also specifies what width to use for several purpose-specific types.
 */
#include <oskit/machine/types.h>


/*
 * Types corresponding to standard POSIX types,
 * for use in the COM interfaces that export POSIX functionality.
 */
typedef oskit_u32_t	oskit_dev_t;	/* Device number */
typedef oskit_u32_t	oskit_ino_t;	/* File serial number */
typedef oskit_u16_t	oskit_nlink_t;	/* Link count of a file */
typedef oskit_s32_t	oskit_pid_t;	/* Process ID */
typedef oskit_u32_t	oskit_uid_t;	/* User ID */
typedef oskit_u32_t	oskit_gid_t;	/* Group ID */
typedef oskit_u16_t	oskit_mode_t;	/* File type and access permissions */
typedef oskit_s64_t	oskit_off_t;	/* File offset */
typedef oskit_u16_t	oskit_wchar_t;	/* Unicode wide character */

#endif /* _OSKIT_TYPES_H_ */
