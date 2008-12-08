/*
 * Copyright (c) 1997-1999 University of Utah and the Flux Group.
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
 * Declarations for some simple interface wrappers provided by -loskit_com.
 */

#ifndef _OSKIT_COM_WRAPPER_H
#define _OSKIT_COM_WRAPPER_H

struct oskit_socket;
struct oskit_stream;
struct oskit_ttystream;
struct oskit_asyncio;
struct oskit_posixio;
struct oskit_file;
struct oskit_dir;
struct oskit_filesystem;
struct oskit_openfile;
struct oskit_blkio;
struct oskit_absio;
struct oskit_dirents;

/*
 * oskit_wrap_~
 *
 * Wrap a ~ object such that `before' is invoked before every call
 * to the wrapped object (`in').  Similarly, `after' is invoked after
 * every call.
 */
oskit_error_t
oskit_wrap_socket(struct oskit_socket *in, 
	void (*before)(void *), 
	void (*after)(void *),
	void *cookie,
	struct oskit_socket **out);

oskit_error_t
oskit_wrap_stream(struct oskit_stream *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_stream **out);

oskit_error_t
oskit_wrap_ttystream(struct oskit_ttystream *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_ttystream **out);

oskit_error_t
oskit_wrap_asyncio(struct oskit_asyncio *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_asyncio **out);

/* 
 * wrap a combo of oskit_socket, oskit_stream, oskit_asyncio, and
 * oskit_bufio_stream - called "sockio"
 */
oskit_error_t
oskit_wrap_sockio(struct oskit_socket *in, 
	void (*before)(void *), 
	void (*after)(void *),
	void *cookie,
	struct oskit_socket **out);

oskit_error_t
oskit_wrap_posixio(struct oskit_posixio *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_posixio **out);

oskit_error_t
oskit_wrap_file(struct oskit_file *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_file **out);

oskit_error_t
oskit_wrap_dir(struct oskit_dir *in,
        void (*before)(void *),
        void (*after)(void *),
	void *cookie,
        struct oskit_dir **out);

oskit_error_t
oskit_wrap_filesystem(struct oskit_filesystem *in,
	void (*before)(),
	void (*after)(),
	void *cookie,
	struct oskit_filesystem **out);

oskit_error_t
oskit_wrap_openfile(struct oskit_openfile *in,
	void (*before)(),
	void (*after)(),
	void *cookie,
	struct oskit_openfile **out);

oskit_error_t
oskit_wrap_openfile_with_file(struct oskit_openfile *in,
	struct oskit_file *file,
	void (*before)(),
	void (*after)(),
	void *cookie,
	struct oskit_openfile **out);

oskit_error_t
oskit_wrap_blkio(struct oskit_blkio *in, 
	void (*before)(void *), 
	void (*after)(void *),
	void *cookie,
	struct oskit_blkio **out);

oskit_error_t
oskit_wrap_absio(struct oskit_absio *in, 
	void (*before)(void *), 
	void (*after)(void *),
	void *cookie,
	struct oskit_absio **out);

oskit_error_t
oskit_wrap_dirents(struct oskit_dirents *in, 
	void (*before)(void *), 
	void (*after)(void *),
	void *cookie,
	struct oskit_dirents **out);

/* #define DEBUGWRAPPERS */

#ifdef	DEBUGWRAPPERS
#define DEBUGWRAPPER(iface, call, err) \
	printf("oskit_" #iface "_" #call " returns 0x%x\n", err);
#else
#define DEBUGWRAPPER(iface, call, err)
#endif

#endif /* _OSKIT_COM_WRAPPER_H */
