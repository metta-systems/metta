/*
	ZRingBuffer.hpp
	Author: Chris Ertel <crertel@762studios.com>
	Created: 12/01/2011

	Purpose: 

	Templated array-backed dequeue implementation.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZRINGBUFFER_HPP
#define _ZRINGBUFFER_HPP

#include <ZSTL/ZArray.hpp>

//Default local storage for ring buffer
#ifndef ZRINGBUFFER_DEFAULT_LOCAL_STORAGE
#define ZRINGBUFFER_DEFAULT_LOCAL_STORAGE (32)
#endif

//Default capacity for ring buffer
#ifndef ZRINGBUFFER_DEFAULT_CAPACITY
#define ZRINGBUFFER_DEFAULT_CAPACITY (10)
#endif

/////////////////
/* ZRingBuffer */
/////////////////
/*
Templated array-backed dequeue implementation.

The template parameter T is the type contained in the buffer.

The template parameter N is the size to use for the local array backing the buffer.
The number of elements able to be stored in the buffer is fixed, and will not change
unless requested to by use of the Resize() function.
*/
template <typename T, size_t N = ZRINGBUFFER_DEFAULT_LOCAL_STORAGE>
class ZRingBuffer
{
protected:
	// maximum number of elements that can be stored in the ring buffer
	size_t Capacity;	

	// current number of elements in the ring buffer
	size_t Size;

	// index into the backing array of the front element of the ring buffer
	size_t FrontIndex;

	// index into the backing array of the back element of the ring buffer
	size_t BackIndex;

	// backing array of the ring buffer
	ZArray<T, N> Buffer;

	//Integrity Check
	inline void CheckIntegrity() const
	{
		#if ZSTL_CHECK_INTEGRITY
		ZSTL_ASSERT(this->FrontIndex < this->Capacity, "ZRingBuffer Error: Front index exceeds capacity!");
		ZSTL_ASSERT(this->BackIndex < this->Capacity, "ZRingBuffer Error: Back index exceeds capacity!");
		#endif
	}

public:
	/*	 
	Default constructor for ZRingBuffer.
	 
	@return ()
	*/
	ZRingBuffer();

	/*	 
	Parameterized constructor for ZRingBuffer. Allows specification of capacity.
	 
	@param _capacity - initial capacity for the ring buffer.
	@return ()
	*/
	ZRingBuffer(size_t _capacity);

	/*	 
	Parameterized constructor to use for ZRingBuffer. Allows assignment of storage backing.
	 
	@param _storage - ZArray to use as backing.
	@return ()
	*/
	ZRingBuffer(ZArray<T>& _storage);

	/*
	Virtual destructor.
	*/
	virtual ~ZRingBuffer();

	/*
	public ZRingBuffer<T>::IsEmpty
	 
	Tests whether or not the ring buffer is empty.
	 
	@return (bool) - true is empty, false if not empty.
	*/
	bool IsEmpty();

	/*
	public ZRingBuffer<T>::IsFull
	 
	Tests whether or not the ring buffer is full.
	 
	@return (bool) - true if full, false if not full.
	*/
	bool IsFull();

	/*
	public ZRingBuffer<T>::GetCapacity
	 
	Gets the capacity of the ring buffer.

	Attempts to push more elements than this value will trigger an assert.
	 
	@return (size_t) - the capacity of the ring buffer.
	*/
	size_t GetCapacity();

	/*
	public ZRingBuffer<T>::GetSize
	 
	Gets the size of the ring buffer--that is, the current number of elements in it.

	Attempts to pop more elements than this value will trigger an assert.
	 
	@return (size_t) - the number of elements in the ring buffer.
	*/
	size_t GetSize();

	/*
	public ZRingBuffer<T>::Reserve
	 
	Changes the capacity of the backing array to accommodate the given size.
	Existing items in the ring buffer will be moved over without complaint.

	If the new capacity is smallar than the current number of elements, an assertion is triggered.
	If the new capacity is greater than the current number of elements but smaller than the current capacity,
	the backing array is resized.
	If the new capacity is identical to the current capacity, nothing is done.
	If the new capacity is greater than the current capacity, the backing array is replaced with a larger one.
	 
	@param _newCapacity - new capacity to reserve in ring buffer.
	@return (void)
	*/
	void Reserve(size_t _newCapacity);

	/*
	public ZRingBuffer<T>::CopyToArray
	 
	Function to copy the contents from the ring buffer into the target array, starting at the back index
	and working forward to the front.

	Note that this does no bounds checking--the array must be sufficiently large to hold all the elements.
	 
	@param _dest - array to copy elements to.
	@return (void)
	*/
	void CopyToArray(T* _dest);

	/*
	public ZRingBuffer<T>::GetBackingArrayCopy
	 
	Gets a copy of the ZArray backing this ring buffer.
	 
	@return (ZArray<T>) - copy of the array backing this ring buffer.
	*/
	ZArray<T> GetBackingArrayCopy();

