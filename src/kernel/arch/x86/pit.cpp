#include "cpu.h"
#include "infopage.h"
#include "timer_impl.h"

// Based on http://wiki.osdev.org/Programmable_Interval_Timer

#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42
#define PIT_MCR 0x43

// MCR bits 6-7
#define MCR_CH0 (0 << 6)
#define MCR_CH1 (1 << 6)
#define MCR_CH2 (2 << 6)
#define MCR_READ_BACK (3 << 6)
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

struct timer_state
{
    time_ns now, alarm;
};

// Timer ops.
static time_ns read(timer_closure* self)
{
    return self->state->now;
}

static void set(timer_closure* self, time_ns time)
{
    self->state->alarm = time;
}

static time_ns clear(timer_closure* /*self*/, time_ns* /*itime*/)
{
    return 0;
}

static void enable(timer_closure* /*self*/)
{
//     interrupt_descriptor_table_t::instance().set_interrupt(?, ?);
}

// Timer closure set up.

static timer_ops ops = {
    read,
    set,
    clear,
    enable
};

// Timer state is in the infopage.
static timer_closure timer;// = { methods: &ops, state: INFO_PAGE_ADDR };

timer_closure* init_timer()
{
    init_pit(100);
    timer.methods = &ops;
    timer.state = reinterpret_cast<timer_state*>(INFO_PAGE_ADDR);
    return &timer;
}
