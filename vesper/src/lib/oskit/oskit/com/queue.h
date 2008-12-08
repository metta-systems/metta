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
 * A general queue interface
 */
#ifndef _OSKIT_COM_QUEUE_H_
#define _OSKIT_COM_QUEUE_H_

#include <oskit/com.h>
#include <oskit/com/listener.h>

/*
 * Simple COM interface for FIFO queues
 * IID 4aa7dfaf-7c74-11cf-b500-08000953adc2
 */
struct oskit_queue {
	struct oskit_queue_ops *ops;
};
typedef struct oskit_queue oskit_queue_t;

struct oskit_queue_ops {

	/* COM-specified IUnknown interface operations. */
	OSKIT_COMDECL_IUNKNOWN(oskit_queue_t)

	/*** Operations specific to the oskit_queue interface ***/

	/* Enqueue a certain item with a certain size. */
	OSKIT_COMDECL	(*enqueue)(oskit_queue_t *s, 
			const void *item, oskit_size_t size);

	/* Dequeue item, return its size - buffer must be big enough. */
	OSKIT_COMDECL_U	(*dequeue)(oskit_queue_t *s, void *item);

	/* Return the length of the queue. */
	OSKIT_COMDECL_U	(*size)(oskit_queue_t *s);

	/* Peek at front of queue.  Returns size - buffer must be big enough. */
	OSKIT_COMDECL_U (*front)(oskit_queue_t *s, void *item);
};

#define oskit_queue_query(io, iid, out_ihandle) \
        ((io)->ops->query((oskit_queue_t *)(io), (iid), (out_ihandle)))
#define oskit_queue_addref(io) \
        ((io)->ops->addref((oskit_queue_t *)(io)))
#define oskit_queue_release(io) \
        ((io)->ops->release((oskit_queue_t *)(io)))
#define oskit_queue_enqueue(s, item, size) \
        ((s)->ops->enqueue((oskit_queue_t *)(s), (item), (size)))
#define oskit_queue_dequeue(s, item) \
        ((s)->ops->dequeue((oskit_queue_t *)(s), (item)))
#define oskit_queue_size(s) \
        ((s)->ops->size((oskit_queue_t *)(s)))
#define oskit_queue_front(s, item) \
	((s)->ops->front((oskit_queue_t *)(s), (item)))

/* GUID for oskit_queue interface */
extern const struct oskit_guid oskit_queue_iid;
#define OSKIT_QUEUE_IID OSKIT_GUID(0x4aa7dfaf, 0x7c74, 0x11cf, \
		0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

/*
 * Functions to create queues
 */
/* 
 * This queue is preallocated and bounded, and the number and size of 
 * the queue items is fixed. It has the additional twist that it will 
 * notify `notify_before_dump' when an attempt is made to enqueue an
 * item in a full queue. After the notification, if droplast is set, 
 * the last entry will be dropped and the new entry added. If droplast
 * is not set, enqueue will fail with OSKIT_ENOMEM
 * `notify_before_dump' may be NULL.
 */
oskit_queue_t *create_bounded_queue_with_fixed_size_items(
	int qlen, oskit_size_t itemsize, 
	struct oskit_listener *notify_before_dump,
	int droplast);

/*
 * Create a queue of pointers to COM objects.
 * Reference counts are handled via oskit_iunknown_addref, etc.
 *
 * NOTE: this doesn't return the object size in `dequeue' and `front' since
 * it doesn't make sense in this context (it would if IUnknown had a `size'
 * method).
 */
oskit_error_t oskit_bounded_com_queue_create(oskit_size_t length,
					     oskit_queue_t **out_q);

#endif /* _OSKIT_COM_QUEUE_H_ */
