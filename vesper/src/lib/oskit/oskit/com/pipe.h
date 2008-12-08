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
 * Definition of the COM oskit_pipe interface, which is a stream
 * representing one side of a descriptor pair. 
 */
#ifndef _OSKIT_COM_PIPE_H_
#define _OSKIT_COM_PIPE_H_

#include <oskit/com/stream.h>

/*
 * IID 4aa7dfb7-7c74-11cf-b500-08000953adc2.
 * This interface extends the stream interface to a pipe interface.
 */
struct oskit_pipe {
	struct oskit_pipe_ops *ops;
};
typedef struct oskit_pipe oskit_pipe_t;

struct oskit_pipe_ops {

	/*** COM-specified IUnknown interface operations ***/
	OSKIT_COMDECL	(*query)(oskit_pipe_t *f,
				 const struct oskit_guid *iid,
				 void **out_ihandle);
	OSKIT_COMDECL_U	(*addref)(oskit_pipe_t *f);
	OSKIT_COMDECL_U	(*release)(oskit_pipe_t *f);

	/*** Operations inherited from oskit_stream interface ***/
	OSKIT_COMDECL	(*read)(oskit_pipe_t *f, void *buf, oskit_u32_t len,
				oskit_u32_t *out_actual);
	OSKIT_COMDECL	(*write)(oskit_pipe_t *f, const void *buf,
				 oskit_u32_t len, oskit_u32_t *out_actual);
	OSKIT_COMDECL	(*seek)(oskit_pipe_t *f, oskit_s64_t ofs,
				oskit_seek_t whence, oskit_u64_t *out_newpos);
	OSKIT_COMDECL	(*setsize)(oskit_pipe_t *f, oskit_u64_t new_size);
	OSKIT_COMDECL	(*copyto)(oskit_pipe_t *f, oskit_stream_t *dst,
				   oskit_u64_t size,
				   oskit_u64_t *out_read,
				   oskit_u64_t *out_written);
	OSKIT_COMDECL	(*commit)(oskit_pipe_t *f, oskit_u32_t commit_flags);
	OSKIT_COMDECL	(*revert)(oskit_pipe_t *f);
	OSKIT_COMDECL	(*lockregion)(oskit_pipe_t *f,
					oskit_u64_t offset, oskit_u64_t size,
					oskit_u32_t lock_type);
	OSKIT_COMDECL	(*unlockregion)(oskit_pipe_t *f,
					 oskit_u64_t offset, oskit_u64_t size,
					 oskit_u32_t lock_type);
	OSKIT_COMDECL	(*stat)(oskit_pipe_t *f, oskit_stream_stat_t *out_stat,
				oskit_u32_t stat_flags);
	OSKIT_COMDECL	(*clone)(oskit_pipe_t *f, oskit_pipe_t **out_stream);
};

#define oskit_pipe_query(f, iid, out_ihandle) \
	((f)->ops->query((oskit_pipe_t *)(f), (iid), (out_ihandle)))
#define oskit_pipe_addref(f) \
	((f)->ops->addref((oskit_pipe_t *)(f)))
#define oskit_pipe_release(f) \
	((f)->ops->release((oskit_pipe_t *)(f)))
#define oskit_pipe_read(f, buf, len, out_actual) \
	((f)->ops->read((oskit_pipe_t *)(f), (buf), (len), (out_actual)))
#define oskit_pipe_write(f, buf, len, out_actual) \
	((f)->ops->write((oskit_pipe_t *)(f), (buf), (len), (out_actual)))
#define oskit_pipe_seek(f, ofs, whence, out_newpos) \
	((f)->ops->seek((oskit_pipe_t *)(f), (ofs), (whence), (out_newpos)))
#define oskit_pipe_setsize(f, new_size) \
	((f)->ops->setsize((oskit_pipe_t *)(f), (new_size)))
#define oskit_pipe_copyto(f, dst, size, out_read, out_written) \
	((f)->ops->copyto((oskit_pipe_t *)(f), (dst), (size), (out_read), (out_written)))
#define oskit_pipe_commit(f, commit_flags) \
	((f)->ops->commit((oskit_pipe_t *)(f), (commit_flags)))
#define oskit_pipe_revert(f) \
	((f)->ops->revert((oskit_pipe_t *)(f)))
#define oskit_pipe_lockregion(f, offset, size, lock_type) \
	((f)->ops->lockregion((oskit_pipe_t *)(f), (offset), (size), (lock_type)))
#define oskit_pipe_unlockregion(f, offset, size, lock_type) \
	((f)->ops->unlockregion((oskit_pipe_t *)(f), (offset), (size), (lock_type)))
#define oskit_pipe_stat(f, out_stat, stat_flags) \
	((f)->ops->stat((oskit_pipe_t *)(f), (out_stat), (stat_flags)))
#define oskit_pipe_clone(f, out_stream) \
	((f)->ops->clone((oskit_pipe_t *)(f), (out_stream)))

/* GUID for oskit_pipe interface */
extern const struct oskit_guid oskit_pipe_iid;
#define OSKIT_PIPE_IID OSKIT_GUID(0x4aa7dfb7, 0x7c74, 0x11cf, \
				0xb5, 0x00, 0x08, 0x00, 0x09, 0x53, 0xad, 0xc2)

/*
 * Utility function to create a pipe.
 */  
OSKIT_COMDECL 
oskit_create_pipe(oskit_pipe_t **out_pipe0, oskit_pipe_t **out_pipe1);

#endif /* _OSKIT_COM_OPENFILE_H_ */
