//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//

// vcpu_context_t is machine dependent.
// vcpu_t is not.
#include "machine/vcpu_context.h"

/*
 * Virtual processor interface.
 */
struct vcpu_t
{
    void (*activate)();
    uint32_t activation_status;
    uint32_t activations_disabled;
    vcpu_context_t execution_context;
    vcpu_context_t activation_context;
    char* kernel_call_shm; // TODO: kernel should check these pointers point to legal process-owned memory
    char* upcall_shm;

    /*
    * Activation status flags.
    */
    enum {
        STATUS_ACTIVATION_MASK    = 0xff,
        STATUS_PREEMPTED          = 0x01,
        STATUS_ALLOCATED          = 0x02,
        STATUS_EXTRA              = 0x04,
        STATUS_EVENTS_DELIVERED   = 0x08,

        STATUS_NEW_EVENTS_PENDING = 0x100,
        STATUS_ACTIVATION_RUNNING = 0x200
    };
};

/*
 * Process has a VCPU interface pointer, which has vcpu_t structure address as instance pointer
 * and syscalls methods table as methods pointer.
 *
 * This allows to treat vcpu uniformly as an interface instance and implement portal manager
 * optimizations on syscalls as needed.
 */
#include "syscalls.h"

struct vcpu_interface
{
    vcpu_t* instance;
    vcpu_methods* methods; // may be generated from IDL? now just defined in syscalls.h
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
