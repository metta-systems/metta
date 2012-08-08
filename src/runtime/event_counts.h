#pragma once

#include "event_v1_interface.h"
#include "events_v1_interface.h"

class event_counter_t
{
	event_v1::count c;
public:
	inline event_counter_t()
	{
		c = PVS(events)->create();
	}
	// event_count_t(const event_count_t& other)
	// {
	// 	c = other.c;
	// }
	inline ~event_counter_t()
	{
		PVS(events)->destroy(c);
	}

	inline event_v1::value read() { return PVS(events)->read(c); }
	inline void advance(event_v1::value n) { PVS(events)->advance(c, n); }
	inline void await(event_v1::value v) { PVS(events)->await(c, v); }
	inline void await_until(event_v1::value v, event_v1::value t) { PVS(events)->await_until(c, v, t); }
	// void sleep_until(event_v1::value t);
	// void sleep(time_v1::time ns) { sleep_until(NOW() + ns); }
};

class event_sequencer_t
{
	event_v1::sequencer s;
public:
	inline event_sequencer_t()
	{
		s = PVS(events)->create_sequencer();
	}
	inline ~event_sequencer_t()
	{
		PVS(events)->destroy_sequencer(s);
	}

	inline event_v1::value read() { return PVS(events)->read_seq(s); }
	inline event_v1::value ticket() { return PVS(events)->ticket(s); }
};

// SRC mutex is non-recursive
// Posix mutex is slightly more tricky as it needs thread-owner ID. See R.J.Black Fawn paper for discussion and implementation.
class mutex_t
{
	event_counter_t e;
	event_sequencer_t s;
public:
	inline mutex_t() : e(), s() { e.advance(1); } // s with value 0 and e with value 1 are the start condition

	inline void lock() { e.await(s.ticket()); }
	inline void unlock() { e.advance(1); }
};

class condition_t
{
	event_counter_t e;
	event_sequencer_t s;
public:
	inline condition_t() : e(), s() {}

	inline void wait(mutex_t& m)
	{
		event_v1::value t = s.ticket();
		m.unlock();
		e.await(t);
		m.lock();
	}
	inline void signal()
	{
		e.advance(1);
	}
	inline void broadcast()
	{
		event_v1::value t = s.read();
		event_v1::value f = e.read();
		if (t > (f+1))
			e.advance(t-f-1);
	}
};
