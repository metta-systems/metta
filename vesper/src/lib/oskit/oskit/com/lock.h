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
 * Definition of the thread safe lock interface.
 */

#ifndef _OSKIT_COM_LOCK_H_
#define _OSKIT_COM_LOCK_H_

#include <oskit/com.h>

/*
 * Interface used to lock and unlock locks created with the lock manager.
 * IID 4aa7dfb6-7c74-11cf-b500-08000953adc2.
 */
struct oskit_lock {
	struct oskit_lock_ops *ops;
};
typedef struct oskit_lock oskit_lock_t;

struct oskit_lock_ops {

	/* COM-specified IUnknown interface operations */
	OSKIT_COMDECL_IUNKNOWN(oskit_lock_t);

	/*** Operations specific to oskit_lock_t ***/

	/*
	 * Lock a lock object.
	 */
	OSKIT_COMDECL	(*lock)(oskit_lock_t *l);

	/*
	 * Unlock a lock object.
	 */
	OSKIT_COMDECL	(*unlock)(oskit_lock_t *l);
};

/* GUID for oskit_lock interface */
extern const struct oskit_guid oskit_lock_iid;
#define OSKIT_LOCK_IID OSKIT_GUID(0x4aa7dfb6, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define oskit_lock_query(l, iid, out_ihandle) \
	((l)->ops->query((oskit_lock_t *)(l), (iid), (out_ihandle)))
#define oskit_lock_addref(l) \
	((l)->ops->addref((oskit_lock_t *)(l)))
#define oskit_lock_release(l) \
	((l)->ops->release((oskit_lock_t *)(l)))
#define oskit_lock_lock(l) \
	((l)->ops->lock((oskit_lock_t *)(l)))
#define oskit_lock_unlock(l) \
	((l)->ops->unlock((oskit_lock_t *)(l)))

#endif /* _OSKIT_COM_LOCK_H_ */
