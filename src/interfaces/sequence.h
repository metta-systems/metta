#pragma once

// #include "types.h"

template<class T>
class sequence
{
	T* base;
	T* data;
	size_t len;
	size_t blen;

public:
	sequence(int)// silly shadowing ctor for now
	{}
};
