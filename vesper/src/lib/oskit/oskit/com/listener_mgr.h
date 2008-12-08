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
 * implement a list of listeners and the code to notify them - this can be
 * used by any oskit_t COM interface implementation
 */
#ifndef _OSKIT_COM_LISTENER_MGR_H_
#define _OSKIT_COM_LISTENER_MGR_H_

#include <oskit/com.h>
struct oskit_iunknown;
struct oskit_listener;
struct listener_mgr;

/*
 * Prototypes
 */
/*
 * create a new listener_mgr
 */
struct listener_mgr	*oskit_create_listener_mgr(struct oskit_iunknown *ioobj);

/*
 * destroy an listener_mgr
 */
void oskit_destroy_listener_mgr(struct listener_mgr *);

/*
 * add a listener to list
 */
oskit_error_t oskit_listener_mgr_add(struct listener_mgr  *mgr, 
		struct oskit_listener *l);

/*
 * remove a listener from list
 * return 0 on success, an error code if object wasn't on list
 */
oskit_error_t oskit_listener_mgr_remove(struct listener_mgr  *mgr,
		struct oskit_listener *l);

/*
 * Notify all listeners
 */
void oskit_listener_mgr_notify(struct listener_mgr  *mgr);

/*
 * Return number of listeners currently attached
 */
int oskit_listener_mgr_count(struct listener_mgr  *mgr);

#endif /* _OSKIT_COM_LISTENER_MGR_H_ */

