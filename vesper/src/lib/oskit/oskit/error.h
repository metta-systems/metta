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
 * Definition of the common error return codes used by the OSKit interfaces,
 * which mostly correspond to error codes defined in existing standards.
 * For error codes that correspond directly to basic COM error codes,
 * the standard COM values are used.
 *
 * NOTE: look also in <oskit/dev/error.h> for other possible error codes you may
 * be getting.
 */
#ifndef _OSKIT_ERROR_H_
#define _OSKIT_ERROR_H_

#ifndef ASSEMBLER
#include <oskit/machine/types.h>

typedef oskit_s32_t oskit_error_t;
#endif

/* Standard COM error codes */
#define OSKIT_E_UNEXPECTED	0x8000ffff	/* Unexpected error */
#define OSKIT_E_NOTIMPL		0x80004001	/* Not implemented */
#define OSKIT_E_NOINTERFACE	0x80004002	/* Interface not supported */
#define OSKIT_E_POINTER		0x80004003	/* Bad pointer */
#define OSKIT_E_ABORT		0x80004004	/* Operation aborted */
#define OSKIT_E_FAIL		0x80004005	/* Operation failed */
#define OSKIT_E_ACCESSDENIED	0x80070005	/* Access denied */
#define OSKIT_E_OUTOFMEMORY	0x8007000e	/* Out of memory */
#define OSKIT_E_INVALIDARG	0x80070057	/* Invalid argument */

/* ISO/ANSI C-1990 errors */
#define OSKIT_EDOM		0x8f100001	/* Argument out of domain */
#define OSKIT_ERANGE		0x8f100002	/* Result too large */

/* POSIX-1990 errors */
#define OSKIT_E2BIG		0x8f100101	/* Argument list too long */
#define OSKIT_EACCES		0x8f100102	/* Permission denied */
#define OSKIT_EAGAIN		0x8f100103	/* Rsrc temporarily unavail */
#define OSKIT_EBADF		0x8f100104	/* Bad file descriptor */
#define OSKIT_EBUSY		0x8f100105	/* Device busy */
#define OSKIT_ECHILD		0x8f100106	/* No child processes */
#define OSKIT_EDEADLK		0x8f100107	/* Resource deadlock avoided */
#define OSKIT_EEXIST		0x8f100108	/* File exists */
#define OSKIT_EFAULT		OSKIT_E_POINTER	/* Bad address */
#define OSKIT_EFBIG		0x8f100109	/* File too large */
#define OSKIT_EINTR		0x8f10010a	/* Interrupted system call */
#define OSKIT_EINVAL		OSKIT_E_INVALIDARG /* Invalid argument */
#define OSKIT_EIO		0x8f10010b	/* Input/output error */
#define OSKIT_EISDIR		0x8f10010c	/* Is a directory */
#define OSKIT_EMFILE		0x8f10010d	/* Too many open files */
#define OSKIT_EMLINK		0x8f10010e	/* Too many links */
#define OSKIT_ENAMETOOLONG	0x8f10010f	/* File name too long */
#define OSKIT_ENFILE		0x8f100110	/* Max files open in system */
#define OSKIT_ENODEV		0x8f100111	/* Op not supported by device */
#define OSKIT_ENOENT		0x8f100112	/* No such file or directory */
#define OSKIT_ENOEXEC		0x8f100113	/* Exec format error */
#define OSKIT_ENOLCK		0x8f100114	/* No locks available */
#define OSKIT_ENOMEM		OSKIT_E_OUTOFMEMORY /* Cannot allocate memory */
#define OSKIT_ENOSPC		0x8f100115	/* No space left on device */
#define OSKIT_ENOSYS		OSKIT_E_NOTIMPL	/* Function not implemented */
#define OSKIT_ENOTDIR		0x8f100116	/* Not a directory */
#define OSKIT_ENOTEMPTY		0x8f100117	/* Directory not empty */
#define OSKIT_ENOTTY		0x8f100118	/* Inappropriate ioctl */
#define OSKIT_ENXIO		0x8f100119	/* Device not configured */
#define OSKIT_EPERM		OSKIT_E_ACCESSDENIED /* Op not permitted */
#define OSKIT_EPIPE		0x8f10011a	/* Broken pipe */
#define OSKIT_EROFS		0x8f10011b	/* Read-only file system */
#define OSKIT_ESPIPE		0x8f10011c	/* Illegal seek */
#define OSKIT_ESRCH		0x8f10011d	/* No such process */
#define OSKIT_EXDEV		0x8f10011e	/* Cross-device link */

