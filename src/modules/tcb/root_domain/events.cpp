#include "event_counts.h"
#include "doubly_linked_list.h"
#include "thread_v1_interface.h"
#include "vcpu_v1_interface.h"
#include "activation_dispatcher_v1_interface.h"
#include "threads_manager_v1_interface.h"
#include "thread_hooks_v1_interface.h"
#include "time_notify_v1_interface.h"
#include "channel_notify_v1_interface.h"
#include "channel_notify_v1_impl.h"
#include "events_v1_impl.h"
#include "module_interface.h"
#include "binder_v1_interface.h"
#include "exceptions.h"
#include "time_macros.h"
#include "heap_new.h"

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
#define NULL_EVENT 0

//=====================================================================================================================
// Events types.
//=====================================================================================================================

struct instance_state_t;

struct qlink_t
{
    dl_link_t<qlink_t>     waitq;
    dl_link_t<qlink_t>     timeq;
    event_v1::value        wait_value;
    time_v1::time          wait_time;
    time_v1::time          block_time; /// for debugging.
    thread_v1::closure_t*  thread;

    inline qlink_t()
    {
        waitq.init(this);
        timeq.init(this);
    }
};

/* Per thread state; encapsulates a 'qlink_t' to block that thread. */
struct events_v1::state_t //per_thread_state_t
{
    events_v1::closure_t       events;      /// Per-thread events closure.
    time_notify_v1::closure_t  time_notify; /// For timeouts from dispatcher.
    instance_state_t*          inst_state;  /// Pointer to shared state.
    qlink_t                    qlink;       /// Used to block/unblock this thread.
};

struct event_count_t
{
    dl_link_t<event_count_t>                 ec_queue;       /// Link for all counts queue.
    event_v1::value                          value;          /// Current value.
    channel_v1::endpoint                     ep;             /// Attached endpoint, or NULL_EP.
    channel_v1::endpoint_type                ep_type;        /// Type of attached endpoint, or undefined.
    channel_notify_v1::closure_t*            prev_notify;    /// Chained notification handlers - one that calls us.
    channel_notify_v1::closure_t*            next_notify;    /// Chained notification handlers - the one we call after us.
    channel_notify_v1::closure_t             notify_closure; /// If attached, d_ops set to proper methods.
    dl_link_t<qlink_t>                       wait_queue;     /// List of threads waiting on this event count.
    instance_state_t*                        inst_state;

