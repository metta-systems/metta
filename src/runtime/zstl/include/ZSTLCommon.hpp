/*
	ZSTLCommon.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 9/13/2011

	Purpose: 

	Common include file used by all ZSTL components.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZSTLCOMMON_HPP
#define _ZSTLCOMMON_HPP

#include <stdint.h>			//Need this for our int types
#include <new>					//This gives us (std::nothrow)
#include <assert.h>				//This gives us assert

//Hash Value Types
typedef uint32_t ZHashValue;
typedef uint32_t ZHashValue32;
typedef uint64_t ZHashValue64;

//Default allocation macro used by ZSTL default allocators
#ifndef ZSTL_NEW
#define ZSTL_NEW new
// #define ZSTL_NEW new (std::nothrow)
#endif

//Default delete macro used by ZSTL default allocators
#ifndef ZSTL_DEL
#define ZSTL_DEL delete
#endif

//Default array allocation macro used by ZSTL default allocators
#ifndef ZSTL_NEW_ARRAY
#define ZSTL_NEW_ARRAY(_type, _size) new _type[_size]
// #define ZSTL_NEW_ARRAY(_type, _size) new (std::nothrow) _type[_size]
#endif

//Default delete macro used by ZSTL default allocators
#ifndef ZSTL_DEL_ARRAY
#define ZSTL_DEL_ARRAY delete[]
#endif

//Default assert macro used by ZSTL
#ifndef ZSTL_ASSERT
#define ZSTL_ASSERT(condition, message) (assert(condition))
#endif

//Error condition macro used by ZSTL
#ifndef ZSTL_ERROR
#define ZSTL_ERROR(message) (assert(0));
#endif

//Swap used in quicksort (takes array, first index, second index)
#define ZSTL_ARRAY_ELEMENT_SWAP(_a, _i, _j) temp = _a[_j]; _a[_j] = _a[_i]; _a[_i] = temp;

//Push used in mergesort (takes list node and ZNew node)
#define ZSTL_LIST_ELEMENT_PUSH(_l, _n) if (_l == NULL) { _l = _n; } else { _l->Next = _n; _n->Previous = _l; _l = _n; }

#include "ZAllocator.hpp"

/*
Iterator interface for ZSTL.  Many methods do not care what the underlying container 
is, and in such cases can use a ZIterator instead of the more specialized iterator 
types.

The template parameter T is the type containd in the container we will be iterating over.
*/
template <typename T>
class ZIterator
{
public:
	//Virtual Destructor
	virtual ~ZIterator() { }

	/*
	virtual public ZIterator<T>::Get
	
	Gets the value at the current iterator position.
	
	@return (T&) - the element at the current iterator location
	*/
	virtual T& Get() const = 0;

	/*
	virtual public ZIterator<T>::HasCurrent
	
	Used to indicate that this iterator has an element at the current location and
	a call to Get is valid.
	
	@return (bool) - true if the current location is iterator valid
	*/
	virtual bool HasCurrent() const = 0;

	/*
	virtual public ZIterator<T>::HasNext
	
	Used to indicate that this iterator has one or more elements following the current location
	and a call to Next is valid.
	
	@return (bool) - true if elements exist after the current iterator position, false otherwise
	*/
	virtual bool HasNext() const = 0;

	/*
	virtual public ZIterator<T>::HasPrev
	
	Used to indicate that this iterator has one or more elements before the current location and 
	a call to Prev is valid.
	
	@return (bool) - true if elements exist before the current iterator position, false otherwise
	*/
	virtual bool HasPrev() const = 0;

	/*
	virtual public ZIterator<T>::Next
	
	Used to advance the iterator to the next position.  Should raise a runtime error 
	if no element exists after this.
	
	@return (void)
	*/
	virtual void Next() = 0;

	/*
	virtual public ZIterator<T>::Prev
	
	Used to return the iterator to the previous position.  Should raise a runtime error
	if no element exists prior to this.
	
	@return (void)
	*/
	virtual void Prev() = 0;
};

/*
Reference counting struct.  Used to count references to something that is
not shared across threads, or where the concurrency control is external.

The template parameter T is the type we are pointing to.
*/
template <typename T>
class ZRefCounter
{
public:
	//The Reference Counted Object
	T* Ptr;

