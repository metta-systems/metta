/*
	ZArray.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 9/12/2010

	Purpose:	
	
	Templated dynamic array implementation.

	Defining the following features to 1 enables the feature.  Defining to 0 disables the
	feature (default behavior if undefined).

	ZSTL_CHECK_INTEGRITY
	Checks integrity of the ZSTL containers after allocations or if non-const functions are called.  
	Used for debugging ZSTL.  Useful if new methods are added.

	ZSTL_DISABLE_RUNTIME_CHECKS
	Disables runtime bounds and error checking on ZArray and ZArray Iterators.

	ZSTL_DISABLE_NEGATIVE_INDEXING
	Disables negative indexing on ZArrays - using negative indices will always result in an
	out of bounds runtime error.
	
	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZARRAY_HPP
#define _ZARRAY_HPP

#include <ZSTL/ZSTLCommon.hpp>

//When passed as template parameter for N, causes ZArray to use allocator only and not local storage
#define ZARRAY_USE_ALLOCATOR (1)

//Local storage number of elements (user can define their own)
#ifndef ZARRAY_LOCAL_STORAGE_COUNT
#define ZARRAY_LOCAL_STORAGE_COUNT (32)
#endif

//Default capacity for ZArray when no capacity is defined (recommended that this be less than or equal to local storage count, user can define their own)
#ifndef ZARRAY_DEFAULT_CAPACITY
#define ZARRAY_DEFAULT_CAPACITY (10)
#endif

//Resize factor for ZArray when operations are performed that require an increase in capacity
//and capacity is not specified (user can define their own, should be greater than 1.0)
#ifndef ZARRAY_CAPACITY_RESIZE_FACTOR
#define ZARRAY_CAPACITY_RESIZE_FACTOR (1.5)
#endif

///////////////////////
/* ZArray Allocators */
///////////////////////

/*
ZArray Allocator class.  Used to allocate arrays of a templated type 
when requested by a ZArray instance.

The template parameter T is the type contained the array this allocator is for.
*/
template <typename T>
class ZArrayAllocator
{
public:
	//Virtual Destructor
	virtual ~ZArrayAllocator() { }

	/*
	virtual public ZArrayAllocator<T>::Allocate

	Allocation method.

	@param _size - size of the array to allocate
	@return - pointer to an array which can store at least _size values
	*/
	virtual T* Allocate(size_t _size) = 0;

	/*
	virtual public ZArrayAllocator<T>::Clone

	Clone method.  Required to return a copy of this allocator.
	
	@return - identical allocator
	*/
	virtual ZArrayAllocator<T>* Clone() = 0;
	
	/*
	virtual public ZArrayAllocator<T>::Deallocate

	Deallocation method.

	@param _ptr - pointer to previously allocated memory by this allocator
	@param _size - size of the memory
	*/
	virtual void Deallocate(T* _ptr, size_t _size) = 0;

	/*
	virtual public ZArrayAllocator<T>::Destroy

	Destroy method.  Called when the allocator is no longer needed by the Array.
	Heap allocated allocators should delete themselves (suicide).

	@return (void)
	*/
	virtual void Destroy() = 0;
};

/////////////////////
/* ZArray Iterator */
/////////////////////

/*
Iterator class for ZArray.

The template parameter T is the type contained in the list this iterator is for.
*/
template <typename T>
class ZArrayIterator : public ZIterator<T>
{
private:
	//The array
	T* Array;

	//The current index into the array
	int Index;

	//The size of the array (at construction)
	size_t Size;

public:
	/*
	Default constructor.
	*/
	ZArrayIterator() 
		: Array(NULL), Index(0), Size(0) { }

	/*
	Copy constructor.
	
	@param _other - the other iterator
	*/
	ZArrayIterator(const ZArrayIterator& _other) 
		: Array(_other.Array), Index(_other.Index), Size(_other.Size) { CheckCurrent(); }

	/*
	Parameterized Constructor.

	@param _array - the array this iterator points to
	@param _index - the index into the array at which to start the iterator.
	@param _size - the size of the array we are pointed to
	*/
	ZArrayIterator(T *_array, int _index, size_t _size) 
		: Array(_array), Index(_index), Size(_size) { CheckCurrent(); }
	
