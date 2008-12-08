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
 * OSKit definition of basic COM/OLE IStorage interface defined by Microsoft
 */
#ifndef _OSKIT_COM_STORAGE_H_
#define _OSKIT_COM_STORAGE_H_

#include <oskit/com.h>

struct com_stream;

/*
 * Basic data storage interface defined by Microsoft,
 * IID 0000000b-0000-0000-C000-000000000046.
 * Its original name in Microsoft-style is IStorage.
 */
struct com_storage {
	struct com_storage_ops *ops;
};
typedef struct com_storage com_storage_t;

struct com_storage_ops {

	/* COM-specified IUnknown interface operations */
	OSKIT_COMDECL	(*query)(com_storage_t *s, const struct com_guid *iid,
				 void **out_ihandle);
	OSKIT_COMDECL_U	(*addref)(com_storage_t *s);
	OSKIT_COMDECL_U	(*release)(com_storage_t *s);

	/* Operations specific to the IStorage interface */
	OSKIT_COMDECL	(*create_stream)(com_storage_t *s,
					 const com_char_t *name,
					 oskit_u32_t mode,
					 oskit_u32_t reserved1,
					 oskit_u32_t reserved2,
					 struct com_stream **out_stream);
	OSKIT_COMDECL	(*open_stream)(com_storage_t *s,
					const com_char_t *name,
					void * reserved1,
					oskit_u32_t mode,
					oskit_u32_t reserved2,
					struct com_stream **out_stream);
	OSKIT_COMDECL	(*create_storage)(com_storage_t *s,
					  const com_char_t *name,
					  oskit_u32_t mode,
					  oskit_u32_t storage_format,
					  oskit_u32_t reserved2,
					  com_storage_t **out_storage);
	OSKIT_COMDECL	(*open_storage)(com_storage_t *s,
					const com_char_t *name,
					com_storage_t *priority,
					oskit_u32_t mode,
					com_char_t **snb_exclude,
					com_storage_t **out_storage);
	OSKIT_COMDECL	(*copy_to)(com_storage_t *s,
				   oskit_u32_t exclude,
				   const com_iid_t *exclude_iid,
				   com_char_t **snb_exclude,
				   com_storage_t *dest);
	OSKIT_COMDECL	(*move_element_to)(com_storage_t *s,
					   const com_char_t *name,
					   com_storage_t *dest,
					   const com_char_t *dest_name,
					   oskit_u32_t flags);
	...more...
};

/* GUID for com_storage interface */
extern const struct com_guid com_storage_iid;
#define OSKIT_STORAGE_IID OSKIT_GUID(0x0000000b, 0x0000, 0x0000, \
		0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)


#endif /* _OSKIT_COM_STORAGE_H_ */
