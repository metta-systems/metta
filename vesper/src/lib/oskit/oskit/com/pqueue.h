/*
 * Copyright (c) 1998 The University of Utah and the Flux Group.
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
 * An interface for priority queues of COM objects with U32 keys.
 * This isn't very general right now...it is tailor-made for the PFQ stuff.
 */

#ifndef _OSKIT_COM_PQUEUE_H_
#define _OSKIT_COM_PQUEUE_H_

#include <oskit/types.h>
#include <oskit/com.h>

typedef oskit_u32_t oskit_pqueue_key_t;

/*
 * Priority queue interface for queues of COM objects.
 * IID 4aa7dfbb-7c74-11cf-b500-08000953adc2
 */
struct oskit_pqueue {
	struct oskit_pqueue_ops *ops;
};
typedef struct oskit_pqueue oskit_pqueue_t;

struct oskit_pqueue_ops {
	/* COM-specified IUnknown interface operations. */
	OSKIT_COMDECL_IUNKNOWN(oskit_pqueue_t)

	/*** Operations specific to priority queues. ***/

	/*
	 * Enqueue an object OBJ with value VAL.
	 * The reference count on OBJ is incremented.
	 * Returns zero if successful, an error code from
	 * <oskit/error.h> otherwise (such as OSKIT_E_OUTOFMEMORY).
	 */
	OSKIT_COMDECL (*enqueue)(oskit_pqueue_t *pq,
				 oskit_iunknown_t *obj,
				 oskit_pqueue_key_t val);

	/*
	 * Return a pointer to front item of the queue PQ,
	 * or NULL if the queue is empty.
	 * The queue is not changed.
	 * The item's value is returned in VALP.
	 * If VALP is NULL, the item's value is not returned.
	 * When the caller is done with the returned object, it must
	 * oskit_iunknown_release it.
	 */
	oskit_iunknown_t * OSKIT_COMCALL (*front)(oskit_pqueue_t *pq,
						  oskit_pqueue_key_t *valp);

	/*
	 * Remove an object OBJ from the queue PQ.
	 * If the object is in there more than once, then only the
	 * first occurrence is removed.
	 * Returns zero if successful, OSKIT_E_FAIL if the item is not found.
	 */
	OSKIT_COMDECL (*remove)(oskit_pqueue_t *pq, oskit_iunknown_t *obj);

	/*
	 * Return the current size of the queue PQ.
	 */
	oskit_size_t OSKIT_COMCALL (*size)(oskit_pqueue_t *pq);

	/*
	 * Find and return a pointer to the first element passing a
	 * predicate.
	 * If none satisfy the predicate, return NULL.
	 */
	oskit_iunknown_t * OSKIT_COMCALL (*first_satisfying)(
		oskit_pqueue_t *pq,
		oskit_bool_t (*predicate)(oskit_iunknown_t *,
					  oskit_pqueue_key_t));
};

#define oskit_pqueue_query(io, iid, out_ihandle) \
	((io)->ops->query((oskit_pqueue_t *)(io), (iid), (out_ihandle)))
#define oskit_pqueue_addref(io) \
	((io)->ops->addref((oskit_pqueue_t *)(io)))
#define oskit_pqueue_release(io) \
	((io)->ops->release((oskit_pqueue_t *)(io)))
#define oskit_pqueue_enqueue(pq, obj, val) \
	((pq)->ops->enqueue((oskit_pqueue_t *)(pq), (obj), (val)))
#define oskit_pqueue_front(pq, valp) \
	((pq)->ops->front((oskit_pqueue_t *)(pq), (valp)))
#define oskit_pqueue_remove(pq, obj) \
	((pq)->ops->remove((oskit_pqueue_t *)(pq), (obj)))
#define oskit_pqueue_size(pq) \
	((pq)->ops->size((oskit_pqueue_t *)(pq)))
#define oskit_pqueue_first_satisfying(pq, pred) \
	((pq)->ops->first_satisfying((oskit_pqueue_t *)(pq), (pred)))

extern const struct oskit_guid oskit_pqueue_iid;
#define OSKIT_PQUEUE_IID OSKIT_GUID(0x4aa7dfbb, 0x7c74, 0x11cf, \
		0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)


/*
 * Create an empty queue.
 * For future improvement, make this take a max size.
 */
oskit_error_t oskit_pqueue_create(oskit_pqueue_t **out_pq);

#endif /* _OSKIT_COM_PQUEUE_H_ */
