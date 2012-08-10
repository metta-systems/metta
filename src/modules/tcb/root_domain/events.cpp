#include "event_counts.h"
#include "doubly_linked_list.h"
#include "thread_v1_interface.h"
#include "vcpu_v1_interface.h"
#include "activation_dispatcher_v1_interface.h"
#include "threads_manager_v1_interface.h"
#include "thread_hooks_v1_interface.h"
#include "time_notify_v1_interface.h"
#include "channel_notify_v1_interface.h"
#include "events_v1_impl.h"
#include "module_interface.h"
#include "binder_v1_interface.h"
#include "exceptions.h"

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

struct instance_state_t;

struct qlink_t : public dl_link_t<qlink_t>
{
    dl_link_t<void>        timeq; 
    event_v1::value        wait_value;
    time_v1::time          wait_time;
    time_v1::time          block_time; /// for debugging.
    thread_v1::closure_t*  thread;
};

/* Per thread state; encapsulates a 'qlink_t' to block that thread. */
struct events_v1::state_t //per_thread_state_t
{
    events_v1::closure_t       events;      /// Per-thread events closure.
    time_notify_v1::closure_t  time_notify; /// For timeouts from dispatcher.
    instance_state_t*          inst_state;  /// Pointer to shared state.
    qlink_t                    qlink;       /// Used to block/unblock this thread.
};

struct event_count_t : public dl_link_t<event_count_t>
{
    event_v1::value                          value;          /// Current value.
    channel_v1::endpoint                     ep;             /// Attached endpoint, or NULL_EP.
    channel_v1::endpoint_type                ep_type;        /// Type of attached endpoint, or undefined.
    dl_link_t<channel_notify_v1::closure_t>  notifications;  /// Chained notification handlers.
    dl_link_t<qlink_t>                       wait_queue;     /// List of threads waiting on this event count.
    channel_notify_v1::closure_t             notify_closure; /// If attached, d_ops set to proper methods.
    instance_state_t*                        inst_state;

    event_count_t(instance_state_t* e_st)
    {
        value = 0;
        ep = NULL_EP;
        ep_type = channel_v1::endpoint_type_none;
        notifications.init();
        wait_queue.init();
        closure_init(&notify_closure, static_cast<channel_notify_v1::ops_t*>(nullptr), static_cast<channel_notify_v1::state_t*>(nullptr));
        inst_state = e_st;
    }
};

typedef event_v1::value sequencer_t;

/* State record for an instance of the event interface */
struct instance_state_t
{
    vcpu_v1::closure_t* vcpu;                        /// vcpu we are running on.
    activation_dispatcher_v1::closure_t* dispatcher; /// For channel notify hooks.
    threads_manager_v1::closure_t* thread_manager;   /// Handle to (un)block threads.
    thread_hooks_v1::closure_t thread_hooks;         /// To setup the per-thread state.
    heap_v1::closure_t*  heap;                       /// Our heap (NB: not locked).
    event_count_t        all_counts;                 /// All event counts in a list.
    dl_link_t<qlink_t>   time_queue;                 /// Things waiting for timeouts.
    events_v1::state_t*  exit_st;                    /// Events structure used for exit.
};

/**
 * vcpu critical sections.
 *
 * lock/unlock sections are nestable (@todo double check).
 *
 * If unlock enables activations, it should cause an activation if there
 * are pending events on incoming event channels, but neither lock nor
 * unlock should need a system call in the common case.
 *
 * lock/unlock sections protect vcpu state (such as context and
 * event allocation) as well as user-level scheduler state (such as the run and
 * blocked queues).
 */

class vcpu_lock_t
{
    vcpu_v1::closure_t* vcpu;
    bool reenable;
public:
    inline vcpu_lock_t(vcpu_v1::closure_t* vcpu_) : vcpu(vcpu_), reenable(false)
    {
        lock();
    }
    inline ~vcpu_lock_t()
    {
        unlock();
    }
    inline void lock()
    {
        reenable = vcpu->are_activations_enabled();
        if (reenable)
            vcpu->disable_activations();
    }
    inline void unlock()
    {
        if (reenable)
        {
            vcpu->enable_activations();
            if (vcpu->are_events_pending())
                vcpu->rfa();
        }
    }
};

//=====================================================================================================================
// Events helper functions.
//=====================================================================================================================