    event_count_t(instance_state_t* e_st)
    {
        value = 0;
        ep = NULL_EP;
        ep_type = channel_v1::endpoint_type_none;
        ec_queue.init(this);
        prev_notify = next_notify = nullptr;
        wait_queue.init(/*this:duh, @todo*/);
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

channel_notify_v1::ops_t notify_methods =
{
    nullptr,
    nullptr
};

//=====================================================================================================================
// Events helper functions.
//=====================================================================================================================

/**
 * block_event:
 *    enqueue the current thread "current" waiting for "value" on event "event_count",
 *    (and, if "until" != FOREVER, waiting for the timeout "until").
 *    Then block & yield the current thread (i.e. self).
 *    Will return from yield once the thread is
 *       (a) unblocked, and
 *       (b) run again.
 *    Return true iff we were unblocked via being alerted.
 *
 *  pre:  event_count.value < value && (until == FOREVER || NOW() < until)
 *  post: unblocked.
 */

static bool
block_event(events_v1::state_t* state, event_count_t* event_count, thread_v1::closure_t* thread,
            event_v1::value value, time_v1::time until)
{
    instance_state_t* istate  = state->inst_state;
    qlink_t* current = &state->qlink;
    bool alerted = false;

    if (state == istate->exit_st)
    {
        state->qlink.thread = thread;
    }

    current->wait_value = value;
    current->wait_time = until;
    current->block_time = NOW();

    if (event_count)
    {
        // Find insertion point in waitq.
        dl_link_t<qlink_t>* wqp = event_count->wait_queue.next();

        while (wqp != &event_count->wait_queue && (*wqp)->wait_value <= value)
            wqp = wqp->next();

        wqp->add_to_tail(current->waitq);
    }
    else
        current->waitq.init(); // use reset() or clear_links() maybe?

    if (until != FOREVER)
    {
        if (!istate->dispatcher->add_timeout(&state->time_notify, until, current))
        {
            // Timeout passed while we were adding it.
            if (current->waitq.next())
            {
                current->waitq.remove();
                current->waitq.init();
                current->timeq.init();
            }
            return alerted;
        }

        dl_link_t<qlink_t>* tqp = istate->time_queue.next();

        while (tqp != &istate->time_queue && (*tqp)->wait_time <= until)
            tqp = tqp->next();

        tqp->add_to_tail(current->timeq);
    }
    else
        current->timeq.init();

    // Now we block the thread in the user-level scheduler, and yield.
    alerted = istate->thread_manager->block_yield(until);

    return alerted;
}

static void
unblock_event(instance_state_t* istate, event_count_t* event_count, bool alerted)
{
    while (!event_count->wait_queue.is_empty()
        && (alerted || EC_LE((*event_count->wait_queue.next())->wait_value, event_count->value)))
    {
        qlink_t *cur = *event_count->wait_queue.next();

        cur->waitq.remove();

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
    if (!res)
        OS_RAISE((exception_support_v1::id)"events_v1.no_resources", 0);

    istate->all_counts.ec_queue.add_to_tail(res->ec_queue);

    return res;
}

static void
events_destroy(events_v1::closure_t* self, event_v1::count ec)
{
    instance_state_t* istate  = self->d_state->inst_state;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);
    vcpu_lock_t lock(istate->vcpu);

    unblock_event(istate, event_count, /*alerted:*/true); // Alert all waiters on this event count.

    event_count->ec_queue.remove();

    if (event_count->ep != NULL_EP)
    {
        if (event_count->prev_notify)
            event_count->prev_notify->set_link(chained_handler_v1::position_after,
                reinterpret_cast<chained_handler_v1::closure_t*>(event_count->next_notify));// oh, man.
        else
        {
            // We were the head of the notify queue, so attach the old handler (may be nullptr)
            // to the endpoint.
            istate->dispatcher->attach(event_count->next_notify, event_count->ep);
        }

        if (event_count->next_notify)
            event_count->next_notify->set_link(chained_handler_v1::position_before,
                reinterpret_cast<chained_handler_v1::closure_t*>(event_count->prev_notify));// oh, man.
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
    instance_state_t* istate  = self->d_state->inst_state;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);
    bool alerted = false;

    if (EC_LE(value, event_count->value))
        return event_count->value;

    istate->thread_manager->enter_critical_section(/*vcpu_cs:*/true);

    // If we're receiving, sync up incoming event counts.
    if (event_count->ep_type == channel_v1::endpoint_type_rx)
    {
        channel_v1::endpoint_type ep_type;
        event_v1::value rx_val, rx_ack;

        /*
         * We must first get into a consistent state wrt. external
         * events, which may arrive at any time. The aim is to get to
         * the stage where we both have a value for the count, and are
         * guaranteed that the FIFO will register the count going above
         * that value.
         */
        while (true)
        {
            // Read count and ack values.
            istate->vcpu->query_channel(event_count->ep, &ep_type, &rx_val, &rx_ack);

            if (EC_GT(rx_val, value) || (rx_val == rx_ack))
                break;

            istate->vcpu->ack(event_count->ep, rx_val);
        }

        /*
         * For receiving end events, the following condition now holds:
         * (with ep = event_count->ep)
         *
         * (rx_val <= ep->value) && (rx_val == ep->ack || rx_val >= value)
         *
         * This is required to ensure that events are not lost by the scheduler.
         */

        // Finally update the local value and unblock awoken threads.
        event_count->value = rx_val;
        unblock_event(istate, event_count, /*alerted:*/false);
    }

    event_v1::value result = event_count->value;

    /*
     * We now block if value < ec->value. This test also takes care of the
     * local case when ec was advanced after the test outside the
     * critical section above.
     */

    if (EC_GT(value, result))
    {
        if (block_event(self->d_state, event_count, PVS(thread), value, FOREVER))
            alerted = true; // => "event_count" might have been freed,
        else
            result = event_count->value;
    }

    istate->thread_manager->leave_critical_section();

    if (alerted)
        OS_RAISE((exception_support_v1::id)"thread_v1.alerted", 0);

    return result;
}

static event_v1::value
events_await_until(events_v1::closure_t* self, event_v1::count ec, event_v1::value value, time_v1::time until)
{
    instance_state_t* istate  = self->d_state->inst_state;
    bool alerted = false;

    if (until < 0)
        until = FOREVER;

    if (ec == NULL_EVENT)
    {
        if (IN_FUTURE(until))
        {
            istate->thread_manager->enter_critical_section(/*vcpu_cs:*/true);

            if (block_event(self->d_state, nullptr, PVS(thread), value, until))
                alerted = true;

            istate->thread_manager->leave_critical_section();
        }

        if (alerted)
            OS_RAISE((exception_support_v1::id)"thread_v1.alerted", 0);

        return 0;
    }

    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);

    if (EC_LE(value, event_count->value) || until <= NOW())
        return event_count->value;

    istate->thread_manager->enter_critical_section(/*vcpu_cs:*/true);

    // If we're receiving, sync up incoming event counts.
    if (event_count->ep_type == channel_v1::endpoint_type_rx)
    {
        channel_v1::endpoint_type ep_type;
        event_v1::value rx_val, rx_ack;

        /*
         * We must first get into a consistent state wrt. external
         * events, which may arrive at any time. The aim is to get to
         * the stage where we both have a value for the count, and are
         * guaranteed that the FIFO will register the count going above
         * that value.
         */
        while (true)
        {
            // Read count and ack values.
            istate->vcpu->query_channel(event_count->ep, &ep_type, &rx_val, &rx_ack);

            if (EC_GT(rx_val, value) || (rx_val == rx_ack) || !IN_FUTURE(until))
                break;

            PVS(threads)->yield();
            istate->vcpu->ack(event_count->ep, rx_val);
        }

        /*
         * For receiving end events, the following condition now holds:
         * (with ep = event_count->ep)
         *
         * (rx_val <= ep->value) && (rx_val == ep->ack || rx_val >= value || until >= NOW)
         *
         * This is required to ensure that events are not lost by the scheduler.
         */

        // Finally update the local value and unblock awoken threads.
        event_count->value = rx_val;
        unblock_event(istate, event_count, /*alerted:*/false);
    }

    event_v1::value result = event_count->value;

    /*
     * We now block if value < ec->value. This test also takes care of the
     * local case when ec was advanced after the test outside the
     * critical section above.
     */

    if (EC_GT(value, result) && IN_FUTURE(until))
    {
        if (block_event(self->d_state, event_count, PVS(thread), value, until))
            alerted = true; // => "event_count" might have been freed,
        else
            result = event_count->value;
    }

    istate->thread_manager->leave_critical_section();

    if (alerted)
        OS_RAISE((exception_support_v1::id)"thread_v1.alerted", 0);

    return result;
}

/**
 * Sequencers.
 */

static event_v1::sequencer
events_create_sequencer(events_v1::closure_t* self)
{
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);

