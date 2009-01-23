//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Kernel part of thread object.
//
#pragma once

#include "object.h"

namespace metta {
namespace kernel {

/**
* Thread is two-fold object:
*  - it's a flow of control and corresponding state, allowing to resume that flow.
*    (related to this are disp_state, csw_state, exc_state)
*  - it's a schedulable entity with corresponding priorities and track record of activations.
*    (related to this are wait_state, task_state, ipc_state and sched_state)
**/
class thread : public object {
public:
protected:
private:
    /**
    * Virtual address of the user part of the thread object
    * as mapped into the task the thread runs in.
    **/
    address_t        thread_va;

    /*
     * The remainder of this structure is simply an aggregation of
     * all the per-thread state used in all of the functionality layers
     * that require any per-thread state.
     * Code in each functionality layer must only access
     * that particular layer's thread-specific state area.
     * This basically takes the place of pthread's thread-specific data;
     * this approach is much simpler and more efficient
     * given the fairly limited, static kernel environment.
     */
    csw_state    csw_state; // context switch state
    exc_state    exc_state; // exception state
//     DECLARE_EXC_STATE;   /* see machine/exception.h */
    disp_state   disp_state; // dispatch state
    wait_state   wait_state; // wait state
    task_state   task_state; // task state
    ipc_state    ipc_state;  // ipc state
    sched_state      sched_state; // scheduler state

    /*
     * When another thread is trying to stop and manipulate us,
     * it cancels us and then waits on this condition variable.
     */
    cond     stop_cond;

    /*
     * When another thread is in a thread_wait waiting for us to
     * wake it up "by name", it puts itself on this queue.
     * When we are destroyed, we must wake all these threads
     * so they can restart and see the invalidity of their links to us.
     */
    cond     death_cond;

    /*
     * Thread flags, as defined in fluke/x86/thread.h.
     * 'gflags' stores the "global" flags accessible by other threads,
     * this flags word is locked by the thread's sob lock.
     * 'lflags' stores the "local" flags accessible only by this thread
     * (or a thread that is holding this thread captive);
     * accesses to this flags word do not need to be locked.
     * The THREAD_GFLAGS symbol below defines the flags that are in gflags,
     * and THREAD_LFLAGS defines the flags that are in lflags.
     */
    unsigned        gflags;
#ifndef CONFIG_OPT_THREADS
    unsigned        lflags;
#define th_lflags lflags
#endif

#if FLASK
    security_class_t sclass;
    security_id_t ssid;
    security_class_t tclass;
    security_id_t tsid;
    access_vector_t requested;
#endif

#ifdef CONFIG_INSTRUMENT
    unsigned                instrument_flags;
    void *                  instrument_buffer;
    unsigned                instrument_buffer_used;
#endif

#ifdef DEBUG
    /* Kernel return stats */
    struct kr_stats {
        unsigned calls;
        unsigned uexceptions;
        unsigned pagefaults;
        unsigned cancels;
        unsigned restarts;
        unsigned secfaults;
    }           kr_stats;
#endif

    /* Machine-dependent thread layer data. */
    s_thread_md  md;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