	/*
	public ZArrayIterator<T>::CheckCurrent
	
	Check function that determines if the iterator is valid at it's current location. 

	@param _endIsValid - indicates that the iterator can be at the 'end' location
	@return (void)
	*/
	void CheckCurrent(bool _endIsValid = true) const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Array != NULL,								
			"Uninitialized ZArray Iterator used!");
		ZSTL_ASSERT(Index >= 0,									
			"ZArray Iterator has gone past beginning of array!");
		ZSTL_ASSERT(Index < (int)Size + _endIsValid ? 1 : 0,	
			"ZArray Iterator has gone past end of array!");
		#endif
	}

	/*
	public ZArrayIterator<T>::CheckNext
	
	Check function that determines if incrementing the iterator would be valid,
	assuming it was already valid.
	
	@param _inc - the amount by which the iterator will be incremented
	@return (void)
	*/
	void CheckNext(int _inc) const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Index + _inc <= (int)Size, "ZArray Iterator has gone past end of array!");
		#endif
	}

	/*
	public ZArrayIterator<T>::CheckPrevious
	
	Check function that determines if decrementing the iterator would be valid,
	assuming it was already valid.

	@param _dec - the amount by which the iterator will be decremented
	@return (void)
	*/
	void CheckPrevious(int _dec) const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Index - _dec >= 0, "ZArray Iterator has gone past beginning of array!");
		#endif
	}

	//Subclass override
	virtual T& Get() const										{ return *(*this); }

	//Subclass override
	virtual bool HasCurrent() const								{ return Index >= 0 && Index < (int)Size; }

	//Subclass override
	virtual bool HasNext() const								{ return Index < (int)Size - 1; }

	//Subclass override
	virtual bool HasPrev() const								{ return Index > 0; }

	//Subclass override
	virtual void Next()											{ ++(*this); }

	//Subclass override
	virtual void Prev()											{ --(*this); }

	//Iterator Operator Overrides
	ZArrayIterator operator +(const int _value)					{ return ZArrayIterator(Array, Index + _value, Size); }
	ZArrayIterator& operator +=(const int _value)				{ CheckNext(_value); Index += _value; return *this; }
	ZArrayIterator& operator ++()								{ CheckNext(1); Index++; return *this; }
	ZArrayIterator operator ++(int)								{ CheckNext(1); return ZArrayIterator(Array, Index++, Size); }
	ZArrayIterator operator -(const int _value)					{ return ZArrayIterator(Array, Index - _value, Size); }
	ZArrayIterator& operator -=(const int _value)				{ CheckPrevious(_value); Index -= _value; return *this; }
	ZArrayIterator& operator --()								{ CheckPrevious(1); Index--; return *this; }
	ZArrayIterator operator --(int)								{ CheckPrevious(1); return ZArrayIterator(Array, Index--, Size); }
	ZArrayIterator& operator =(const ZArrayIterator &_other)	{ Array = _other.Array; Index = _other.Index; Size = _other.Size; CheckCurrent(); return *this; }
	bool operator ==(const ZArrayIterator &_other) const		{ return Index == _other.Index; }
	bool operator !=(const ZArrayIterator &_other) const		{ return !(Index == _other.Index); }
	bool operator <(const ZArrayIterator &_other) const			{ return (Index < _other.Index); }
	bool operator <=(const ZArrayIterator &_other) const		{ return (Index <= _other.Index); }
	bool operator >(const ZArrayIterator &_other) const			{ return (Index > _other.Index); }
	bool operator >=(const ZArrayIterator &_other) const		{ return (Index >= _other.Index); }
	T& operator *() const										{ CheckCurrent(false); return Array[Index]; }
	operator int() const										{ return Index; }
};

////////////
/* ZArray */
////////////

/*
Templated dynamic array implementation.

The template parameter T is the type contained in the array.

The template parameter N is the size of local storage.  ZArray will always 
use local storage unless capacity goes above the local storage size.  By 
setting N to ZARRAY_USE_ALLOCATOR, the ZArrayAllocator instance will 
always be used.
*/
template <typename T, size_t N = ZARRAY_LOCAL_STORAGE_COUNT>
class ZArray
{
protected:
	//The 'size' of the raw array pointer
	size_t ArrayCapacity;

	//The current number of contained elements in the array
	size_t ArraySize;

	//The raw array pointer
	T *Array;

	//Local array storage
	T Local[N];

	//Allocator for the Array
	ZArrayAllocator<T> *AllocatorInstance;

	//Makes a call to the allocator, but checks to ensure we aren't local
	inline void AllocateCheckLocal(size_t _size)
	{
		if (_size > ZARRAY_USE_ALLOCATOR || _size > N)
		{
			//If we haven't been assigned an allocator, use ZSTL_NEW_ARRAY / ZSTL_DELETE_ARRAY
			if (AllocatorInstance != NULL)
				Array = AllocatorInstance->Allocate(_size);
			else
				Array = ZSTL_NEW_ARRAY(T, _size);

			#if !ZSTL_DISABLE_RUNTIME_CHECKS
			ZSTL_ASSERT(Array != NULL, "ZArrayAllocator failed to properly allocate array! (returned NULL)");
			#endif
		}
		else
			Array = Local;
	}

	//Makes a call to the allocator, but checks to ensure we aren't local
	inline void DeallocateCheckLocal(T* _array, size_t _size)
	{
		if (_array != Local)
		{
			if (AllocatorInstance != NULL)
			{
				AllocatorInstance->Deallocate(_array, _size);
			}
			else
			{
				ZSTL_DEL_ARRAY _array;
			}
		}
	}

	//Function that gives an absolute  index into Array given a signed index and verifies
	//we didn't go out of bounds.
	inline size_t AbsoluteIndex(int _index, int _boundary) const
	{
		#if ZSTL_DISABLE_NEGATIVE_INDEXING

			#if !ZSTL_DISABLE_RUNTIME_CHECKS
			ZSTL_ASSERT(_index >= 0 && _index < _boundary, "ZArray out of bounds access!");
			#endif

		return _index;

		#else

		int index = _index < 0 ? ArraySize + _index : _index;

			#if !ZSTL_DISABLE_RUNTIME_CHECKS
			ZSTL_ASSERT(index < _boundary, "ZArray out of bounds access!");
			#endif

		return index;

		#endif
	}

	//Integrity Check
	inline void CheckIntegrity() const
	{
		#if ZSTL_CHECK_INTEGRITY

		ZSTL_ASSERT(Array != NULL, "ZArray Error: Array is invalid!");
		ZSTL_ASSERT(ArrayCapacity >= ArraySize, "ZArray Error: Array capacity less than size!");

		#endif

		#if ZSTL_CHECK_INTEGRITY && defined(_MSC_VER)

		ZSTL_ASSERT(Array != (void*)0xfeeefeee && Array != (void*)0xcdcdcdcd, "ZArray Error: Array is invalid!");

		#endif
	}

public:
	/*
	Used when an index is to be returned to indicate an invalid index.
	*/
	const static int InvalidPos = -1;

	/*
	Typedef for ZArrayIterator (Allows ZArray<T, N>::Iterator notation).
	*/
	typedef ZArrayIterator<T> Iterator;

	/*
	Default Constructor.  Uses new / delete to allocate storage.
	*/
	ZArray();

	/*
	Parameterized Constructor.  Uses new / delete to allocate storage.

	@param _capacity - the starting capacity of the array
	*/	
	ZArray(size_t _capacity);