	//The Reference Count
	int RefCount;

	//Default Constructor
	ZRefCounter() : Ptr(NULL), RefCount(0) { }

	//Increment the counter
	void Increment() { ++RefCount; }

	//Decrement the counter
	bool Decrement() { return (--RefCount == 0); }
};

/*
Node class, which is used by ZList to contain list data and previous / next pointers.  Could
theoretically be used by other classes that also need basic list-like functionality.

The template parameter T is the type contained by the list node.
*/
template <typename T>
struct ZListNode
{
	//Default Constructor
	ZListNode() { Next = NULL; Previous = NULL; }

	//The contained element
	T Element;

	//The next node
	ZListNode* Next;

	//The previous node
	ZListNode* Previous;
};

/*
Comparator class, used for sorting operations on containers that can be sorted.  In 
order to sort containers with specialized elements using a unique set of rules, a 
specialized comparator can be created.

Alternatively, overriding the < operator and == operator will allow the object to be compared using the
default comparator.

The template parameter T is the type we will be comparing to each other.
*/
template <typename T>
class ZComparator
{
public:
	//Virtual Destructor
	virtual ~ZComparator() { }

	/*
	virtual public ZComparator::Equals

	Returns true if the first argument is equal to the second.  Defaults to using the 
	equals operator to compare the two.

	@param _a - first of type T
	@param _b - second of type T
	@return (bool) - true if _a is equal to _b
	*/
	virtual bool Equals(const T& _a, const T& _b) const { return _a == _b; }

	/*
	virtual public ZComparator::LessThan

	Returns true if the first argument is less than the second.  Defaults to using the 
	less than operator to compare the two.

	@param _a - first of type T
	@param _b - second of type T
	@return (bool) - true if _a is less than _b
	*/
	virtual bool LessThan(const T& _a, const T& _b) const { return _a < _b; }
};

/*
Array sorter class.  Used to define the algorithm used to sort the array.  Should be extended, though
ZArray::Sort has a templated compile-time polymorphism based sort as well.

The template parameter T is the type contained by the array we will be sorting.
*/
template <typename T>
class ZArraySortAlgorithm
{
public:
	//Virtual Destructor
	virtual ~ZArraySortAlgorithm() { }

	/*
	virtual public ZArraySortAlgorithm::Sort

	Sort method.  Implements the actual sorting functionality.

	@param _comparator - the comparator to use to sort the values
	@param _array - the array to sort as a raw pointer
	@param _size - the size of the array
	@return (void)
	*/
	virtual void Sort(const ZComparator<T>& _comparator, T* _array, int _size) const = 0;
};

/*
Array quicksort class.  Uses a recursive in-place sort (not a 'stable' sort).

The template parameter T is the type contained by the array we will be sorting.
*/
template <typename T>
class ZArrayQuickSort : public ZArraySortAlgorithm<T>
{
private:
	//Helper function which partitions the array
	inline int Partition(const ZComparator<T>& _comp, T* _array, int _left, int _right, int _pivot) const
	{
		int i, j;
		T temp;

		//Get the value at the pivot point
		T pivotValue = _array[_pivot];

		//Move pivot to end
		ZSTL_ARRAY_ELEMENT_SWAP(_array, _pivot, _right);
		
		//Check values from left up to pivot
		for (i = _left, j = _left; i < _right; i++) 
		{
			//If less than or equal to the pivot value
			if (_comp.LessThan(_array[i], pivotValue) || _comp.Equals(_array[i], pivotValue))
			{
				//Swap back and increment our 'target' index j
				ZSTL_ARRAY_ELEMENT_SWAP(_array, i, j);
				j++;
			}
		}

		//Move pivot to final location (all values with index < j are < pivotValue)
		ZSTL_ARRAY_ELEMENT_SWAP(_array, j, _right); 

		return j;
	}

	//Helper function which performs the sorting
	void QuickSort(const ZComparator<T>& _comp, T* _array, int _left, int _right) const
	{
		int pivot;

		if (_right > _left)
		{
			//Center pivot point (guarded against overflow)
			pivot = _left + (_right - _left) / 2;

			//Get our next pivot after partitioning around the current
			pivot = Partition(_comp, _array, _left, _right, pivot);

			//Sort the left partition
			QuickSort(_comp, _array, _left, pivot - 1);

			//Sort the right partition
			QuickSort(_comp, _array, pivot + 1, _right);
		}
	}

public:
	//Subclass Override
	void Sort(const ZComparator<T>& _comparator, T* _array,  int _size) const
	{
		QuickSort(_comparator, _array, 0, _size - 1);
	}
};

