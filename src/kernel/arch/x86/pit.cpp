//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "cpu.h"
#include "infopage.h"
#include "timer_v1_interface.h"
#include "timer_v1_impl.h"
#include "default_console.h"

// Based on http://wiki.osdev.org/Programmable_Interval_Timer

// Driver specification in Chorus OS:
// http://docs.sun.com/app/docs/doc/806-3343/6jcgfleep?l=sv&a=view

#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42
#define PIT_MCR 0x43

// MCR bits 6-7
#define MCR_CH0         (0 << 6)
#define MCR_CH1         (1 << 6)
#define MCR_CH2         (2 << 6)
#define MCR_READ_BACK   (3 << 6)
// MCR bits 4-5
#define MCR_LATCH_COUNT (0 << 4)
#define MCR_LOBYTE      (1 << 4)
#define MCR_HIBYTE      (2 << 4)
#define MCR_LOHI        (3 << 3)
// MCR bits 1-3 - operating mode
#define MCR_OP_INTR_TERM_COUNT (0 << 1)
#define MCR_OP_HW_ONESHOT      (1 << 1)
#define MCR_OP_RATE_GENERATOR  (2 << 1)
#define MCR_OP_SQUARE_WAVE     (3 << 1)
#define MCR_OP_SW_STROBE       (4 << 1)
#define MCR_OP_HW_STROBE       (5 << 1)
#define MCR_OP_RATE_GEN2       (6 << 1) // Same as RATE_GENERATOR
#define MCR_OP_SQUARE_WAVE2    (7 << 1) // Same as SQUARE_WAVE
// MCR bit 0: 1 = bcd, 0 = 16 bit hex
#define MCR_BCD_MODE           (1 << 0)

static void init_pit(int hz)
{
    int divisor = 1193180 / hz;
    // Set channel 0 to mode 0, write both lo and hi bytes of reload value.
    x86_cpu_t::outb(PIT_MCR, MCR_CH0 | MCR_LOHI | MCR_OP_INTR_TERM_COUNT);
    x86_cpu_t::outb(PIT_CH0, divisor & 0xff);
    x86_cpu_t::outb(PIT_CH0, (divisor >> 8) & 0xff);
}

struct timer_v1::state_t : information_page_t
{
};

// Timer ops.
static time_v1::ns read(timer_v1::closure_t* self)
{
    return self->d_state->now;
}

static void arm(timer_v1::closure_t* self, time_v1::ns time)
{
    self->d_state->alarm = time;
}

static time_v1::ns clear(timer_v1::closure_t* /*self*/, time_v1::ns* /*itime*/)
{
    return 0;
}

static void enable(timer_v1::closure_t* /*self*/, uint32_t /*sirq*/)
{
    kconsole << "timer.enable()" << endl;
//     interrupt_descriptor_table_t::instance().set_interrupt(?, ?);
}

// Timer closure set up.

static const timer_v1::ops_t ops = 
{
    read,
    arm,
    clear,
    enable
};

// Timer state is in the infopage.
static timer_v1::closure_t timer =
{
    &ops,
    reinterpret_cast<timer_v1::state_t*>(information_page_t::ADDRESS)
};

timer_v1::closure_t* init_timer()
{
    kconsole << "Initializing interrupt timer." << endl;
    init_pit(100);
    return &timer;
}


// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
