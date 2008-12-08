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
 * Definition of the Services interface,
 * which is just a registration database
 * that indexes active COM object references
 * according to the GUIDs of their interfaces (their IIDs).
 * Thus, through this database,
 * it is possible to lookup and rendezvous with an arbitrary "service"
 * given only the interface ID of the desired interface.
 * More than one interface supporting a particular IID can be registered;
 * for example, there might be one registered 'oskit_etherdev_t' interface
 * for each active Ethernet device in the system.
 */
#ifndef _OSKIT_COM_SERVICES_H_
#define _OSKIT_COM_SERVICES_H_

#include <oskit/com.h>

/*
 * Basic service database interface,
 * IID 4aa7dfa5-7c74-11cf-b500-08000953adc2.
 */
struct oskit_services {
	struct oskit_services_ops *ops;
};
typedef struct oskit_services oskit_services_t;

struct oskit_services_ops {

	/*** Operations inherited from IUnknown ***/
	OSKIT_COMDECL	(*query)(oskit_services_t *s,
				 const struct oskit_guid *iid,
				 void **out_ihandle);
	OSKIT_COMDECL_U	(*addref)(oskit_services_t *s);
	OSKIT_COMDECL_U	(*release)(oskit_services_t *s);

	/*** Operations specific to oskit_services_t ***/

	/*
	 * Register an interface in the services database.
	 * More than one interface can be registered for a particular IID.
	 */
	OSKIT_COMDECL	(*addservice)(oskit_services_t *s,
				const struct oskit_guid *iid, void *intf);

	/*
	 * Unregister a previously registered interface.
	 */
	OSKIT_COMDECL	(*remservice)(oskit_services_t *s,
				const struct oskit_guid *iid, void *intf);

	/*
	 * Obtain a list of all the registered interfaces with a specified IID.
	 * When the client is finished with the returned array,
	 * it must release all the references it contains
	 * and free the array itself using the task allocator.
	 * Returns the number of interface pointers in the returned array;
	 * if there are no matches in the database, returns 0
	 * with *out_interface_array set to NULL.
	 * By default, the first interface registered is the first returned.
	 */
	OSKIT_COMDECL	(*lookup)(oskit_services_t *s,
			        const oskit_guid_t *iid,
			        void ***out_interface_array);

	/*
	 * Lookup the first interface registered for a given IID.
	 * This is typically used to look up "the" instance of a service,
	 * when only one instance is needed or expected.
	 * Returns an error if no interface is registered with the IID.
	 */
	OSKIT_COMDECL	(*lookup_first)(oskit_services_t *s,
				const oskit_guid_t *iid, void **out_intf);

	/*
	 * Clone an entire database
	 */
	OSKIT_COMDECL	(*clone)(oskit_services_t *s, oskit_services_t **intf);
};

/* GUID for oskit_services interface */
extern const struct oskit_guid oskit_services_iid;
#define OSKIT_SERVICES_IID OSKIT_GUID(0x4aa7dfa5, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define oskit_services_query(s, iid, out_ihandle) \
	((s)->ops->query((oskit_services_t *)(s), (iid), (out_ihandle)))
#define oskit_services_addref(s) \
	((s)->ops->addref((oskit_services_t *)(s)))
#define oskit_services_release(s) \
	((s)->ops->release((oskit_services_t *)(s)))
#define oskit_services_addservice(s, iid, intf) \
	((s)->ops->addservice((oskit_services_t *)(s), (iid), (intf)))
#define oskit_services_remservice(s, iid, intf) \
	((s)->ops->remservice((oskit_services_t *)(s), (iid), (intf)))
#define oskit_services_lookup(s, iid, out_interface_array) \
	((s)->ops->lookup((oskit_services_t *)(s),(iid),(out_interface_array)))
#define oskit_services_lookup_first(s, iid, out_intf) \
	((s)->ops->lookup_first((oskit_services_t *)(s), (iid), (out_intf)))
#define oskit_services_clone(s, out_intf) \
	((s)->ops->clone((oskit_services_t *)(s), (out_intf)))

/*
 * Function to create a services database.
 */
struct oskit_mem;
oskit_error_t
oskit_services_create(struct oskit_mem *memobject, oskit_services_t **intf);

/*** Default services database implementation ***/
/*
 * These functions provide a default implementation of the services database.
 * The oskit_global_registry_create is the primary call. It creates the
 * the global registry. It must be parameterized with an appropriate memory
 * object to avoid the circularity that would be caused if it used malloc.
 */
oskit_error_t oskit_global_registry_create(struct oskit_mem *memobject);
oskit_services_t *oskit_get_services(void);
oskit_error_t oskit_register(const struct oskit_guid *iid, void *interface);
oskit_error_t oskit_unregister(const struct oskit_guid *iid, void *interface);
oskit_error_t oskit_lookup(const oskit_guid_t *iid, void ***out_interface_array);
oskit_error_t oskit_lookup_first(const oskit_guid_t *iid, void **out_interface);

#endif /* _OSKIT_COM_SERVICES_H_ */
