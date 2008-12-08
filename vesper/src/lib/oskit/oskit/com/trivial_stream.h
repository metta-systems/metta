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
 * Implement a trivial COM stream using a simple getchar/putchar function pair.
 */
#ifndef _OSKIT_COM_TRIVIAL_STREAM_H_
#define _OSKIT_COM_TRIVIAL_STREAM_H_

#include <oskit/com/stream.h>

/*
 * This is an "open implementation" adapter, with its data structure
 * declared in the public header so users can define a statically-initialized
 * `struct oskit_trivial_stream' that is ready for use with no initialization.
 * oskit_trivial_stream_ops is the method table used for trivial streams.
 */
struct oskit_trivial_stream {
	oskit_stream_t streami;	/* { ops: &oskit_trivial_stream_ops } */
	int (*getchar)(void);
	int (*putchar)(int);
};
extern struct oskit_stream_ops oskit_trivial_stream_ops;



#endif /* _OSKIT_COM_TRIVIAL_STREAM_H_ */
