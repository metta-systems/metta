//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "thread.h"

/**
* Initialize a new thread in kernel space.
*
* It does not have a task or a start func yet.
**/
// thread::thread()
//     : csw_state()
//     , task_state()
// {
//     task_state.task = 0;
//     csw_state.page_dir = null_pdir;
//     flags = STOPPED;
// }

/** Called from kernel: perform an upcall into the task, effectively activating thread's life. */
// void thread::upcall(task *task)
// {
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
