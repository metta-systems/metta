//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Thread dispatcher (in-kernel support of scheduling system)
//
#pragma once

namespace metta {
namespace kernel {
// namespace threads?

/**
* Thread dispatch state.
**/
class thread_disp_state {
public:
protected:
private:
    spin_lock_t     lock;

    /** Link on wait or ready queue.  */
    thread     *next;

    /** Wakeup information.  */
    thread     *last_thread;

    /** `disp_run_tick' value when this thread was last switched into */
    uint32_t     run_tick;

    /**
     * Per-CPU structure for the CPU currently assigned to this thread.
     * This is null when this thread not running.
     */
    struct fluke_cpu_info   *cpu;

    /**
     * Thread we are donating our CPU to; his provider points back to us.
     * If we are readied, we cancel this thread.
     */
    thread     *donating_to;

    /**
     * Thread providing CPU to us; his donating_to points back to us.
     * If we block, we dispatch back to him.  This is nonzero only if
     * we have control of the CPU; the chain of provider links goes back
     * to a root thread representing one real CPU.
     */
    thread     *provider;

    /**
     * This is used with disp_yield to request that this thread
     * yield its CPU back to its provider.
     */
    struct preempter    preempter;

    /**
     * Scheduler port link.
     */
    struct link     sched_link;

    /**
     * This member is only set while the thread is in
     * blocked in the WAIT_SCHEDULER state.
     * Port alias of the scheduler port that led
     * to the scheduler pset wait, so the IPC can be completed
     * without chasing the port and pset links again.
     */
    fluke_alias_t       alias; /* FIXME: use portal aliases? */

    /* Statistics */
    int     scheds;
    int     stamp, valid;
    unsigned    lattotal;
    unsigned    latmin, latmax;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
