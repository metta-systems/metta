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
 * Definition of the condition variable interface
 */

#ifndef _OSKIT_COM_CONDVAR_H_
#define _OSKIT_COM_CONDVAR_H_

#include <oskit/com.h>
#include <oskit/com/lock.h>

/*
 * Interface for using condition variables.
 * IID 4aa7dfb8-7c74-11cf-b500-08000953adc2.
 */
struct oskit_condvar {
	struct oskit_condvar_ops *ops;
};
typedef struct oskit_condvar oskit_condvar_t;

struct oskit_condvar_ops {

	/* COM-specified IUnknown interface operations */
	OSKIT_COMDECL_IUNKNOWN(oskit_condvar_t);

	/*** Operations specific to oskit_condvar_t ***/

	/*
	 * Wait on a condition variable.
	 */
	OSKIT_COMDECL	(*wait)(oskit_condvar_t *c, oskit_lock_t *l);

	/*
	 * Signal a condition variable.
	 */
	OSKIT_COMDECL	(*signal)(oskit_condvar_t *c);

	/*
	 * Broadcast a condition variable.
	 */
	OSKIT_COMDECL	(*broadcast)(oskit_condvar_t *c);
};

/* GUID for oskit_condvar interface */
extern const struct oskit_guid oskit_condvar_iid;
#define OSKIT_CONDVAR_IID OSKIT_GUID(0x4aa7dfb8, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define oskit_condvar_query(c, iid, out_ihandle) \
	((c)->ops->query((oskit_condvar_t *)(c), (iid), (out_ihandle)))
#define oskit_condvar_addref(c) \
	((c)->ops->addref((oskit_condvar_t *)(c)))
#define oskit_condvar_release(c) \
	((c)->ops->release((oskit_condvar_t *)(c)))
#define oskit_condvar_wait(c, l) \
	((c)->ops->wait((oskit_condvar_t *)(c), (oskit_lock_t *)(l)))
#define oskit_condvar_signal(c) \
	((c)->ops->signal((oskit_condvar_t *)(c)))
#define oskit_condvar_broadcast(c) \
	((c)->ops->broadcast((oskit_condvar_t *)(c)))
#endif /* _OSKIT_COM_LOCK_H_ */