	/*
	public ZRingBuffer<T>::CanPush
	 
	Checks to see if pushes will succeed on this ring buffer (either front or back).
	 
	@return (bool) - true if pushes are possible, false if not.
	*/
	bool CanPush();

	/*
	public ZRingBuffer<T>::PushFront
	 
	Pushes an element onto the front of this ring buffer.
	
	Note that an assertion will be triggered if this the ring buffer would overflow.

	@param _in - element to push onto front of ring buffer.
	@return (void)
	*/
	void PushFront(const T& _in);

	/*
	public ZRingBuffer<T>::PushBack
	 
	Pushes an element onto the back of this ring buffer.

	Note that an assertion will be triggered if this the ring buffer would overflow.
	 
	@param _in - element to push onto back of ring buffer
	@return (void)
	*/
	void PushBack(const T& _in);

	/*
	public ZRingBuffer<T>::TryPushFront
	 
	Tries to push an element onto the ring buffer, returning success or failure.

	If the element is unable to be pushed onto the buffer, no change is made to the buffer.
	 
	@param _in - element to push onto front of ring buffer.
	@return (bool) - true if push was successful, false if not.
	*/
	bool TryPushFront(const T& _in);

	/*
	public ZRingBuffer<T>::TryPushBack
	 
	 Tries to push an element onto the ring buffer, returning success or failure.

	 If the element is unable to be pushed onto the buffer, no change is made to the buffer.
	 
	@param _in - element to push onto the back of the ring buffer.
	@return (bool) - true if push was successful, false if not.
	*/
	bool TryPushBack(const T& _in);

	/*
	public ZRingBuffer<T>::CanPeek
	 
	Checks to see if peeks will succeed on this ring buffer (either front or back).
	 
	@return (bool) - true if a peek would succeed, false otherwise.
	*/
	bool CanPeek();

	/*
	public ZRingBuffer<T>::PeekFront
	 
	Gets a reference to the front element of the buffer. Does not change buffer otherwise.

	If the buffer is not peekable (because it is empty, etc.), this will trigger an assertion.
	 
	@return (T&) - reference to front element in buffer.
	*/
	T& PeekFront();

	/*
	public ZRingBuffer<T>::PeekBack
	 
	Gets a reference to the back element of the buffer. Does not change buffer otherwise.

	If the buffer is not peekable (because it is empty, etc.), this will trigger an assertion.
	 
	@return (T&) - reference to back element in buffer.
	*/
	T& PeekBack();

	/*
	public ZRingBuffer<T>::CanPop
	 
	Checks to see if pops will succeed on this ring buffer (either front or back).
	 
	@return (bool) - true if pops will succeed, false otherwise.
	*/
	bool CanPop();

	/*
	public ZRingBuffer<T>::PopFront
	 
	Function to pop the front element off of the ring buffer.

	If the buffer is not poppable (because it is empty, etc.), this will trigger an assertion.
	 
	@return (T) - copy of former front element in buffer.
	*/
	T PopFront();

	/*
	public ZRingBuffer<T>::PopBack
	 
	 Function to pop the back element off of the ring buffer.

	 If the buffer is not poppable (because it is empty, etc.), this will trigger an assertion.
	 
	@return (T) - copy of the former back element in buffer.
	*/
	T PopBack();	
};

template <typename T, size_t N>
ZRingBuffer<T, N>::ZRingBuffer()
: Buffer(ZRINGBUFFER_DEFAULT_CAPACITY), Capacity(0), Size(0), FrontIndex(0), BackIndex(0)
{
	this->Buffer.Resize(ZRINGBUFFER_DEFAULT_CAPACITY);
}

template <typename T, size_t N>
ZRingBuffer<T, N>::ZRingBuffer(size_t _capacity)
: Buffer(_capacity), Capacity(_capacity), Size(0), FrontIndex(0), BackIndex(0)
{
	this->Buffer.Resize(_capacity);
}

template <typename T, size_t N>
ZRingBuffer<T, N>::ZRingBuffer(ZArray<T>& _storage)
: Buffer(_storage), Capacity(_storage.Capacity()), Size(0), FrontIndex(0), BackIndex(1)
{
	this->Buffer.Resize(_storage.Capacity());
}

