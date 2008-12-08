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
 * Definition of the dynamic aggregation interfaces,
 * which allow an already-existing object's interfaces
 * to be dynamically aggregated into another object.
 * This contrasts with IClassFactory-based aggregation in COM,
 * where the aggregated interfaces can only be created
 * at the time the inner object itself is created.
 */
#ifndef _OSKIT_COM_AGGREGATE_H_
#define _OSKIT_COM_AGGREGATE_H_


/*
 * Interface used to administer a piece of an aggregate,
 * IID 4aa7df91-7c74-11cf-b500-08000953adc2.
 *
 * This interface basically corresponds to the "special" IUnknown interface
 * returned from CoCreateInstance() when pUnkOuter is non-NULL
 * in the standard COM class-based aggregation mechanism;
 * however, unlike in the kludgy Microsoft aggregation mechanism,
 * an aggregate_part object is a _real_ COM object
 * that supports standard QueryInterface semantics and such.
 * Instead of overloading the QueryInterface method,
 * we provide an additional, separate method, QueryAggregate(),
 * to produce the interfaces that are to become part of the larger aggregate.
 * In addition, this interface safely supports "blind aggregation",
 * which is simply flat-out forbidden in the original system.
 *
 * Note that even if an object supports aggregation,
 * it does not necessarily support it on all of its interfaces.
 * In other words, QueryAggregate() may fail for an interface ID
 * for which a Query() on the basic, non-aggregated inner object would succeed.
 */
struct aggregate_part {
	struct aggregate_part_ops *ops;
};
typedef struct aggregate_part aggregate_part_t;

struct aggregate_part_ops {

	/*** IUnknown inherited methods ***/
	OSKIT_COMDECL	(*query)(aggregate_part_t *p,
				 const struct com_guid *iid,
				 void **out_ihandle);
	OSKIT_COMDECL_U	(*addref)(aggregate_part_t *p);
	OSKIT_COMDECL_U	(*release)(aggregate_part_t *p);

	/*** Methods specific to aggregate interface ***/

	/*
	 * Locate (or create) and return a reference
	 * to an interface which is implemented by the inner object
	 * but is aggregated into the outer object.
	 * Any IUnknown operations on the returned interface
	 * will be forwarded directly to the outer object.
	 * As part of a successful QueryAggregate() call,
	 * the outer object's IUnknown reference count will be bumped by one
	 * (since the outer object performs all the reference counting
	 * on behalf of these aggregated interfaces).
	 * Because the aggregate interfaces are not individually ref-counted,
	 * all of these aggregated interfaces must remain in existence
	 * for the lifetime of the aggregate_part object that produced them.
	 */
	OSKIT_COMDECL	(*query_aggregate)(aggregate_part_t *p,
					   const struct com_guid *iid,
					   void **out_handle);

	/*
	 * This method performs the same function as QueryAggregate(),
	 * except it only succeeds on interfaces whose semantics
	 * safely allow "blind aggregation" with arbitrary other interfaces.
	 * This is generally true of most interfaces, but not all.
	 * To be specific, any interfaces whose semantics
	 * include "blanket constraints" on the object as a whole,
	 * or on all the object's other interfaces,
	 * is not safe for blind aggregation in an outer object,
	 * because the outer object does not know those constraints
	 * and thus will not be able to satisfy them properly.
	 * For example, an interface whose presence indicates
	 * that the entire object is thread-safe cannot be blindly aggregated.
	 * However, an interface that includes a method
	 * to query a property of a _specific_ other interface,
	 * such as the ISupportErrorInfo interface defined by OLE,
	 * is safe for blind aggregation
	 * because the inner object's implementation of that method
	 * will return false for any interfaces it doesn't know about
	 * but were aggregated with it by the outer object.
	 */
	OSKIT_COMDECL	(*query_blind)(aggregate_part_t *p,
				       const struct com_guid *iid,
				       void **out_handle);
};

/* GUID for aggregate_part interface */
extern const struct com_guid aggregate_part_iid;
#define AGGREGATE_PART_IID OSKIT_GUID(0x4aa7df91, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define aggregate_part_query(p, iid, out_ihandle) \
	((p)->ops->query((aggregate_part_t *)(p), (iid), (out_ihandle)))
#define aggregate_part_addref(p) \
	((p)->ops->addref((aggregate_part_t *)(p)))
#define aggregate_part_release(p) \
	((p)->ops->release((aggregate_part_t *)(p)))
#define aggregate_part_query_aggregate(p, iid, out_handle) \
	((p)->ops->query_aggregate((aggregate_part_t *)(p), (iid), (out_handle)))
#define aggregate_part_query_blind(p, iid, out_handle) \
	((p)->ops->query_blind((aggregate_part_t *)(p), (iid), (out_handle)))

/*
 * Dynamic object aggregation interface,
 * If an object supports this interface,
 * it means that it supports dynamic aggregation (to some degree anyway),
 * meaning that this pre-existing object can be aggregated
 * into some larger object being constructed later.
 * (In contrast, Microsoft's standard aggregation mechanism
 * only allows objects to be aggregated at creation time.)
 * IID 4aa7df90-7c74-11cf-b500-08000953adc2.
 */
struct aggregation {
	struct aggregation_ops *ops;
};
typedef struct aggregation aggregation_t;

struct aggregation_ops {

	/*** IUnknown inherited methods ***/
	OSKIT_COMDECL	(*query)(aggregation_t *a,
				 const struct com_guid *iid,
				 void **out_ihandle);
	OSKIT_COMDECL_U	(*addref)(aggregation_t *a);
	OSKIT_COMDECL_U	(*release)(aggregation_t *a);

	/*** Methods specific to the dynamic aggregation interface ***/

	/*
	 * Aggregate this object into a larger "outer" object.
	 * To be specific:
	 * Create and return an aggregate_part object
	 * from which interfaces can be requested
	 * that are functionally identical
	 * to this object's actual corresponding interfaces,
	 * except that their IUnknown operations are delegated
	 * to the specified 'outer' object.
	 * The inner object will _not_ call addref() on this outer object,
	 * as this would create a reference counting cycle;
	 * instead the inner object will simply assume
	 * that the outer object will remain in existence at least as long
	 * as this new aggregate_part object remains in existence.
	 */
	OSKIT_COMDECL	(*aggregate)(aggregation_t *a, com_unknown_t *outer,
				     aggregate_part_t **out_part);
};

/* GUID for aggregation interface */
extern const struct com_guid aggregation_iid;
#define AGGREGATE_IID OSKIT_GUID(0x4aa7df90, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

#define aggregation_query(a, iid, out_ihandle) \
	((a)->ops->query((aggregation_t *)(a), (iid), (out_ihandle)))
#define aggregation_addref(a) \
	((a)->ops->addref((aggregation_t *)(a)))
#define aggregation_release(a) \
	((a)->ops->release((aggregation_t *)(a)))
#define aggregation_aggregate(a, outer, out_part) \
	((a)->ops->aggregate((aggregation_t *)(a), (outer), (out_part)))

#endif /* _OSKIT_COM_AGGREGATE_H_ */
