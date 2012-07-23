#pragma once
#include "event_v1_interface.h"

class event_count_t
{
	event_v1::value v;
public:
	event_count_t() : v(0) {}

	event_v1::value read();
	void await(event_v1::value v);
	void await_until(event_v1::value v, event_v1::value t);
	void advance(event_v1::value n);
	void sleep_until(event_v1::value t);
};

class event_sequencer_t
{
	event_v1::value v;
public:
	event_sequencer_t() : v(0) {}

	event_v1::value read();
	event_v1::value ticket();
};

// SRC mutex is non-recursive
// Posix mutex is slightly more tricky as it needs thread-owner ID. See R.J.Black Fawn paper for discussion and implementation.
class mutex_t
{
	event_count_t e;
	event_sequencer_t s;
public:
	inline mutex_t() : e(), s() { e.advance(1); } // s with value 0 and e with value 1 are the start condition

	inline void lock() { e.await(s.ticket()); }
	inline void unlock() { e.advance(1); }
};

class condition_t
{
	event_count_t e;
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
