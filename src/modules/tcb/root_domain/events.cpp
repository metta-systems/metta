#include "event_counts.h"
#include "doubly_linked_list.h"
#include "thread_v1_interface.h"
#include "vcpu_v1_interface.h"
#include "activation_dispatcher_v1_interface.h"
#include "threads_manager_v1_interface.h"
#include "thread_hooks_v1_interface.h"
#include "time_notify_v1_interface.h"
#include "events_v1_impl.h"

/* 
 * Eventcount and Sequencer stuff
 */

/*
 * The following 4 macros do the right thing even if event values have
 * wrapped, on the ASSUMPTION that the values being compared never
 * differ by 2^(word-size - 1) or more.  NB. "Event_Val"s are incremented
 * as UNsigned quantities and their differences are compared with 0 here as
 * SIGNED quantities.
 */

#define EC_LT(ev1,ev2)           (((int64_t) ((ev1) - (ev2))) < 0)
#define EC_LE(ev1,ev2)           (((int64_t) ((ev1) - (ev2))) <= 0)
#define EC_GT(ev1,ev2)           (((int64_t) ((ev1) - (ev2))) > 0)
#define EC_GE(ev1,ev2)           (((int64_t) ((ev1) - (ev2))) >= 0)

#define NULL_EP 0

//=====================================================================================================================
// Events types.
//=====================================================================================================================

struct qlink_t
{
    dl_link_t<void>        waitq;
    dl_link_t<void>        timeq; 
    event_v1::value        wait_value;
    time_v1::time          wait_time;
    time_v1::time          block_time; /// for debugging.
    thread_v1::closure_t*  thread;
};

/* State record for an instance of the event interface */
struct instance_state_t
{
    vcpu_v1::closure_t* vcpu;                        /// vcpu we are running on.
    activation_dispatcher_v1::closure_t* dispatcher; /// For channel notify hooks.
    threads_manager_v1::closure_t* thread_manager;   /// Handle to (un)block threads.
    thread_hooks_v1::closure_t thread_hooks;         /// To setup the per-thread state.
    heap_v1::closure_t*  heap;                       /// Our heap (NB: not locked).
    event_counter_t      all_counts;                 /// All event counts in a list.
    dl_link_t<void>      time_queue;                 /// Things waiting for timeouts.
    events_v1::state_t*  exit_st;                    /// Events structure used for exit.
};

/* Per thread state; encapsulates a 'qlink_t' to block that thread. */
struct events_v1::state_t //per_thread_state_t
{
    events_v1::closure_t events; /// Per-thread events closure.
    time_notify_v1::closure_t time_notify; /// For timeouts from dispatcher.
    instance_state_t* evt_st;     /// Pointer to shared state.
    qlink_t         qlink;      /// Used to block/unblock this thread.
};

struct event_count_t : public dl_link_t<event_count_t>
{
    event_v1::value  value;   /// Current value.
    channel_v1::endpoint ep;  /// Attached endpoint, or NULL_EP.
    channel_v1::endpoint_type ep_type; /// Type of attached endpoint, or undefined.

    // ChannelNotify_clp next_cn;    /* Chained notification handler       */
    // ChannelNotify_clp prev_cn;    /* Previous notification handler      */
    // link_t           wait_queue;  /* list of threads waiting on this ec */
    // ChannelNotify_cl cn_cl;       /* op != NULL iff attached            */
    instance_state_t* evt_st;

    event_count_t()
    {
        value = 0;
        ep = NULL_EP;
        ep_type = channel_v1::endpoint_type_none;
    }
};

//=====================================================================================================================
// Events.
//=====================================================================================================================

static event_v1::count
events_create(events_v1::closure_t* self)
{
    instance_state_t* istate  = self->d_state->evt_st;

    // vcpu_lock_t lock(events->vcpu);
    event_count_t* res = new(istate->heap) event_count_t;
    // lock.unlock();

    return res;
}

static void
events_destroy(events_v1::closure_t* self, event_v1::count ec)
{

}

static event_v1::value
events_read(events_v1::closure_t* self, event_v1::count ec)
{
    return 0;
}

static void
events_advance(events_v1::closure_t* self, event_v1::count ec, event_v1::value increment)
{

}

static event_v1::value
events_await(events_v1::closure_t* self, event_v1::count ec, event_v1::value value)
{
    return 0;
}

static event_v1::value
events_await_until(events_v1::closure_t* self, event_v1::count ec, event_v1::value value, time_v1::time until)
{
    return 0;
}

static event_v1::sequencer
events_create_sequencer(events_v1::closure_t* self)
{
    return 0;
}

static void
events_destroy_sequencer(events_v1::closure_t* self, event_v1::sequencer seq)
{

}

static event_v1::value
events_read_seq(events_v1::closure_t* self, event_v1::sequencer seq)
{
    return 0;
}

static event_v1::value
events_ticket(events_v1::closure_t* self, event_v1::sequencer seq)
{
    return 0;
}

static void
events_attach(events_v1::closure_t* self, event_v1::count ec, channel_v1::endpoint channel, channel_v1::endpoint_type type)
{

}

static void
events_attach_pair(events_v1::closure_t* self, event_v1::pair events, channel_v1::pair channels)
{

}

static channel_v1::endpoint
events_query_endpoint(events_v1::closure_t* self, event_v1::count ec, channel_v1::endpoint_type* type)
{
    return 0;
}

static channel_v1::endpoint
events_create_channel(events_v1::closure_t* self)
{
    return 0;
}

static void
events_destroy_channel(events_v1::closure_t* self, channel_v1::endpoint channel)
{

}

events_v1::ops_t events_methods = 
{
    events_create,
    events_destroy,
    events_read,
    events_advance,
    events_await,
    events_await_until,
    events_create_sequencer,
    events_destroy_sequencer,
    events_read_seq,
    events_ticket,
    events_attach,
    events_attach_pair,
    events_query_endpoint,
    events_create_channel,
    events_destroy_channel
};
