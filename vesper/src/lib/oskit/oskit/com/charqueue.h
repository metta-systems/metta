/*
 * Copyright (c) 1999 University of Utah and the Flux Group.
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
 * A simple bounded queue of characters presented as a COM stream.
 */
#ifndef _OSKIT_COM_CHARQUEUE_H_
#define _OSKIT_COM_CHARQUEUE_H_

#include <oskit/com/stream.h>

/*
 * Create a stream/asyncio object that represents a bounded queue of bytes.
 * The queue is stored in a fixed buffer of the given size.
 * The stream's write method appends characters to the tail of the queue,
 * and its read method consumes characters from the head of the queue.
 * The object also supports the oskit_asyncio_t interface.
 * Writes are always nonblocking; the flags argument determines what
 * happens when a write overruns the queue size: fail with OSKIT_EWOULDBLOCK,
 * report characters as written and drop them, or eject old characters off the
 * head of the queue to make space for new characeters.  Reads are always
 * nonblocking, returning short read counts or OSKIT_EWOULDBLOCK when
 * the queue is empty.
 */
oskit_stream_t *oskit_charqueue_create(oskit_size_t size, unsigned int flags);


#define OSKIT_CHARQUEUE_FULL_ERROR	0x01 /* full queue gives EWOULDBLOCK */
#define OSKIT_CHARQUEUE_FULL_DROP	0x02 /* new chars dropped when full */
#define OSKIT_CHARQUEUE_FULL_REPLACE	0x04 /* old chars dropped for new */


#endif /* _OSKIT_COM_CHARQUEUE_H_ */
