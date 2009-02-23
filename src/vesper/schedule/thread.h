//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Kernel part of thread object.
//
#pragma once

#include <setjmp.h>
#include "object.h"
#include "dispatch.h"
#include "link.h"

namespace metta {
namespace kernel {

class page_directory;
class thread;
class task;
class cond;//expand to condition ?

/**
* Context-switch state.
**/
struct csw_state
{
    /**
    * Page directory to load while running this thread.
    * Kept up-to-date by the task code in task.cpp.
    **/
    page_directory* page_dir;

    /**
    * Saved kernel register state for this thread.
    **/
    jmp_buf         state;
};

#define THREAD_STACK_SIZE   3500

/**
* Exception state.
*
* This structure describes the state of user registers
* as saved upon kernel trap/interrupt entry.
* Note that we don't need segment registers or v86-mode stuff
* because none of that is supported in the standard Fluke environment;
* emulating other OS's is done in a separate "emulation" environment.
**/
struct exc_state
{
    uint32_t    stack[THREAD_STACK_SIZE/4];

    /* PUSHA register state frame */
    uint32_t    edi;
    uint32_t    esi;
    uint32_t    ebp;
    uint32_t    ebx;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    eax;

    /* Fluke exception code and subcode */
    uint32_t    code;
    uint32_t    subcode;

    /* Processor state frame */
    uint32_t    eip;
    uint32_t    cs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    ss;
};

typedef uint32_t wait_val_t;

/**
* Wait state.
*
* Per-thread state maintained by the cancellation support code.
**/
struct wait_state : public lockable
{
    /** Current thread waiting state value, defined above. */
    wait_val_t      val;

    /**
    * Cancel pending flag.
    * Set by thread_cancel(); cleared by thread_clear_cancel().
    **/
    bool            cancel_pending;

    /**
    * KR_* code indicating what to do on wakeup.
    * Set to KR_RESTART when a thread is woken normally.
    * Set to KR_CANCEL when a thread is woken by cancellation,
    * and can be set to KR_USER_EXCEPTION by the IPC code
    * to cause a receiver thread to take an exception.
    **/
    int             resume_rc;

    /**
    * Whenever this thread is waiting on a condition variable,
    * this variable points to the condition the thread is waiting on.
    * At all other times, this pointer is null.
    **/
    cond*           waiting_on;
};

/**
* Task state.
*
* Per-thread state maintained by the task layer.
**/
struct task_state
{
    /**
    * Pointer to the task this thread is running in, if any.
    * If non-NULL, implies that we are on that task's threads list.
    * Can only be modified by the thread itself,
    * while holding the task's main object lock.
    **/
    task*           task_;

    /**
    * Chain on our task's threads list.
    * Protected by the task's main lock.
    **/
//     queue_chain_t   task_chain;
// FIXME: need a sane lock-free/lockable queue object?

    /**
    * When someone is trying to destroy a task,
    * it sets the task_dying flags in all threads running in the task,
    * and cancels all of those threads.
    * When the threads wake up and notice this flag,
    * they stop and remove themselves from the task's list.
    **/
    bool            task_dying;

    /**
    * When a thread is stopped,
    * this holds its task association instead of 'task'.
    **/
    link            task_link;
};

/* == berkus ==
When thread is making excursion into another protection domain, its remaining
activation record is marked as "waiting for [idempotent] ipc".
It also marks the restarting point should IPC fail/require restart.
*/
/**
* IPC state.
*
* Per-thread state relating to IPC.
* FIXME: should be significantly simpler in Pebble/Metta than in Fluke.
**/
struct ipc_state
{
    /**
    * Flags defining the direction of the current reliable connection:
    * contains FLUKE_THREAD_CLIENT_SENDER and FLUKE_THREAD_SERVER_SENDER.
    * (This pair is defined below as THREAD_IPCFLAGS.)
    **/
    unsigned        flags;

    /**
    * Pointer to the server thread to which this client is attached,
    * or NULL if no active connection exists.
    * Invariant: server->client == this.
    * Invariant: server_link == NULL.
    **/
    thread*         server;

    /**
    * Reference to the client thread to which this server is attached.
    * Invariant: client->server == this.
    * Invariant: client_link == NULL.
    **/
    thread*         client;

    /**
    * Client and server links for this thread,
    * used when only a half-connection is present
    * or when the thread is stopped.
    * When the thread is live, these fields are thread-private;
    * when stopped, the fields are locked by the thread's s_ob lock.
    **/
    link            client_link;
    link            server_link;

    /**
    * Idempotent client/server connection pointers.
    * There are no pickled versions of these,
    * since breaking an idempotent connection just cancels it.
    **/
    thread*         idemp_server;
    thread*         idemp_client;

    /**
    * Server-side IPC entrypoint information,
    * set using fluke_thread_set_server_info().
    **/
    address_t       server_pset_uva;
    unsigned        server_esp;
    unsigned        server_oneway_eip;
    unsigned        server_idempotent_eip;
    unsigned        server_reliable_eip;

    /**
    * The effective SID of this thread in its role as
    * a server waiting on a port set.
    **/
    security_id_t   effective_server_sid;

    /**
    * The effective client SID of this thread's client.
    * The connection is only complete if this SID is identical
    * to the effective_client_sid field in the client's state.
    **/
    security_id_t   client_effective_client_sid;