	/*
	Parameterized Constructor.

	@param _capacity - the starting capacity of the array
	@param _allocator - the allocator to allocate storage with
	*/
	ZArray(size_t _capacity, ZArrayAllocator<T> *_allocator);

	/*
	Copy Constructor.  Makes a deep copy of the array.

	@param _other - the array to copy from
	*/
	ZArray(const ZArray<T, N>& _other);

	/*
	Copy Constructor.  Makes a deep copy of the array with different local storage size.

	@param _other - the array to copy from
	*/
	template <size_t M>
	ZArray(const ZArray<T, M>& _other);

	/*
	Destructor.
	*/
	~ZArray();

	/*
	[] Operator overload.  Gets a value from the array.

	@param _index - the integer index into the array
	@return (T&) - reference to a value contained in the array
	*/
	T& operator [] (const int _index) const;

	/*
	+ Operator overload.  Concatenates two arrays.

	@param _other - the array to concatenate with
	@return (ZArray<T, N>) - an array containing the elements of this array
		concatenated with the elements of the other
	*/
	template <size_t M>
	ZArray<T, N> operator + (const ZArray<T, M>& _other) const;

	/*
	+= Operator overload.  Concatenates two arrays.

	@param _other - the array to concatenate with
	@return (ZArray<T, N>&) - this array
	*/
	template <size_t M>
	ZArray<T, N>& operator += (const ZArray<T, M>& _other);

	/*
	= Operator overload.  Sets this array equal to the other by making
	a copy.

	@param _other - the array to be set equal to
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& operator = (const ZArray<T, N>& _other);

	/*
	= Operator overload.  Sets this array equal to the other by making
	a copy (Different local storage sizes).

	@param _other - the array to be set equal to
	@return (ZArray<T, N>&) - this array
	*/
	template <size_t M>
	ZArray<T, N>& operator = (const ZArray<T, M>& _other);

	/*
	== Operator overload.  Performs an element comparison on the two arrays.

	@param _other - the array to compare to
	@return (bool) - true if this array is equivalent to the other
	*/
	template <size_t M>
	bool operator == (const ZArray<T, M>& _other) const;
	
	/*
	!= Operator overload.  Performs an element comparison on the two arrays.

	@param _other - the array to compare to
	@return (bool) - false if this array is equivalent to the other
	*/
	template <size_t M>
	bool operator != (const ZArray<T, M>& _other) const;

	/*
	T* Cast overload.  Allows this array to be cast into a T*.

	@return (T*) - the array contents
	*/
	operator T*() const;

	/*
	public ZArray<T, N>::AbsoluteIndex

	Function that gives an absolute index into Array given a signed index and verifies
	we didn't go out of bounds.

	@param _index - signed index
	@return (size_t) -  index
	*/
	size_t AbsoluteIndex(int _index) const;

	/*
	public ZArray<T, N>::Allocator

	Returns a reference to the current allocator.

	@return (ZArrayAllocator<T>&) - current allocator instance
	*/
	ZArrayAllocator<T>& Allocator() const;

	/*
	public ZArray<T, N>::Allocator

	Swaps out the current allocator with a new instance.  Requires
	reallocation of the entire array using the new allocator.

	@param _allocator - the new allocator to use
	@return (void)
	*/
	void Allocator(ZArrayAllocator<T> *_allocator);

	/*
	public ZArray<T, N>::Begin

	Returns an iterator to the beginning of the array.

	@return (ZArray<T, N>::Iterator) - Iterator with index zero
	*/
	Iterator Begin() { return ZArrayIterator<T>(Array, 0, ArraySize); }

	/*
	public ZArray<T, N>::Capacity

	Returns the capacity of the array, which is the number of values it can contain
	before allocation occurs.

	@return (size_t) - array capacity
	*/
	size_t Capacity() const;

	/*
	public ZArray<T, N>::Clear

	Clears out the array of all contained elements, keeping the currently allocated 
	storage.

	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Clear();
	
	/*
	public ZArray<T, N>::ClearAndFree

	Clears out the array of all contained elements and returns allocated memory
	to the system.  Reallocates the internal storage.

	@param _newCapacity - the capacity of the array after clearing
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& ClearAndFree(size_t _newCapacity = ZARRAY_DEFAULT_CAPACITY);

	/*
	public ZArray<T, N>::Contains

	Determines if the array contains the given value.

	@param _value - the value to search for
	@param _index - the index to start searching at
	@return (int) - true if found, false otherwise
	*/
	bool Contains(const T& _value, int _index = 0) const;

	/*
	public ZArray<T, N>::Count

	Returns the number of occurrences of the given value at and after the given index.

	@param _value - the value to search for 
	@param _index - the index to start at
	@return (size_t) - the number of occurrences of _value at and after _index
	*/
	size_t Count(const T& _value, int _index = 0) const;

	/*	
	public ZArray<T, N>::Empty

	Returns true if the array is empty (size 0), false otherwise.
	
	@return (bool) - true if empty, false otherwise
	*/
	bool Empty() const;

	/*
	public ZArray<T, N>::End

	Returns an iterator to the end of the array.

	@return (ZArray<T, N>::Iterator) - Iterator with index set to capacity
	*/
	Iterator End() { return ZArrayIterator<T>(Array, ArraySize, ArraySize); }

	/*
	public ZArray<T, N>::Erase

	Erase function.  Removes an element from the array at the given index.

	@param _index - the index at which to remove the element from the array.
	@return (T) - the element removed
	*/
	T Erase(int _index);

	/*
	public ZArray<T, N>::Erase

	Erase function.  Removes elements from the array between the given indices.

	@param _i - first index
	@param _j - second index (exclusive)
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Erase(int _i, int _j);

	/*
	public ZArray<T, N>::Erase

	Erase function.  Removes an element from the array at the given iterator
	between the given iterators.  End iterator is set to start
	iterator after function.

	@param _start - first iterator
	@param _end - last iterator (exclusive)
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Erase(Iterator& _start, Iterator& _end);

	/*
	public ZArray<T, N>::Find

	Find function.  Looks for the first occurrence of a value after the 
	specified index.  Returns ZArray<T, N>::InvalidPos if not found.

	@param _value - the value to look for
	@param _index - the index to start looking at
	@return (int) - the index of the first match
	*/
	int Find(const T& _value, int _index = 0) const;