/* POSIX-1993 errors */
#define OSKIT_EBADMSG		0x8f100120	/* Bad message */
#define OSKIT_ECANCELED		0x8f100121	/* Operation canceled */
#define OSKIT_EINPROGRESS	0x8f100122	/* Operation in progress */
#define OSKIT_EMSGSIZE		0x8f100123	/* Bad message buffer length */
#define OSKIT_ENOTSUP		0x8f100124	/* Not supported */

/* POSIX-1996 errors */
#define OSKIT_ETIMEDOUT		0x8f100130	/* Operation timed out */

/* X/Open UNIX 1994 errors */
#define OSKIT_EADDRINUSE		0x8f100200	/* Address in use */
#define OSKIT_EADDRNOTAVAIL	0x8f100201	/* Address not available */
#define OSKIT_EAFNOSUPPORT	0x8f100202	/* Address family unsupported */
#define OSKIT_EALREADY		0x8f100203	/* Already connected */
#define OSKIT_ECONNABORTED	0x8f100204	/* Connection aborted */
#define OSKIT_ECONNREFUSED	0x8f100205	/* Connection refused */
#define OSKIT_ECONNRESET		0x8f100206	/* Connection reset */
#define OSKIT_EDESTADDRREQ	0x8f100207	/* Dest address required */
#define OSKIT_EDQUOT		0x8f100208	/* Reserved */
#define OSKIT_EHOSTUNREACH	0x8f100209	/* Host is unreachable */
#define OSKIT_EIDRM		0x8f10020a	/* Identifier removed */
#define OSKIT_EILSEQ		0x8f10020b	/* Illegal byte sequence */
#define OSKIT_EISCONN		0x8f10020c	/* Connection in progress */
#define OSKIT_ELOOP		0x8f10020d	/* Too many symbolic links */
#define OSKIT_EMULTIHOP		0x8f10020e	/* Reserved */
#define OSKIT_ENETDOWN		0x8f10020f	/* Network is down */
#define OSKIT_ENETUNREACH	0x8f100210	/* Network unreachable */
#define OSKIT_ENOBUFS		0x8f100211	/* No buffer space available */
#define OSKIT_ENODATA		0x8f100212	/* No message is available */
#define OSKIT_ENOLINK		0x8f100213	/* Reserved */
#define OSKIT_ENOMSG		0x8f100214	/* No message of desired type */
#define OSKIT_ENOPROTOOPT	0x8f100215	/* Protocol not available */
#define OSKIT_ENOSR		0x8f100216	/* No STREAM resources */
#define OSKIT_ENOSTR		0x8f100217	/* Not a STREAM */
#define OSKIT_ENOTCONN		0x8f100218	/* Socket not connected */
#define OSKIT_ENOTSOCK		0x8f100219	/* Not a socket */
#define OSKIT_EOPNOTSUPP		0x8f10021a	/* Op not supported on socket */
#define OSKIT_EOVERFLOW		0x8f10021b	/* Value too large */
#define OSKIT_EPROTO		0x8f10021c	/* Protocol error */
#define OSKIT_EPROTONOSUPPORT	0x8f10021d	/* Protocol not supported */
#define OSKIT_EPROTOTYPE		0x8f10021e	/* Socket type not supported */
#define OSKIT_ESTALE		0x8f10021f	/* Reserved */
#define OSKIT_ETIME		0x8f100220	/* Stream ioctl timeout */
#define OSKIT_ETXTBSY		0x8f100221	/* Text file busy */
#define OSKIT_EWOULDBLOCK	0x8f100222	/* Operation would block */

#endif /* _OSKIT_ERROR_H_ */
