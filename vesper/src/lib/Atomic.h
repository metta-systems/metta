#pragma once

class Atomic
{
public:
	static Address exchange(Address *lock, Address newVal);
};