    sequencer_t* res = new(istate->heap) sequencer_t(0);
    if (!res)
        OS_RAISE((exception_support_v1::id)"events_v1.no_resources", 0);

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

/**
 * Events and Channels.
 */

static void
events_attach(events_v1::closure_t* self, event_v1::count ec, channel_v1::endpoint channel, channel_v1::endpoint_type type)
{
    instance_state_t* istate  = self->d_state->inst_state;
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);

    OS_TRY {
        channel_v1::endpoint_type ep_type;
        event_v1::value rx_val, rx_ack;

        istate->thread_manager->enter_critical_section(/*vcpu_cs:*/true);

        channel_v1::state state = istate->vcpu->query_channel(event_count->ep, &ep_type, &rx_val, &rx_ack);

        if (state == channel_v1::state_connected)
        {
            if (ep_type != type)
                OS_RAISE((exception_support_v1::id)"channel_v1.invalid", channel);
        }
        else
            if (state != channel_v1::state_local)
                OS_RAISE((exception_support_v1::id)"channel_v1.bad_state", /*channel, state*/0);

        if (event_count->ep_type != channel_v1::endpoint_type_none)
            OS_RAISE((exception_support_v1::id)"channel_v1.invalid", 0); //ec); // Already attached.

        event_count->ep = channel;
        event_count->ep_type = type;

        if (type == channel_v1::endpoint_type_rx)
        {
            closure_init(&event_count->notify_closure, &notify_methods, reinterpret_cast<channel_notify_v1::state_t*>(event_count));
            event_count->next_notify = istate->dispatcher->attach(&event_count->notify_closure, channel);
            if (event_count->next_notify)
            {
                event_count->next_notify->set_link(chained_handler_v1::position_before, reinterpret_cast<chained_handler_v1::closure_t*>(&event_count->notify_closure));
            }
            event_count->prev_notify = nullptr;
        }

        if (state == channel_v1::state_connected)
        {
            if (type == channel_v1::endpoint_type_rx)
            {
                event_count->value = rx_val;
                unblock_event(istate, event_count, false);
            }
            else
            {
                istate->vcpu->send(event_count->ep, event_count->value);
            }
        }
    }
    OS_FINALLY {
        istate->thread_manager->leave_critical_section();
    }
    OS_ENDTRY;
}

