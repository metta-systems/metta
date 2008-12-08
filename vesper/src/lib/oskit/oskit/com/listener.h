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
 * OSKit definition of basic listener interface for event notification
 */
#ifndef _OSKIT_COM_LISTENER_H_
#define _OSKIT_COM_LISTENER_H_

#include <oskit/com.h>

/*
 * Basic oskit_listener interface
 * IID 4aa7dfa8-7c74-11cf-b500-08000953adc2
 */
struct oskit_listener {
	struct oskit_listener_ops *ops;
};
typedef struct oskit_listener oskit_listener_t;

struct oskit_listener_ops {
	/* COM-specified IUnknown interface operations */
	OSKIT_COMDECL_IUNKNOWN(oskit_listener_t)

	/* Operations specific to the oskit_listener interface */
	/*
	 * This method will be called with `obj' set to the
	 * object which causes the notification
	 */
	OSKIT_COMDECL	(*notify)(oskit_listener_t *s, oskit_iunknown_t *obj);
};

#define oskit_listener_query(io, iid, out_ihandle) \
	((io)->ops->query((oskit_listener_t *)(io), (iid), (out_ihandle)))
#define oskit_listener_addref(io) \
	((io)->ops->addref((oskit_listener_t *)(io)))
#define oskit_listener_release(io) \
	((io)->ops->release((oskit_listener_t *)(io)))
#define oskit_listener_notify(s, obj) \
	((s)->ops->notify((oskit_listener_t *)(s), (obj)))

/* GUID for oskit_listener interface */
extern const struct oskit_guid oskit_listener_iid;
#define OSKIT_LISTENER_IID OSKIT_GUID(0x4aa7dfa8, 0x7c74, 0x11cf, \
		0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

typedef oskit_error_t (*oskit_listener_callback_t)(
	struct oskit_iunknown *, void *);
oskit_listener_t * oskit_create_listener(
	oskit_listener_callback_t hdlr, void *arg);

#endif /* _OSKIT_COM_LISTENER_H_ */
