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
 * OSKit definition of basic COM/OLE IStream interface defined by Microsoft
 */
#ifndef _OSKIT_COM_STREAM_H_
#define _OSKIT_COM_STREAM_H_

#include <oskit/com.h>

/*
 * Seek bases for COM streams.
 * This enumeration corresponds to Microsoft's STREAM_SEEK enum.
 */
enum oskit_seek {
	OSKIT_SEEK_SET,
	OSKIT_SEEK_CUR,
	OSKIT_SEEK_END
};
typedef enum oskit_seek oskit_seek_t;


/*
 * Stat structure returned by the oskit_stream->stat() operation.
 * This corresponds to the STATSTG structure defined by Microsoft.
 */
struct oskit_stream_stat {
	oskit_wchar_t	*name;	/* XXX is this actually Unicode? */
	oskit_u32_t	type;
	oskit_u64_t	size;
};
typedef struct oskit_stream_stat oskit_stream_stat_t;

/*
 * Basic stream I/O interface defined by Microsoft,
 * IID 0000000c-0000-0000-C000-000000000046.
 * Its original name in Microsoft-style is IStream.
 */
struct oskit_stream {
	struct oskit_stream_ops *ops;
};
typedef struct oskit_stream oskit_stream_t;

struct oskit_stream_ops {

	/* COM-specified IUnknown interface operations */
	OSKIT_COMDECL_IUNKNOWN(oskit_stream_t)

	/* Operations specific to the IStream interface */
	OSKIT_COMDECL	(*read)(oskit_stream_t *s, void *buf, oskit_u32_t len,
				oskit_u32_t *out_actual);
	OSKIT_COMDECL	(*write)(oskit_stream_t *s, const void *buf,
				 oskit_u32_t len, oskit_u32_t *out_actual);
	OSKIT_COMDECL	(*seek)(oskit_stream_t *s, oskit_s64_t ofs,
				oskit_seek_t whence, oskit_u64_t *out_newpos);
	OSKIT_COMDECL	(*setsize)(oskit_stream_t *s, oskit_u64_t new_size);
	OSKIT_COMDECL	(*copyto)(oskit_stream_t *s, oskit_stream_t *dst,
				  oskit_u64_t size,
				  oskit_u64_t *out_read,
				  oskit_u64_t *out_written);
	OSKIT_COMDECL	(*commit)(oskit_stream_t *s, oskit_u32_t commit_flags);
	OSKIT_COMDECL	(*revert)(oskit_stream_t *s);
	OSKIT_COMDECL	(*lockregion)(oskit_stream_t *s,
				      oskit_u64_t offset, oskit_u64_t size,
				      oskit_u32_t lock_type);
	OSKIT_COMDECL	(*unlockregion)(oskit_stream_t *s,
					oskit_u64_t offset, oskit_u64_t size,
					oskit_u32_t lock_type);
	OSKIT_COMDECL	(*stat)(oskit_stream_t *s, oskit_stream_stat_t *out_stat,
				oskit_u32_t stat_flags);
	OSKIT_COMDECL	(*clone)(oskit_stream_t *s, oskit_stream_t **out_stream);
};

#define oskit_stream_query(io, iid, out_ihandle) \
	((io)->ops->query((oskit_stream_t *)(io), (iid), (out_ihandle)))
#define oskit_stream_addref(io) \
	((io)->ops->addref((oskit_stream_t *)(io)))
#define oskit_stream_release(io) \
	((io)->ops->release((oskit_stream_t *)(io)))
#define oskit_stream_read(s, buf, len, out_actual) \
	((s)->ops->read((oskit_stream_t *)(s), (buf), (len), (out_actual)))
#define oskit_stream_write(s, buf, len, out_actual) \
	((s)->ops->write((oskit_stream_t *)(s), (buf), (len), (out_actual)))
#define oskit_stream_seek(s, ofs, whence, out_newpos) \
	((s)->ops->seek((oskit_stream_t *)(s), (ofs), (whence), (out_newpos)))
#define oskit_stream_setsize(s, new_size) \
	((s)->ops->setsize((oskit_stream_t *)(s), (new_size)))
#define oskit_stream_copyto(s, dst, size, out_read, out_written) \
	((s)->ops->copyto((oskit_stream_t *)(s), (dst), (size), (out_read), (out_written)))
#define oskit_stream_commit(s, commit_flags) \
	((s)->ops->commit((oskit_stream_t *)(s), (commit_flags)))
#define oskit_stream_revert(s) \
	((s)->ops->revert((oskit_stream_t *)(s)))
#define oskit_stream_lockregion(s, offset, size, lock_type) \
	((s)->ops->lockregion((oskit_stream_t *)(s), (offset), (size), (lock_type)))
#define oskit_stream_unlockregion(s, offset, size, lock_type) \
	((s)->ops->unlockregion((oskit_stream_t *)(s), (offset), (size), (lock_type)))
#define oskit_stream_stat(s, out_stat, stat_flags) \
	((s)->ops->stat((oskit_stream_t *)(s), (out_stat), (stat_flags)))
#define oskit_stream_clone(s, out_stream) \
	((s)->ops->clone((oskit_stream_t *)(s), (out_stream)))

/* GUID for oskit_stream interface */
extern const struct oskit_guid oskit_stream_iid;
#define OSKIT_STREAM_IID OSKIT_GUID(0x0000000c, 0x0000, 0x0000, \
		0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)


#endif /* _OSKIT_COM_STREAM_H_ */
