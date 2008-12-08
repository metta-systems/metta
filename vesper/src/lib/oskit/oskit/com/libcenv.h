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
 * The libc environment object. Used by the C/POSIX library to gather
 * up required external interfaces from the client OS. 
 */
#ifndef _OSKIT_COM_LIBCENV_H_
#define _OSKIT_COM_LIBCENV_H_

#include <oskit/com.h>
#include <oskit/error.h>
#include <oskit/com/stream.h>
#include <oskit/io/ttystream.h>
#include <oskit/fs/fsnamespace.h>
#include <oskit/dev/dev.h>

struct oskit_timespec;

/*
 * Libc environment object.
 * IID 4aa7dfe9-7c74-11cf-b500-08000953adc2
 */
struct oskit_libcenv {
	struct oskit_libcenv_ops *ops;
};
typedef struct oskit_libcenv oskit_libcenv_t;

struct oskit_libcenv_ops {

	/*** COM-specified IUnknown interface operations ***/
	OSKIT_COMDECL_IUNKNOWN(oskit_libcenv_t)

	/*** Operations specific to oskit_libcenv_t ***/

	/*
	 * Set/Get the root directory for the filesystem. Setting the
	 * the rootdir clears out the old one and releases it. Set it to
	 * NULL for the final release when tearing down the object.
	 */
	OSKIT_COMDECL	(*getfsnamespace)(oskit_libcenv_t *s,
				oskit_fsnamespace_t **out_dir);
	OSKIT_COMDECL	(*setfsnamespace)(oskit_libcenv_t *s,
				oskit_fsnamespace_t *dir);

	/*
	 * Set/Get the hostname.
	 */
	OSKIT_COMDECL	(*gethostname)(oskit_libcenv_t *s,
				char *hostname, int len);
	OSKIT_COMDECL	(*sethostname)(oskit_libcenv_t *s,
				const char *hostname, int len);

	/*
	 * Call the exit function. Set the exit function.
	 */
	OSKIT_COMDECL_V	(*exit)(oskit_libcenv_t *s, oskit_u32_t exitval);
	OSKIT_COMDECL	(*setexit)(oskit_libcenv_t *s, void (*exitfunc)(int));

	/*
	 * Set/Get the console object (stdio interface).
	 */
	OSKIT_COMDECL	(*getconsole)(oskit_libcenv_t *s,
				oskit_ttystream_t **out_ttystream);
	OSKIT_COMDECL	(*setconsole)(oskit_libcenv_t *s,
				oskit_ttystream_t *ttystream);

	/*
	 * Initialize the library for taking signals. This hooks up the
	 * connection between the C library and the kernel library.
	 */
	OSKIT_COMDECL	(*signals_init)(oskit_libcenv_t *s,
				int (*func)(int, int, void *));
	OSKIT_COMDECL	(*setsiginit)(oskit_libcenv_t *s,
				void (*sigfunc)(int (*func)(int,int,void *)));

	/*
	 * Sleep/wakeup interface. This is how the C/POSIX library puts
	 * itself to sleep.
	 */
	OSKIT_COMDECL_V (*sleep_init)(oskit_libcenv_t *s,
				osenv_sleeprec_t *sleeprec);
	OSKIT_COMDECL_U (*sleep)(oskit_libcenv_t *s,
				osenv_sleeprec_t *sleeprec,
				struct oskit_timespec *timeout);
	OSKIT_COMDECL_V (*wakeup)(oskit_libcenv_t *s,
				osenv_sleeprec_t *sleeprec);
	
	/*
	 * Clone an entire thing.
	 */
	OSKIT_COMDECL	(*clone)(oskit_libcenv_t *s, oskit_libcenv_t **intf);
};

/* GUID for oskit_libcenv interface */
extern const struct oskit_guid oskit_libcenv_iid;
#define OSKIT_LIBCENV_IID OSKIT_GUID(0x4aa7dfe9, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define oskit_libcenv_query(s, iid, out_ihandle) \
	((s)->ops->query((oskit_libcenv_t *)(s), (iid), (out_ihandle)))
#define oskit_libcenv_addref(s) \
	((s)->ops->addref((oskit_libcenv_t *)(s)))
#define oskit_libcenv_release(s) \
	((s)->ops->release((oskit_libcenv_t *)(s)))

#define oskit_libcenv_getfsnamespace(s, intf) \
	((s)->ops->getfsnamespace((oskit_libcenv_t *)(s), (intf)))
#define oskit_libcenv_setfsnamespace(s, intf) \
	((s)->ops->setfsnamespace((oskit_libcenv_t *)(s), (intf)))
#define oskit_libcenv_gethostname(s, name, len) \
	((s)->ops->gethostname((oskit_libcenv_t *)(s), (name), (len)))
#define oskit_libcenv_sethostname(s, name, len) \
	((s)->ops->sethostname((oskit_libcenv_t *)(s), (name), (len)))
#define oskit_libcenv_exit(s, val) \
	((s)->ops->exit((oskit_libcenv_t *)(s), (val)))
#define oskit_libcenv_setexit(s, func) \
	((s)->ops->setexit((oskit_libcenv_t *)(s), (func)))
#define oskit_libcenv_getconsole(s, intf) \
	((s)->ops->getconsole((oskit_libcenv_t *)(s), (intf)))
#define oskit_libcenv_setconsole(s, intf) \
	((s)->ops->setconsole((oskit_libcenv_t *)(s), (intf)))
#define oskit_libcenv_signals_init(s, func) \
	((s)->ops->signals_init((oskit_libcenv_t *)(s), (func)))
#define oskit_libcenv_setsiginit(s, func) \
	((s)->ops->setsiginit((oskit_libcenv_t *)(s), (func)))
#define oskit_libcenv_sleep_init(s, sr) \
	((s)->ops->sleep_init((oskit_libcenv_t *)(s), (sr)))
#define oskit_libcenv_sleep(s, sr, timeout) \
	((s)->ops->sleep((oskit_libcenv_t *)(s), (sr), (timeout)))
#define oskit_libcenv_wakeup(s, sr) \
	((s)->ops->wakeup((oskit_libcenv_t *)(s), (sr)))
#define oskit_libcenv_clone(s, out_intf) \
	((s)->ops->clone((oskit_libcenv_t *)(s), (out_intf)))

/*
 * Create the default libcenv interface object.
 */
oskit_error_t	oskit_libcenv_create(oskit_libcenv_t **out_iface);
oskit_error_t	oskit_libcenv_create_pthreads(oskit_libcenv_t **out_iface);

/*
 * The boottime (initial) libcenv object. 
 */
#ifdef KNIT
extern oskit_libcenv_t	*initial_clientos_libcenv;
#else
oskit_libcenv_t	*initial_clientos_libcenv;
#endif

#endif /* _OSKIT_COM_LIBCENV_H_ */