	/*
	public ZArray<T, N>::Insert

	Insert function.  Inserts the given value in the specified location.

	@param _index - the index at which to perform the insertion
	@param _value - the value to insert
	@param _count - the number of times to insert _value
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Insert(int _index, const T& _value, size_t _count = 1);
	
	/*
	public ZArray<T, N>::Insert

	Insert function.  Inserts the given value in the location specified by
	the iterator given.  The iterator remains valid, and will point to the next 
	location after the inserted values.

	@param _itr - the iterator pointing to the location at which to perform the insertion
	@param _value - the value to insert
	@param _count - the number of times to insert _value
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Insert(Iterator& _itr, const T& _value, size_t _count = 1);

	/*
	public ZArray<T, N>::Insert

	Insert function.  Inserts the given array into the specified location.  Copies
	the values into the array.

	@param _index - the index at which to perform the insertion
	@param _other - the array to insert
	@return (ZArray<T, N>&) - this array
	*/
	template <size_t M>
	ZArray<T, N>& Insert(int _index, const ZArray<T, M>& _other);

	/*
	public ZArray<T, N>::Insert

	Insert function.  Inserts the given array into the location specified by the 
	iterator given.  The iterator remains valid, and will point to the next location after
	the inserted values.

	@param _itr - the iterator pointing to the location at which to perform the insertion
	@param _other - the array to insert
	@return (ZArray<T, N>&) - this array
	*/
	template <size_t M>
	ZArray<T, N>& Insert(Iterator& _index, const ZArray<T, M>& _other);

	/*
	public ZArray<T, N>::Peek

	Peek function.  Gives a reference to the last element in the array.

	@return (T&) - reference to the last element in the array
	*/
	T& Peek();

	/*
	public ZArray<T, N>::Pop

	Pop function.  Removes and returns the last element in the array.

	@return (T) - the last element in the array
	*/
	T Pop();

	/*
	public ZArray<T, N>::Push

	Push function.  Attaches an element to the end of the array.

	@param _element - the element to place at the end of the array.
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Push(const T& _element);

	/*
	public ZArray<T, N>::Remove

	Removes the first occurrence of the given element.

	@param _element - the element to remove
	@param _index - the index to start at (if negative, searches backwards)
	@return (int)- the index at which the first occurrence was removed from, ZArray<T>::InvalidPos if no occurence
	*/
	int Remove(const T& _element);

	/*
	public ZArray<T, N>::Remove

	Removes the first occurrence of the given element after the given index.

	@param _element - the element to remove
	@param _index - the index to start at (if negative, searches backwards)
	@return (int) - the index at which the first occurrence was removed from, ZArray<T>::InvalidPos if no occurrence
	*/
	int Remove(const T& _element, int _index);

	/*
	public ZArray<T, N>::RemoveAll

	Removes all instances of the given element from the array after the given index.

	@param _element - the element to remove
	@param _index - the index to start at (if negative, searches backwards)
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& RemoveAll(const T& _element, int _index = 0);

	/*
	public ZArray<T, N>::Reserve

	Reserves an amount of space in the vector.  Allocates ZNew space if necessary.

	@param _capacity - the new capacity requested
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Reserve(size_t _capacity);

	/*
	public ZArray<T, N>::Resize

	Resize function.  Increases or Decreases the size and capacity of the array.
	
	@param _size - the new size to grow the array to
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Resize(size_t _size);

	/*
	public ZArray<T, N>::Resize

	Resize function.  Increases or Decreases the size and capacity of the array and
	sets new values equal to the given value.  The array capacity cannot be reduced
	in size below the number of contained elements.
	
	@param _size - the new size to grow the array to
	@param _value - the new value to set all added values to
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Resize(size_t _size, const T& _value);

	/*
	public ZArray<T, N>::Shift

	Adds an element to the beginning of the array.  Requires moving all the array elements
	up in the array, so use with care.

	@param _element - the new element to add to the front of the array
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Shift(const T& _element);

	/*
	public ZArray<T, N>::Size

	Size function.  Give the size of the array.

	@return (size_t) - the size of this array
	*/
	size_t Size() const;

	/*
	public ZArray<T, N>::Slice

	Returns a portion of the array indicated by the given indices.

	@param _i - where to begin the slice
	@param _j - where to end the slice (exclusive)
	@return (ZArray<T, N>) - an array containing the elements between indices _i and _j
	*/
	ZArray<T, N> Slice(int _i, int _j) const;

	/*
	public ZArray<T, N>::Sort

	Sorts the array in place.

	@param _comparator - an optional comparator used to sort the contained values
	@param _algorithm - the array sort algorithm to use to sort the array
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Sort(const ZComparator<T>& _comparator = ZComparator<T>(), const ZArraySortAlgorithm<T>& _algorithm = ZArrayQuickSort<T>());

	/*
	public ZArray<T, N>::Sort<S>
	
	Sorts the array in place using an algorithm defined via template parameter.
	
	@param A - the ZArraySortAlgorithm type to use
	@param _comparator - an optional comparator used to sort the contained values.
	@return (ZArray<T, N>&) - this array
	*/
	template <typename A>
	ZArray<T, N>& Sort(const ZComparator<T>& _comparator);

	/*
	public ZArray<T, N>::Sort
	
	Sorts the array in place using the algorithm referenced.
	
	@param C - the ZComparator type to use
	@param _algorithm - the ZArraySortAlgorithm to use
	@return (ZArray<T, N>&) - this array
	*/
	template <typename C>
	ZArray<T, N>& Sort(const ZArraySortAlgorithm<T>& _algorithm);