    /**
    * The effective SID of this thread in its role as
    * a client in a reliable IPC connection.
    *
    * The effective client SID for an idempotent or
    * one-way IPC has no relationship to this effective
    * client SID.
    **/
    security_id_t   effective_client_sid;

    /**
    * The effective server SID of this thread's server.
    * The connection is only complete if this SID is identical
    * to the effective_server_sid field in the server's state.
    **/
    security_id_t   server_effective_server_sid;

    /**
    * A flag indicating whether or not this thread
    * wants to be supplied with the effective SID of
    * its client when it is awakened from wait_receive.
    * This flag is set to true when this thread
    * uses a wait_receive_secure call, and causes
    * a min_msg register to be used for the
    * effective client SID rather than for the
    * corresponding min_msg word.
    **/
    bool            server_wants_client_sid;

    /* Include machine-dependent state, if any. */
    /**
    * Save area for the minimum message received during IPC.
    * On architectures with a reasonable number of registers,
    * the min_msg just gets dropped straight into registers;
    * however, on the x86, we have to save them off elsewhere
    * and load them into registers at the very end of the receive
    * (see IPC_FINISH_RECEIVE(), below).
    **/
    unsigned                min_msg[2];

    /**
    * IPC parameters: only valid in certain wait states.
    * While in WAIT_IPC_CALL, recv params are valid, for the reply.
    * While in WAIT_IPC_ORECV, recv params are valid, for ack-send.
    **/
// receive and send buffers for an IPC operation.
//     fluke_ipc_params  params;
};

/**
* Sched state.
*
* Scheduler-dependent.
**/
struct sched_state
{
    thread *next; // simple FIFO scheduling
};

/**
* Per-thread machine-dependent thread layer state.
**/
struct thread_md
{
    /**
    * Exception handler entrypoints.
    **/
    unsigned        trap_handler;
    unsigned        interrupt_handler;
    unsigned        client_alert_handler;
    unsigned        server_alert_handler;

    /**
    * Exception state save area.
    **/
    unsigned        saved_eax;
    unsigned        saved_ecx;
    unsigned        saved_edx;
    unsigned        saved_eip;
    unsigned        saved_eflags;

#define S_THREAD_SYSCALL_ST_SIZE    8
    unsigned        syscall_st[S_THREAD_SYSCALL_ST_SIZE]; // debug
};

/**
* Thread - a sequential flow of execution.
*
* Thread is split into two coupled entities:
* - Activation Record - a flow of control and corresponding state, allowing to resume that flow,
*   execution context, including task, exception handler, stack, registers.
*   (related to this are disp_state, csw_state, exc_state)
*  - Activation Records stack up in piles when threads do jumps across task boundaries.
*  - Activation Record can be seen from within the task to represent and manipulate the thread.
* - Scheduling Record - a schedulable entity with corresponding priorities and
*   resource accounting.
*    (related to this are wait_state, task_state, ipc_state and sched_state)
*
* Threads start life as kernel entities.
*
* Process calls kernel to spawn a new thread, kernel initializes thread in it's own address space
* and then makes an upcall into the calling process from this new thread.
*
* Thread syncronization in passive objects can be implemented by either single activation record
* per-object (allowing no more than 1 thread to enter the passive object at a time) or by holding
* object-wide locks in places which modify the state of this object (more granular, less latency,
* more race issues).
*
* IPC is different from Fluke because there's only one thread involved in the IPC in general case.
* Networked IPC (network thread migration) might be slightly different (?).
*
* During IPC the portal code marks activation record status with idempotent IPC or
* current IPC stage.
**/
class thread : public object
{
public:
    thread() : object(object_type::thread) {}

    void run(); ///< Mark thread as runnable.
    void cancel(); ///< Stop the thread asap.

private:
    /**
    * Virtual address of the user part of the thread object
    * as mapped into the task the thread runs in.
    **/
//     address_t        thread_va;

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
    csw_state    csw_state_; // context switch state
//     exc_state    exc_state; // exception state
//     thread_disp_state   disp_state; // dispatch state
//     wait_state   wait_state; // wait state
//     task_state   task_state; // task state
//     ipc_state    ipc_state;  // ipc state
//     sched_state      sched_state; // scheduler state

    /*
     * When another thread is trying to stop and manipulate us,
     * it cancels us and then waits on this condition variable.
     */
//     cond     stop_cond;

    /*
     * When another thread is in a thread_wait waiting for us to
     * wake it up "by name", it puts itself on this queue.
     * When we are destroyed, we must wake all these threads
     * so they can restart and see the invalidity of their links to us.
     */
//     cond     death_cond;

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
//     unsigned        gflags;
// #ifndef CONFIG_OPT_THREADS
//     unsigned        lflags;
// #define th_lflags lflags
// #endif

//     security_class_t sclass;
//     security_id_t ssid;
//     security_class_t tclass;
//     security_id_t tsid;
//     access_vector_t requested;

//     unsigned                instrument_flags;
//     void *                  instrument_buffer;
//     unsigned                instrument_buffer_used;

    /* Kernel return stats */
//     struct kr_stats {
//         unsigned calls;
//         unsigned uexceptions;
//         unsigned pagefaults;
//         unsigned cancels;
//         unsigned restarts;
//         unsigned secfaults;
//     } kr_stats;

    /* Machine-dependent thread layer data. */
//     thread_md  md;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
