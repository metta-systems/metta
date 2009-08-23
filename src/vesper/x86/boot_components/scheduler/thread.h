#pragma once

#include "page_directory.h"

namespace scheduler
{

/*!
* Context-switch state.
*/
struct csw_state_t
{
    /*!
    * Page directory to load while running this thread.
    * Kept up-to-date by the task code in task.cpp.
    */
    page_directory_t* page_dir;

    /*!
    * Saved kernel register state for this thread.
    */
    jmp_buf         state;
};

/*!
* Thread is an execution flow abstration, supported by the scheduler.
*/
class thread_t
{
    thread_t();
private:
};

}