	/*
	public ZArray::Sort<C, S>
	
	Sorts the array in place using a comparator and algorithm defined via template parameter.
	
	@param C - the ZComparator type to use
	@param A - the ZArraySortAlgorithm type to use
	@return (ZArray<T, N>&) - this array
	*/
	template <typename C, typename A>
	ZArray<T, N>& Sort();

	/*
	public ZArray<T, N>::Swap

	Swaps the array contents and allocator with another array with any
	local storage size.

	@param _other - the array to swap contents and allocator with
	@return (ZArray<T, N>&) - this array
	*/
	template <size_t M>
	ZArray<T, N>& Swap(ZArray<T, M>& _other);

	/*
	public ZArray<T, N>::SwapElements

	Swaps the values in two indices to this array.

	@param _i - the first value
	@param _j - the second value
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& SwapElements(int _i, int _j);

	/*
	public ZArray<T, N>::Trim

	Trim function.  Cuts the capacity down to size.

	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Trim();

	/*
	public ZArray<T, N>::Unshift

	Removes and returns the first element in the array.  Requires moving the remaining
	elements in the array, so use with care.

	@return (T) - first element in the array
	*/
	T Unshift();
	
	/*
	public ZArray<T, N>::Write

	Write the specified data into this array at the given index.  Overwrites the existing data.

	@param _data - the data to write into this array
	@param _index - the index to start writing at
	@param _count - the number of values from _data to write
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Write(const T* _data, int _index, size_t _count);

	/*
	public ZArray<T, N>::Write

	Writes the specified value into the array starting at the given index.

	@param _value - the value to write into this array
	@param _index - the index to start writing at
	@param _count - the number of times to write the value
	@return (ZArray<T, N>&) - this array
	*/
	ZArray<T, N>& Write(const T _value, int _index, size_t _count); 
};

template <typename T, size_t N>
ZArray<T, N>::ZArray() 
: ArrayCapacity(ZARRAY_DEFAULT_CAPACITY), ArraySize(0), Array(NULL), AllocatorInstance(NULL)
{
	AllocateCheckLocal(ArrayCapacity);

	CheckIntegrity();
}

template <typename T, size_t N>
ZArray<T, N>::ZArray(size_t _capacity) 
: ArrayCapacity(_capacity), ArraySize(0), Array(NULL), AllocatorInstance(NULL)
{
	AllocateCheckLocal(ArrayCapacity);

	CheckIntegrity();
}

template <typename T, size_t N>
ZArray<T, N>::ZArray(size_t _capacity, ZArrayAllocator<T> *_allocator) 
: ArrayCapacity(_capacity), ArraySize(0), Array(NULL), AllocatorInstance(_allocator)
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(AllocatorInstance != NULL, "ZArray constructed NULL allocator!");
	#endif

	AllocateCheckLocal(ArrayCapacity);

	CheckIntegrity();
}