/*
List sorter class.  Used to sort a list in place.  Should be extended, though
ZList::Sort also has a templated compile-time polymorphism based sort as well.

The template parameter T is the type contained by the underlying list we will be sorting.
*/
template <typename T>
class ZListSortAlgorithm
{
public:
	//Virtual Destructor
	virtual ~ZListSortAlgorithm() { }

	/*
	virtual public ZListSortAlgorithm::Sort

	Sort method.  Implements the actual sorting functionality.

	@param _comparator - the comparator to use to sort the values
	@param _list - pointer to the beginning node of the list to sort
	@param _end - pointer to the end node of the list to sort
	@return (void)
	*/
	virtual void Sort(const ZComparator<T>& _comparator, ZListNode<T>* _list, ZListNode<T>* _end) const = 0;
};

/*
Merge sort implementation of the ListSortAlgorithm.  This implementation 
is a 'stable' sort.

The template parameter T is the type contained by the underlying list we will be sorting.
*/
template <typename T>
class ZListMergeSort : public ZListSortAlgorithm<T>
{
private:
	//Special 'Len' function which uses no end node
	inline int Length(ZListNode<T>* _list) const
	{
		int i;
		ZListNode<T>* node;

		for (i = 0, node = _list; node != NULL; i++, node = node->Next);

		return i;
	}

	//Helper function which merges two lists, returns the last node
	inline ZListNode<T>* Merge(const ZComparator<T>& _comparator, ZListNode<T>* _left, ZListNode<T>* _right) const
	{
		ZListNode<T>* merged = NULL;

		//While left and right still have elements
		while (_left != NULL && _right != NULL)
		{
			//Compare first elements
			if (_comparator.LessThan(_left->Element, _right->Element))
			{
				//Add the left element
				ZSTL_LIST_ELEMENT_PUSH(merged, _left);
				_left = _left->Next;
			}
			else
			{
				//Add the right element
				ZSTL_LIST_ELEMENT_PUSH(merged, _right);
				_right = _right->Next;
			}
		}

		//While the left still has elements
		while (_left != NULL)
		{
			//Add them
			ZSTL_LIST_ELEMENT_PUSH(merged, _left);
			_left = _left->Next;
		}

		//While the right still has elements
		while (_right != NULL)
		{
			//Add them
			ZSTL_LIST_ELEMENT_PUSH(merged, _right);
			_right = _right->Next;
		}

		//Return the end node
		return merged;
	}

	//Helper function which merge sorts a list, returns the last node
	inline ZListNode<T>* MergeSort(const ZComparator<T>& _comparator, ZListNode<T>* _list) const
	{
		int i;
		int middle;

		ZListNode<T>* left;
		ZListNode<T>* right;

		//If none or one element
		if (_list == NULL || _list->Next == NULL)
			return _list;

		//Determine midpoint
		middle = Length(_list) / 2;

		//Set our left and right
		left = right = _list;

		//Set right to midpoint
		for (i = 0; i < middle; i++)
			right = right->Next;

		//Seperate the list
		right->Previous->Next = NULL;
		right->Previous = NULL;

		//Sort left and right recursively
		MergeSort(_comparator, left);
		MergeSort(_comparator, right);

		//Set back our left list pointer
		while (left != NULL && left->Previous != NULL)
			left = left->Previous;

		//Set back our right list pointer
		while (right != NULL && right->Previous != NULL)
			right = right->Previous;

		//Return the last node from the merged lists
		return Merge(_comparator, left, right);
	}

public:
	//Subclass implementation
	void Sort(const ZComparator<T>& _comparator, ZListNode<T>* _list, ZListNode<T>* _end) const 
	{
		//Split the end node off
		_end->Previous->Next = NULL;

		//Reset the end node back
		_end->Previous = MergeSort(_comparator, _list);

		//Attach
		_end->Previous->Next = _end;
	}
};

#endif
