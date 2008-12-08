/*
 * Copyright (c) 2000 University of Utah and the Flux Group.
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
 * OSKit random number generator component, based on the FreeBSD C library
 * `random' and `srandom' functions.
 */
#ifndef _OSKIT_COM_RANDOM_H_
#define _OSKIT_COM_RANDOM_H_

#include <oskit/types.h>
#include <oskit/com.h>

/*
 * Random number generator interface.
 * IID 4aa7dffd-7c74-11cf-b500-08000953adc2
 */
struct oskit_random {
	struct oskit_random_ops *ops;
};
typedef struct oskit_random oskit_random_t;

struct oskit_random_ops {
	/*
	 * COM-specified IUnknown interface operations.
	 */
	OSKIT_COMDECL_IUNKNOWN(oskit_random_t)

	/*
	 * Operations specific to the random number generator interface.
	 */
	OSKIT_COMDECL	(*random)(oskit_random_t *		r,
				  oskit_s32_t *			out_num);
	OSKIT_COMDECL	(*srandom)(oskit_random_t *		r,
				   oskit_u32_t			seed);
#if 0
	OSKIT_COMDECL	(*srandomdev)(oskit_random_t *		r);
	OSKIT_COMDECL	(*initstate)(oskit_random_t *		r,
				     oskit_s32_t		seed,
				     com_char_t *		out_state,
				     oskit_s32_t		out_state_n);
	OSKIT_COMDECL	(*setstate)(oskit_random_t *		r,
				    com_char_t *		state);
#endif /* 0 */
};

#define oskit_random_query(r, iid, out_ihandle) \
	((r)->ops->query((oskit_random_t *)(r), (iid), (out_ihandle)))
#define oskit_random_addref(r) \
	((r)->ops->addref((oskit_random_t *)(r)))
#define oskit_random_release(r) \
	((r)->ops->release((oskit_random_t *)(r)))
#define oskit_random_random(r, out_num) \
	((r)->ops->random((oskit_random_t *)(r), (out_num)))
#define oskit_random_srandom(r, seed) \
	((r)->ops->srandom((oskit_random_t *)(r), (seed)))

/* GUID for oskit_random interface */
extern const struct oskit_guid oskit_random_iid;
#define OSKIT_RANDOM_IID OSKIT_GUID(0x4aa7dffd, 0x7c74, 0x11cf, \
		0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

/*
 * Create a random number generator.
 */
oskit_error_t oskit_random_create(oskit_random_t **out_r);

#endif /* _OSKIT_COM_RANDOM_H_ */