template <typename T, size_t N>
ZArray<T, N>::ZArray(const ZArray<T, N>& _other) 
: ArrayCapacity(_other.Capacity()), ArraySize(_other.Size()), 
  Array(NULL), AllocatorInstance(_other.AllocatorInstance == NULL ? NULL : _other.AllocatorInstance->Clone())
{
	T* data;
	size_t i;

	AllocateCheckLocal(ArrayCapacity);

	CheckIntegrity();

	for (i = 0, data = (T*)_other; i < ArraySize; i++)
		Array[i] = data[i];
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>::ZArray(const ZArray<T, M>& _other) 
: ArrayCapacity(_other.Capacity()), ArraySize(_other.Size()), 
  Array(NULL), AllocatorInstance(_other.AllocatorInstance == NULL ? NULL : _other.AllocatorInstance->Clone())
{
	T* data;
	size_t i;

	AllocateCheckLocal(ArrayCapacity);
	
	CheckIntegrity();

	for (i = 0, data = (T*)_other; i < ArraySize; i++)
		Array[i] = data[i];
}

template <typename T, size_t N>
ZArray<T, N>::~ZArray()
{
	CheckIntegrity();

	DeallocateCheckLocal(Array, ArrayCapacity);

	if (AllocatorInstance != NULL)
		AllocatorInstance->Destroy();
}

template <typename T, size_t N>
T& ZArray<T, N>::operator [] (const int _index) const
{
	int index = AbsoluteIndex(_index, ArraySize);

	return Array[index];
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N> ZArray<T, N>::operator + (const ZArray<T, M>& _other) const
{ 
	//Make an array with enough capacity
	ZArray<T, N> ret(ArraySize + _other.Size());

	//Resize to be at full capacity
	ret.Resize(ArraySize + _other.Size());

	//Write data from both arrays into the return array
	ret.Write(Array, 0, ArraySize);
	ret.Write((T*)_other, ArraySize, _other.Size());

	return ret;
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>& ZArray<T, N>::operator += (const ZArray<T, M>& _other) 
{ 
	int initialSize = ArraySize;

	//Resize this array to have enough capacity
	Resize(ArraySize + _other.Size());

	//Write the other array data into this array (this calls CheckIntegrity)
	Write((T*)_other, initialSize, _other.Size());

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::operator = (const ZArray<T, N>& _other)
{ 
	//duh
	if (this == &_other)
		return *this;

	//Make sure we have room
	Resize(_other.Size());

	//Write the other array's data into this array (calls CheckIntegrity)
	Write((T*)_other, 0, _other.Size());

	return *this;
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>& ZArray<T, N>::operator = (const ZArray<T, M>& _other)
{ 
	//duh
	if (this == &_other)
		return *this;

	//Make sure we have room
	Resize(_other.Size());

	//Write the other array's data into this array (calls CheckIntegrity)
	Write((T*)_other, 0, _other.Size());

	return *this;
}

template <typename T, size_t N> template <size_t M>
bool ZArray<T, N>::operator == (const ZArray<T, M>& _other) const
{ 
	size_t i;

	//First see if we have the same size
	if (ArraySize == _other.Size())
	{
		//Element wise comparison
		for (i = 0; i < ArraySize; i++)
		{
			if (Array[i] != _other.Array[i])
				return false;
		}
	}		
	else
	{
		//Nope, so false
		return false;
	}

	return true;
}

template <typename T, size_t N> template <size_t M>
bool ZArray<T, N>::operator != (const ZArray<T, M>& _other) const
{
	//Make != depend on ==
	return !(*this == _other);
}

template <typename T, size_t N>
ZArray<T, N>::operator T*() const
{
	//Return the raw array that backs the ZArray
	return Array;
}

template <typename T, size_t N>
size_t ZArray<T, N>::AbsoluteIndex(int _index) const
{
	//Check the given index against the array size
	return AbsoluteIndex(_index, ArraySize);
}

template <typename T, size_t N>
ZArrayAllocator<T>& ZArray<T, N>::Allocator() const
{
	//Give back the allocator
	return *AllocatorInstance;
}

template <typename T, size_t N>
void ZArray<T, N>::Allocator(ZArrayAllocator<T> *_allocator)
{
	T* temp = NULL;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(_allocator != NULL, "ZArray assigned NULL allocator!");
	#endif

	//If we are not using local storage
	if (Array != &Local)
	{
		//It is now the temporary array
		temp = Array;

		//Allocate a new array using
		Array = AllocatorInstance->Allocate(ArrayCapacity);

		//Write the data from local
		Write(temp, 0, ArraySize);
	}

	//If temp is not null and not the local array
	if (temp != NULL && temp != &Local)
	{
		//Deallocate it and destroy the old allocator
		if (AllocatorInstance != NULL)
		{
			AllocatorInstance->Deallocate(temp, ArrayCapacity);
			AllocatorInstance->Destroy();
		}
		else
		{
			ZSTL_DEL_ARRAY temp;
		}
	}

	//Set our new allocator instance
	AllocatorInstance = _allocator;
}

template <typename T, size_t N>
size_t ZArray<T, N>::Capacity() const
{ 
	//Give back our current capacity
	return ArrayCapacity;	
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Clear()
{
	//Reset the array size
	ArraySize = 0;

	//Non-const method, so check integrity
	CheckIntegrity();	

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::ClearAndFree(size_t _newCapacity)
{ 
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(_newCapacity > 0, "ZArray: Cannot set capacity to zero!");
	#endif

	//Drop our size to zero
	ArraySize = 0;

	//Make sure this isn't a pointless operation
	if (Array != Local && _newCapacity != ArrayCapacity)
	{
		//Deallocate, set new capacity, and reallocate
		DeallocateCheckLocal(Array, ArrayCapacity);

		ArrayCapacity = _newCapacity;

		AllocateCheckLocal(ArrayCapacity);

		//Check integrity because this is non-const
		CheckIntegrity();
	}

	return *this;
}

template <typename T, size_t N>
bool ZArray<T, N>::Contains(const T& _value, int _index) const
{
	//See if we find the value
	return Find(_value, _index) == ZArray<T, N>::InvalidPos ? false : true;
}

template <typename T, size_t N>
size_t ZArray<T, N>::Count(const T& _value, int _index) const
{ 
	size_t i;

	int index;
	int count;

	//Make sure the rest of this isn't pointless
	if (ArraySize == 0)
		return 0;

	//Grab our index and do bounds checking
	index = AbsoluteIndex(_index, ArraySize);
	count = 0;
	
	//Count the instances
	for (i = index; i < ArraySize; i++)
	{
		if (Array[i] == _value)
			count++;
	}

	return count;
}

template <typename T, size_t N>
bool ZArray<T, N>::Empty() const
{
	//Make sure we aren't empty
	return ArraySize == 0;
}

template <typename T, size_t N>
T ZArray<T, N>::Erase(int _index)
{ 
	size_t i;

	//Grab our index and do bounds checking
	int index = AbsoluteIndex(_index, ArraySize);
	T element = Array[index];

	//Shift the entire array down
	for (i = index; i + 1 < ArraySize; i++)
		Array[i] = Array[i + 1];

	//Reduce size by one
	ArraySize--;

	//Non-const, so check integrity
	CheckIntegrity();

	return element;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Erase(int _i, int _j)
{
	size_t idx;

	//Grab our indices and do bounds checking (_j can go past the end of the array by one)
	int i = AbsoluteIndex(_i, ArraySize);
	int j = AbsoluteIndex(_j, ArraySize + 1);

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(i <= j, "ZArray cannot Erase with j < i!");
	#endif

	//Copy the elements down
	for (idx = i; idx + (j - i) < ArraySize; idx++)
		Array[idx] = Array[idx + (j - i)];

	//Compute our new size
	ArraySize = ArraySize - (j - i);
	
	//We are non-const, so check
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Erase(Iterator& _start, Iterator& _end)
{ 
	//Make this depend on the existing implementation
	Erase((int)_start, (int)_end);

	//Set the end iterator equal to the start
	_end = _start;

	return *this;
}

template <typename T, size_t N>
int ZArray<T, N>::Find(const T& _value, int _index) const
{
	size_t i;

	//Grab our index, check bounds, and count up, looking for elements
	for (i = AbsoluteIndex(_index, ArraySize); i < ArraySize; i++)
	{
		if (Array[i] == _value)
			return i;
	}

	return ZArray<T, N>::InvalidPos;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Insert(int _index, const T& _value, size_t _count)
{ 
	size_t i;

	//Get our insertion index (we allow past end to indicate insert at 'end')
	int index = AbsoluteIndex(_index, ArraySize + 1);

	//Reserve enough space for the new elements
	if (ArraySize + _count > ArrayCapacity)
		Reserve( ArraySize + _count );

	/*
		Shift elements up, but do so in reverse so that we don't just copy Array[i] to all of the higher addresses.
		If you do:
			Array[i+1] = Array[i];
		...in a loop, then the first iteration overwrites the second value with the first, but it doesn't preserve
		the second so it can be copied into the third. If you run this code on [1, 2, 3, 0], you get [1, 1, 3, 0] after the first
		iteration, and then [1, 1, 1, 0] after the second iteration, and so on.

		The solution is to copy backwards. Consider an array of 100 elements:

		[1, 2, 3, ..., 100]

		...that is to by shifted by 3 elements, starting at index 5. Then, we know that
		the final array should look like:

		[1, 2, 3, 4, X, X, X, ..., 5, 6, 7, ... 100].

		where X is undefined (in this function, it is "_value"). The number of elements that
		will be shifted is 100 - 5 = 95 elements. Further more, each element will be shifted 3
		units. Let S be the size of the array (100), I be the index to start at (5), and N
		be the number to shift by (3). Then,

		Since we are copying backwards, consider the final element: Array[S-1]. This element
		is to be moved N units, so:

			Array[S-1 + N] = Array[S-1];
		
		Add in a counter, K, that decreases the index value each loop iteration, and loop S-I times.

			Array[S-1 + N - K] = ArraySize[S-1 - K]

	*/

	//Number of elements to shift.
	const size_t nrToShift = ArraySize - _index;

	for(i = 0; i < nrToShift; i++)
	{
		Array[ArraySize - 1 + _count - i] = Array[ArraySize - 1 - i];
	}

	//Insert new values
	for (i = 0; i < _count; i++)
		Array[index + i] = _value;

	//Increase our array size
	ArraySize += _count;

	//We are non const, so check
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Insert(Iterator& _itr, const T& _value, size_t _count)
{
	//Make this depend on the indexed version
	Insert( (int)_itr, _value, _count );

	//Increase the position of the given iterator
	_itr += _count;

	return *this;
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>& ZArray<T, N>::Insert(int _index, const ZArray<T, M>& _other)
{
	size_t i;

	//Get our index (once again, allowed to go past end to indicate 'append')
	size_t index = AbsoluteIndex(_index, ArraySize + 1);
	size_t newSize = ArraySize + _other.Size();

	//Make sure we have enough space
	if (newSize > ArrayCapacity)
		Reserve( newSize );

	//Shift elements up
	for (i = ArraySize - 1; i > index; i--)
		Array[i + _other.Size()] = Array[i];
	
	//Make sure we get the first element
	Array[i + _other.Size()] = Array[i];

	//Copy in the data from the other
	for (i = 0; i < _other.Size(); i++)
		Array[i + index] = ((T*)_other)[i];

	//Set our new size
	ArraySize = newSize;

	//We are non-const, so check
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>& ZArray<T, N>::Insert(Iterator& _itr, const ZArray<T, M>& _other)
{
	//Make this version depend upon the indexed version
	Insert( (int)_itr, _other );

	//Change our iterator location
	_itr += _other.Size();

	return *this;
}

template <typename T, size_t N>
T& ZArray<T, N>::Peek()
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(ArraySize > 0, "Cannot peek at array with no elements!");
	#endif

	//Return the last element (reference)
	return Array[ArraySize - 1];
}

template <typename T, size_t N>
T ZArray<T, N>::Pop()
{ 
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(ArraySize > 0, "Cannot pop from array with no elements!");
	#endif

	//Grab the last element in the array and decrease our array size
	return Array[--(ArraySize)];
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Push(const T& _element)
{ 
	//See if we need more space
	if (ArraySize >= ArrayCapacity)
		Reserve( ( int)((float)ArrayCapacity * ZARRAY_CAPACITY_RESIZE_FACTOR) + 1 ); //Calls CheckIntegrity

	//Add in the element
	Array[(ArraySize)++] = _element;

	return *this;
}

template <typename T, size_t N>
int ZArray<T, N>::Remove(const T& _element)
{
	//Make this depend upon the indexed version
	return Remove(_element, 0);
}

template <typename T, size_t N>
int ZArray<T, N>::Remove(const T& _element, int _index)
{
	//Get our index and do our bounds checking
	size_t i = AbsoluteIndex(_index, ArraySize);

	//Look for the first occurrence and call Erase at that index
	for (; i < ArraySize; i++)
	{
		if (Array[i] == _element)
		{
			Erase(i); //calls check integrity
			return i;
		}
	}

	return ZArray<T, N>::InvalidPos;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::RemoveAll(const T& _element, int _index)
{ 
	//Get our index and do our bounds checking
	size_t i = AbsoluteIndex(_index, ArraySize);

	//Iterate, calling remove until we are out of elements to remove
	while (i < ArraySize && i != (size_t)ZArray<T, N>::InvalidPos)
		i = Remove(_element, i); //Calls Check Integrity

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Reserve(size_t _capacity)
{ 
	size_t i;
	T* temp;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(_capacity >= ArraySize, "ZArray cannot reserve capacity less than array size!");
	#endif

	//Set our temp array
	temp = Array;

	//Allocate an array
	AllocateCheckLocal(_capacity);

	//Make sure we aren't doing something pointless here
	if (temp == Array)
		return *this;

	//Copy the data
	for (i = 0; i < ArraySize; i++)
		Array[i] = temp[i];

	//Deallocate the temporary array
	DeallocateCheckLocal(temp, ArrayCapacity);

	//Set our new capacity
	ArrayCapacity = _capacity;

	//non-const, so check integrity
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Resize(size_t _size)
{ 
	//Check to see if we need more space
	if (_size > ArrayCapacity)
		Reserve(_size);

	//Change our size
	ArraySize = _size;

	//Non-const, so check integrity
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Resize(size_t _size, const T& _value)
{
	int i;

	//See if we need more space
	if (_size > ArrayCapacity)
		Reserve(_size * ZARRAY_CAPACITY_RESIZE_FACTOR);

	//Copy in the new value
	for (i = ArraySize; i < _size; i++)
		Array[i] = _value;

	//Change our size
	ArraySize = _size;

	//Non-const, so check integrity
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Shift(const T& _element)
{
	//Makes this depend upon insert
	return Insert(0, _element, 1);
}

template <typename T, size_t N>
size_t ZArray<T, N>::Size() const
{
	//Returns the current size
	return ArraySize;
}

template <typename T, size_t N>
ZArray<T, N> ZArray<T, N>::Slice(int _i, int _j) const
{
	//Make sure our indices are correct
	int i = AbsoluteIndex(_i, ArraySize);
	int j = AbsoluteIndex(_j, ArraySize + 1);

	//Make sure the rest of this isn't pointless
	if (j <= i)
		return ZArray<T, N>();

	//Get the size of our slice
	int sliceSize = _j - _i;

	//Make a new array to return with the right capacity
	ZArray<T, N> ret(sliceSize);

	//Get at the proper size and write data into the array
	ret.Resize(sliceSize);
	ret.Write(&Array[i], 0, sliceSize);

	return ret;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Sort(const ZComparator<T>& _comparator, const ZArraySortAlgorithm<T>& _algorithm)
{ 
	//Sort using the provided algo
	_algorithm.Sort(_comparator, Array, ArraySize);

	//Make sure sorting didn't destroy everything
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N> template <typename A>
ZArray<T, N>& ZArray<T, N>::Sort( const ZComparator<T>& _comparator )
{
	A algo;

	return Sort(_comparator, algo);
}

template <typename T, size_t N> template <typename C>
ZArray<T, N>& ZArray<T, N>::Sort( const ZArraySortAlgorithm<T>& _algorithm )
{
	C comparator;

	return Sort(comparator, _algorithm);
}

template <typename T, size_t N> template <typename C, typename A>
ZArray<T, N>& ZArray<T, N>::Sort()
{
	C comparator;
	A algo;

	return Sort(comparator, algo);
}

template <typename T, size_t N> template <size_t M>
ZArray<T, N>& ZArray<T, N>::Swap(ZArray<T, M>& _other)
{	
	//First we see if either of us are using local storage
	if (Array == Local || _other.Array == _other.Local)
	{
		size_t i;

		//Create a temporary array
		T* tempArray = ZSTL_NEW_ARRAY(T, ArrayCapacity > _other.ArrayCapacity ? ArrayCapacity : _other.ArrayCapacity);
		int tempCapacity = _other.ArrayCapacity;
		int tempSize = _other.ArraySize;
		ZArrayAllocator<T>* tempAllocator = _other.AllocatorInstance;

		//Copy other into the temp array
		for (i = 0; i < _other.Size(); i++)
			tempArray[i] = _other[i];

		//Resize the other array to the required size
		_other.Resize(ArraySize);
		_other.Reserve(ArrayCapacity);
		_other.AllocatorInstance = AllocatorInstance;

		//Copy data from this array into other
		for (i = 0; i < ArraySize; i++)
			_other[i] = Array[i];

		//Resize this array to the required size
		Resize(tempSize);
		Reserve(tempCapacity);
		AllocatorInstance = tempAllocator;

		//Copy data from temp array into this array
		for (i = 0; i < ArraySize; i++)
			Array[i] = tempArray[i];

		//Free the temporary array
		ZSTL_DEL_ARRAY(tempArray);
	}
	else
	{
		//Just swap array pointers and state data
		T* tempArray = Array;
		int tempCapacity = ArrayCapacity;
		int tempSize = ArraySize;
		ZArrayAllocator<T>* tempAllocator = AllocatorInstance;

		Array = _other.Array;
		_other.Array = tempArray;

		ArrayCapacity = _other.ArrayCapacity;
		_other.ArrayCapacity = tempCapacity;

		ArraySize = _other.ArraySize;
		_other.ArraySize = tempSize;

		AllocatorInstance = _other.AllocatorInstance;
		_other.AllocatorInstance = tempAllocator;
	}

	//Check integrity on both arrays
	CheckIntegrity();
	_other.CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::SwapElements(int _i, int _j)
{
	//Get our indices to swap
	int i = AbsoluteIndex(_i, ArraySize);
	int j = AbsoluteIndex(_j, ArraySize);
	
	//Doo eet
	T temp = Array[i];
	Array[i] = Array[j];
	Array[j] = temp;

	//Make sure we didn't screw everyhing up 
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Trim()
{
	//Reserves exactly the amount of our array size
	Reserve(ArraySize);

	return *this;
}

template <typename T, size_t N>
T ZArray<T, N>::Unshift()
{
	//Makes this depend upon Erase
	return Erase(0);
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Write(const T* _data, int _index, size_t _count)
{
	size_t i;
	int index;
	
	//Make sure the rest of this isn't pointless
	if (_count == 0)
		return *this;
	
	//Get our current index
	index = AbsoluteIndex(_index, ArraySize);

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(index + _count <= ArraySize, "Write operation overruns ZArray size!");
	#endif

	//Copy the data
	for (i = 0; i < _count; i++)
		Array[index++] = _data[i];

	//non-const, so check
	CheckIntegrity();

	return *this;
}

template <typename T, size_t N>
ZArray<T, N>& ZArray<T, N>::Write(const T _value, int _index, size_t _count)
{
	int i;
	int index;

	//Make sure the rest of this isn't pointless
	if (_count == 0)
		return *this;

	//Get our current index
	index = AbsoluteIndex(_index, ArraySize);

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(index + _count <= ArraySize, "Write operation overruns ZArray size!");
	#endif

	//Copy the value into the array
	for (i = 0; i < _count; i++)
		Array[index + i] = _value;
	
	//Non-const, so check
	CheckIntegrity();

	return *this;
}

#endif