static void
events_attach_pair(events_v1::closure_t* self, event_v1::pair events, channel_v1::pair channels)
{
    instance_state_t* istate  = self->d_state->inst_state;

    OS_TRY {
        channel_v1::state rx_state, tx_state;
        channel_v1::endpoint_type ep_type;
        event_v1::value rx_val, rx_ack;
        event_v1::value tx_val, tx_ack;
        event_count_t* rx = reinterpret_cast<event_count_t*>(events.receiver);
        event_count_t* tx = reinterpret_cast<event_count_t*>(events.sender);

        istate->thread_manager->enter_critical_section(/*vcpu_cs:*/true);

        tx_state = istate->vcpu->query_channel(channels.sender, &ep_type, &tx_val, &tx_ack);

        if (tx_state == channel_v1::state_connected)
        {
            if (ep_type != channel_v1::endpoint_type_tx)
                OS_RAISE((exception_support_v1::id)"channel_v1.invalid", channels.sender);
        }
        else
            if (tx_state != channel_v1::state_local)
                OS_RAISE((exception_support_v1::id)"channel_v1.bad_state", /*channel, state*/0);

        rx_state = istate->vcpu->query_channel(channels.receiver, &ep_type, &rx_val, &rx_ack);

        if (rx_state == channel_v1::state_connected)
        {
            if (ep_type != channel_v1::endpoint_type_rx)
                OS_RAISE((exception_support_v1::id)"channel_v1.invalid", channels.receiver);
        }
        else
            if (rx_state != channel_v1::state_local)
                OS_RAISE((exception_support_v1::id)"channel_v1.bad_state", /*channel, state*/0);

        if (tx->ep_type != channel_v1::endpoint_type_none)
            OS_RAISE((exception_support_v1::id)"channel_v1.invalid", 0); //tx); // Already attached.

        if (rx->ep_type != channel_v1::endpoint_type_none)
            OS_RAISE((exception_support_v1::id)"channel_v1.invalid", 0); //rx); // Already attached.

        tx->ep = channels.sender;
        tx->ep_type = channel_v1::endpoint_type_tx;

        rx->ep = channels.receiver;
        rx->ep_type = channel_v1::endpoint_type_rx;

        // Initialize receiving end.
        closure_init(&rx->notify_closure, &notify_methods, reinterpret_cast<channel_notify_v1::state_t*>(rx));
        rx->next_notify = istate->dispatcher->attach(&rx->notify_closure, channels.receiver);
        if (rx->next_notify)
        {
            rx->next_notify->set_link(chained_handler_v1::position_before, reinterpret_cast<chained_handler_v1::closure_t*>(&rx->notify_closure));
        }
        rx->prev_notify = nullptr;

        if (rx_state == channel_v1::state_connected)
        {
            rx->value = rx_val;
            unblock_event(istate, rx, false);
        }

        if (tx_state == channel_v1::state_connected)
        {
            istate->vcpu->send(tx->ep, tx->value);
        }
    }
    OS_FINALLY {
        istate->thread_manager->leave_critical_section();
    }
    OS_ENDTRY;
}

static channel_v1::endpoint
events_query_endpoint(events_v1::closure_t* self, event_v1::count ec, channel_v1::endpoint_type* type)
{
    if (ec == NULL_EVENT)
        OS_RAISE((exception_support_v1::id)"events_v1.invalid", 0);//ec);

    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);

    // Inside the critical section, assign to temporaries that we know can't page fault.
    event_count_t* event_count = reinterpret_cast<event_count_t*>(ec);
    channel_v1::endpoint_type type_tmp = event_count->ep_type;
    channel_v1::endpoint ep_tmp = event_count->ep;
    lock.unlock();

    *type = type_tmp;
    return ep_tmp;
}

static channel_v1::endpoint
events_create_channel(events_v1::closure_t* self)
{
    instance_state_t* istate  = self->d_state->inst_state;
    channel_v1::endpoint result = NULL_EP;
    vcpu_lock_t lock(istate->vcpu);

    OS_TRY {
        result = istate->vcpu->allocate_channel();
        auto old_notify = PVS(dispatcher)->attach(nullptr, result);
        if (old_notify)
            PANIC("Newly allocated channel is already attached!");
    }
    OS_FINALLY {
        lock.unlock();
    }
    OS_ENDTRY;

    return result;
}

static void
events_destroy_channel(events_v1::closure_t* self, channel_v1::endpoint channel)
{
    instance_state_t* istate  = self->d_state->inst_state;
    vcpu_lock_t lock(istate->vcpu);

    OS_TRY {
        auto old_notify = PVS(dispatcher)->attach(nullptr, channel);
        if (old_notify)
            PANIC("Trying to free channel which is still attached!");
        istate->vcpu->release_channel(channel);
    }
    OS_FINALLY {
        lock.unlock();
    }
    OS_ENDTRY;
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
