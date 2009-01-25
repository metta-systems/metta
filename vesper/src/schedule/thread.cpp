#include "thread.h"

/**
* Initialize a new thread in kernel space.
*
* It does not have a task or a start func yet.
**/
thread::thread()
    : csw_state()
    , task_state()
{
    task_state.task = 0;
    csw_state.page_dir = null_pdir;
    flags = STOPPED;
}

/** Called from kernel: perform an upcall into the task, effectively activating thread's life. */
void thread::upcall(task *task)
{
}