template <typename T, size_t N>
ZRingBuffer<T, N>::~ZRingBuffer()
{
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::IsFull()
{
	return this->Size == this->Capacity;
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::IsEmpty()
{
	return this->Size == 0;
}

template <typename T, size_t N>
size_t ZRingBuffer<T, N>::GetCapacity()
{
	return this->Capacity;
}

template <typename T, size_t N>
size_t ZRingBuffer<T, N>::GetSize()
{
	return this->Size;
}

template <typename T, size_t N>
ZArray<T> ZRingBuffer<T, N>::GetBackingArrayCopy()
{
	return this->Buffer;
}

template <typename T, size_t N>
void ZRingBuffer<T, N>::Reserve(size_t _newCapacity)
{
	// if the new and current capacities are equal, do nothing.
	if (this->Capacity == _newCapacity)
		return;

	// if the new capacity is smaller than the current size, assert.
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size <= this->Capacity, "ZRingBuffer::PushFront() exceeded capacity!\n");
	#endif

	// make a new backing, copy over elements, etc.
	ZArray<T> tempBuffer = this->GetBackingArrayCopy();
	this->Buffer.Resize(_newCapacity);
	
	size_t capacityCopy = this->Capacity;
	size_t currIndex = this->BackIndex;

	for (size_t i = 0; i < this->Size; i++)
	{
		this->Buffer[i] = tempBuffer[currIndex];
		if (currIndex == capacityCopy - 1)
			currIndex = 0;
		else
			currIndex++;
	}

	// setup new metadata
	this->BackIndex = 0;
	this->FrontIndex = this->Size - 1;
	this->Capacity = _newCapacity;
}

template <typename T, size_t N>
void ZRingBuffer<T, N>::CopyToArray(T* _dest)
{
	size_t currIndex = 0;
	currIndex = this->BackIndex;
	for (size_t numToCopy = 0; numToCopy < this->Size; numToCopy++)
	{
		if (currIndex == this->Capacity)
			currIndex = 0;
		_dest[numToCopy] = this->Buffer[currIndex];
		currIndex++;
	}
}

template <typename T, size_t N>
void ZRingBuffer<T, N>::PushFront(const T& _in)
{
	this->Size++;
	
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size <= this->Capacity, "ZRingBuffer::PushFront() exceeded capacity!\n");
	#endif

	// increment front index
	this->FrontIndex++;
	if (this->FrontIndex >= this->Capacity )
		this->FrontIndex = 0;

	// calculate back index
	if (this->FrontIndex - this->BackIndex > this->Size)
	{
		// the front index must have wrapped around
		// backbit is the number of spaces in from of the back of the buffer to the end of the array
		// remainder is the number of spaces from the beginning of the array to the front of the buffer
		size_t backbit = (this->Capacity) - this->BackIndex;
		size_t remainder = (this->Size - 1) - backbit;
		this->FrontIndex = remainder;
	}
	else
	{
		//no wrapping happened, so calculate directly
		this->BackIndex = this->FrontIndex - (this->Size - 1);
	}

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size <= this->Capacity, "ZRingBuffer::PushBack() exceeded capacity!\n");
	#endif

	this->Buffer[this->FrontIndex] = _in;

	CheckIntegrity();
}

template <typename T, size_t N>
void ZRingBuffer<T, N>::PushBack(const T& _in)
{
	this->Size++;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size <= this->Capacity, "ZRingBuffer::PushBack() exceeded capacity!\n");
	#endif

	// decrement back index
	if (this->BackIndex  == 0)
		this->BackIndex = this->Capacity - 1;
	else
		this->BackIndex--;

	// calculate front index
	if (this->BackIndex + (this->Size-1) >= this->Capacity)
	{
		// wrapping will occur

		// the front index must have wrapped around
		// backbit is the number of spaces in from of the back of the buffer to the end of the array
		// remainder is the number of spaces from the beginning of the array to the front of the buffer
		size_t frontbit = this->Capacity - this->BackIndex;
		size_t remainder = (this->Size - 1) - frontbit;
		this->FrontIndex = remainder;
	}
	else
	{
		//wrapping no occurred
		this->FrontIndex = this->BackIndex + (this->Size - 1);
	}

	this->Buffer[this->BackIndex] = _in;

	CheckIntegrity();
}

template <typename T, size_t N>
T ZRingBuffer<T, N>::PopFront()
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size > 0, "ZRingBuffer::PopFront() caused underflow!\n");
	#endif

	T temp = this->Buffer[this->FrontIndex];
	if (this->FrontIndex == 0)
		this->FrontIndex = this->Capacity - 1;
	else
		this->FrontIndex--;
	this->Size--;

	CheckIntegrity();
	return temp;
}

template <typename T, size_t N>
T ZRingBuffer<T, N>::PopBack()
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT( this->Size > 0, "ZRingBuffer::PopBack() caused underflow!\n");
	#endif
	
	T temp = this->Buffer[this->BackIndex];
	
	if (this->BackIndex == this->Capacity - 1)
		this->BackIndex = 0;
	else
		this->BackIndex++;
	
	this->Size--;

	CheckIntegrity();
	
	return temp;
}

template <typename T, size_t N>
T& ZRingBuffer<T, N>::PeekFront()
{
	return this->Buffer[this->FrontIndex];
}

template <typename T, size_t N>
T& ZRingBuffer<T, N>::PeekBack()
{
	return this->Buffer[this->BackIndex];
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::CanPush()
{
	return this->Size < this->Capacity;
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::CanPeek()
{
	return this->Size > 0;
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::CanPop()
{
	return this->Size > 0;
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::TryPushFront(const T& _in)
{
	if (this->CanPush() == false)
		return false;
	
	this->PushFront(_in);
	
	return true;
}

template <typename T, size_t N>
bool ZRingBuffer<T, N>::TryPushBack(const T& _in)
{
	if (this->CanPush() == false)
		return false;
	
	this->PushBack(_in);
	
	return true;
}

#endif
