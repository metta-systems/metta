/*
 * Copyright (c) 1997-1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * Contributed by the Computer Security Research division,
 * INFOSEC Research and Technology Office, NSA.
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
 * Declarations for the sfs wrappers provided by -loskit_com.
 */

#ifndef _OSKIT_COM_SFS_H
#define _OSKIT_COM_SFS_H

#include <oskit/com.h>
#include <oskit/com/services.h>
#include <oskit/flask/security.h>
#include <oskit/flask/avc.h>
#include <oskit/fs/filepsid.h>
#include <oskit/fs/dir.h>

oskit_error_t
oskit_sfs_wrap(oskit_dir_t *rroot, 
	       oskit_security_id_t fs_sid,
	       oskit_services_t *osenv,
	       oskit_filepsid_t *psid,
	       oskit_avc_t *avc,
	       oskit_security_t *security,
	       oskit_dir_t **out_sroot);


#endif /* _OSKIT_COM_SFS_H */
