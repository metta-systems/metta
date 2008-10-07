#pragma once

#include "Atomic.h"

/**
 * A class that implements a spinlock / binary semaphore.
 */
class Lockable
{
public:
	Lockable()
	{
		lock = 0;
	}

	~Lockable() {}

	// Spin until we get the lock.
	void getLock()
	{
		uint32_t newVal = 1;
		// If we exchange the lock value with 1 and get 1 out, it was locked.
		while (Atomic::exchange(&lock, newVal) == 1)
		{
			// Do nothing.
		}
		// We got the lock, return.
	}

	bool tryLock()
	{
		// Spin once.
		uint32_t newVal = 1;
		if (Atomic::exchange(&lock, newVal) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool testLock()
	{
		return lock;
	}

	void releaseLock()
	{
		lock = 0;
	}

private:
	/**
	 * The actual lock variable
	 */
	uint32_t lock;
};