static void
unblock_event(instance_state_t* istate, event_count_t* event_count, bool alerted)
{
    while (!event_count->wait_queue.is_empty()
        && (alerted || EC_LE(event_count->wait_queue.next->wait_value, event_count->value)))
    {
        qlink_t *cur = event_count->wait_queue.next;

        cur->remove();

        // @todo: remove from timeq too...

        if (alerted)
            cur->thread->alert();

        istate->thread_manager->unblock_thread(cur->thread, /*in_cs:*/false);
    }
}

//=====================================================================================================================
// Events.
//=====================================================================================================================

static event_v1::count
events_create(events_v1::closure_t* self)
{
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);

    event_count_t* res = new(istate->heap) event_count_t(istate);
    // if (!res) RAISE_Events$NoResources(); @todo
    istate->all_counts.add_to_tail(res); // Add to the set of all counts.

    return res;
}

static void
events_destroy(events_v1::closure_t* self, event_v1::count ec)
{
    instance_state_t* istate  = self->d_state->inst_state;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);
    vcpu_lock_t lock(istate->vcpu);

    unblock_event(istate, event_count, /*alerted:*/true); // Alert all waiters on this event count.

    event_count->remove();

    if (event_count->ep != NULL_EP)
    {
        if (event_count->notifications.prev)
            event_count->notifications.prev->set_link(chained_handler_v1::position_after,
                reinterpret_cast<chained_handler_v1::closure_t*>(event_count->notifications.next));// oh, man.
        else
        {
            // We were the head of the notify queue, so attach the old handler (may be NULL) to the endpoint.
            istate->dispatcher->attach(event_count->notifications.next, event_count->ep);
        }

        if (event_count->notifications.next)
            event_count->notifications.next->set_link(chained_handler_v1::position_before,
                reinterpret_cast<chained_handler_v1::closure_t*>(event_count->notifications.prev));// oh, man.
    }
    lock.unlock();

    // The closedown call to the binder must be outside the critical section: it does a blocking IDC call.
    if (event_count->ep != NULL_EP)
    {
        PVS(binder)->close(event_count->ep);
        self->destroy_channel(event_count->ep);
    }

    // Finally, we can free the event count.
    lock.lock();
    istate->heap->free(reinterpret_cast<memory_v1::address>(event_count)); // oh, man.
}

/**
 * read_event must check any attached channel, since otherwise updates
 * are only propagated from the channel to the count on an activation,
 * and an activation will not occur unless someone is blocked on this
 * event count. This means that read_event can raise the same
 * exceptions as vcpu.poll, principally channel.bad_state.
 */
static event_v1::value
events_read(events_v1::closure_t* self, event_v1::count ec)
{
    vcpu_v1::closure_t* vcpu  = self->d_state->inst_state->vcpu;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);

    // This can be replaced simply with ep_type check, since I've added ep_type == none now... @todo
    if ((event_count->ep != NULL_EP) && (event_count->ep_type == channel_v1::endpoint_type_rx))
    {
        event_count->value = vcpu->poll(event_count->ep);
    }

    return event_count->value;
}

/**
 * Advance preserves vcpu activations mode. Thus, it can be
 * called within an activations-off critical section.
 */
static void
events_advance(events_v1::closure_t* self, event_v1::count ec, event_v1::value increment)
{
    instance_state_t* istate  = self->d_state->inst_state;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);
    vcpu_lock_t lock(istate->vcpu);

    event_count->value += increment;

    unblock_event(istate, event_count, /*alerted:*/false); // unblock awoken threads

    // If event is connected, send out the new value.
    if (event_count->ep_type == channel_v1::endpoint_type_tx)
    {
        OS_TRY {
            istate->vcpu->send(event_count->ep, event_count->value);
        }
        OS_FINALLY {
            lock.unlock();
        }
        OS_ENDTRY;
    }
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
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);

    sequencer_t* res = new(istate->heap) sequencer_t(0);
    // if (!res) RAISE_Events$NoResources(); @todo

    return res;
}

static void
events_destroy_sequencer(events_v1::closure_t* self, event_v1::sequencer seq)
{
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);
    istate->heap->free(reinterpret_cast<memory_v1::address>(seq)); // oh, man.
}

static event_v1::value
events_read_seq(events_v1::closure_t* self, event_v1::sequencer seq)
{
    return *reinterpret_cast<sequencer_t*>(seq);
}

static event_v1::value
events_ticket(events_v1::closure_t* self, event_v1::sequencer seq)
{
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);
    sequencer_t* s = reinterpret_cast<sequencer_t*>(seq);
    sequencer_t v = *s;
    ++(*s);
    return v;
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
