/*
	ZList.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 9/12/2011

	Purpose: 

	Templated doubly-linked list implementation.

	Defining the following features to 1 enables the feature.  Defining to 0 disables the
	feature (default behavior if undefined).

	ZSTL_CHECK_INTEGRITY
	Checks integrity of the ZSTL containers after allocations or if non-const functions are called.  
	Used for debugging ZSTL or if new methods are added.

	ZSTL_DISABLE_RUNTIME_CHECKS
	Disables runtime bounds and error checking on ZSTL containers.

	ZSTL_DISABLE_NEGATIVE_INDEXING
	Disables negative indexing on ZSTL containers.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZLIST_HPP
#define _ZLIST_HPP

#include <ZSTL/ZSTLCommon.hpp>

//////////////////////
/* ZList Allocators */
//////////////////////

/*
Allocator for ZList.  Handles allocation of nodes of the templated type.

The template parameter T is the type contained in the list this allocator is for.
*/
template <typename T>
class ZListAllocator
{
public:
	//Virtual Destructor
	virtual ~ZListAllocator() { }

	/*
	virtual public ZListAllocator<T>::Allocate

	Allocator function which allocates a ZListNode<T>.

	@return - an allocated ZListNode<T>
	*/
	virtual ZListNode<T>* Allocate() = 0;

	/*
	virtual public ZListAllocator<T>::Clone

	Clone function, which is required to allocate and return a ZNew instance of this
	type of allocator.

	@return - allocated instance of this type of allocator
	*/
	virtual ZListAllocator<T>* Clone() = 0;

	/*
	virtual public ZListAllocator<T>::Deallocate

	Deallocation function which deallocates a previously allocated ZListNode<T>.

	@param _node - node to deallocate
	*/
	virtual void Deallocate(ZListNode<T>* _node) = 0;

	/*
	virtual public ZListAllocator<T>::Destroy

	Destroy method.  Called when the allocator is no longer needed by the ZList.
	Heap allocated allocators should delete themselves (suicide).

	@return (void)
	*/
	virtual void Destroy() = 0;
};

////////////////////
/* ZList Iterator */
////////////////////

/*
Iterator class for ZList.

The template parameter T is the type contained in the list this iterator is for.
*/
template <typename T>
class ZListIterator : public ZIterator<T>
{
private:
	//The current node we are pointing to
	ZListNode<T> *Node;

public:
	/*
	Default Constructor.
	*/
	ZListIterator() : Node(NULL) { }

	/*
	Useful constructor.

	@param _node - the node we are to begin iterating at
	*/
	ZListIterator(ZListNode<T>* _node) : Node(_node) { }

	/*	
	Copy Constructor.

	@param _other - the other iterator
	*/
	ZListIterator(const ZListIterator& _other) : Node(_other.Node) { }

	
	/*
	public ZListIterator<T>::CheckNodeCurrent
	
	Node check function that does not allow the current node to point to NULL
	
	@return (void)
	*/
	void CheckNodeCurrent() const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Node != NULL, "ZList Iterator Next Invalid!");
		#endif
	}

	/*
	public ZListIterator<T>::CheckNodeNext
	
	Node check function that does not allow Node to be 'End'
	
	@return (void)
	*/
	void CheckNodeNext() const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Node != NULL && Node->Next != NULL, "ZList Iterator Next Invalid!");
		#endif
	}

	/*
	public ZListIterator<T>::CheckNodePrevious
	
	Node check function that does not allow Node to be 'Begin'
	
	@return (void)
	*/
	void CheckNodePrevious() const
	{
		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(Node != NULL && Node->Previous != NULL, "ZList Iterator Previous Invalid!");
		#endif
	}

	/*
	public ZListIterator<T>::GetNode
	
	Gets the node that this iterator is currently pointed at.
	
	@return (ZListNode<T>*) - the node we are currently pointed at
	*/
	ZListNode<T>* GetNode()										{ return Node; }

	/*
	public ZListIterator<T>::SetNode
	
	Sets the current node for this iterator.  Useful for when the underlying list changes state.
	
	@param _node - the node to point this iterator at
	@return (void)
	*/
	void SetNode(ZListNode<T>* _node)							{ Node = _node; }

	//Subclass override
	virtual T& Get() const										{ return *(*this); }

	//Subclass override
	virtual bool HasCurrent() const								{ return Node != NULL; }

	//Subclass override
	virtual bool HasNext() const								{ return Node != NULL && Node->Next != NULL; }

	//Subclass override
	virtual bool HasPrev() const								{ return Node != NULL && Node->Previous != NULL; }

	//Subclass override
	virtual void Next()											{ ++(*this); }

	//Subclass override
	virtual void Prev()											{ --(*this); }

	//Iterator Operator Overrides
	ZListIterator& operator ++()								{ CheckNodeNext(); Node = Node->Next; return *this; }
	ZListIterator operator ++(int)								{ ZListNode<T> *node = Node; CheckNodeNext(); Node = Node->Next; return ZListIterator(node); }
	ZListIterator& operator --()								{ CheckNodePrevious(); Node = Node->Previous; return *this; }
	ZListIterator operator --(int)								{ ZListNode<T> *node = Node; CheckNodePrevious(); Node = Node->Previous; return ZListIterator(node); }
	ZListIterator& operator =(const ZListIterator &_other)		{ Node = _other.Node; return *this; }
	bool operator ==(const ZListIterator &_other) const			{ return Node == _other.Node; }
	bool operator !=(const ZListIterator &_other) const			{ return !(Node == _other.Node); }
	T& operator *() const										{ CheckNodeCurrent(); return Node->Element; }
};

//////////////////////////
/* ZList Implementation */
//////////////////////////

/*
List implementation for ZEngine.

ZList takes a single template parameter, which is the contained type.  The allocator is
passed in at construction, which allows the allocator to be decoupled from the list container.

The template parameter T is the type to be contained in the list.
*/
template <typename T>
class ZList
{
protected:
	//Pointer to the first node in this list
	ZListNode<T> *First;

	//Pointer to the last node in the list (always the 'end' node)
	ZListNode<T> *Last;

	//Empty list node
	ZListNode<T> EmptyNode;

	//Allocator for the list
	ZListAllocator<T> *AllocatorInstance;

	//Local allocate function
	inline ZListNode<T>* AllocateNode()
	{
		if (AllocatorInstance != NULL)
		{
			return AllocatorInstance->Allocate();
		}
		else
		{
			return ZSTL_NEW ZListNode<T>();
		}
	}

	//Local destroy function
	inline void DeallocateNode(ZListNode<T> *_node)
	{
		if (AllocatorInstance != NULL)
		{
			AllocatorInstance->Deallocate(_node);
		}
		else
		{
			ZSTL_DEL _node;
		}
	}

	//Integrity Check
	inline void CheckIntegrity() const
	{
		#if ZSTL_CHECK_INTEGRITY

		ZListNode<T>* current;
		ZListNode<T>* previous;

		current = First;

		ZSTL_ASSERT(current != NULL , "ZList Error: Contains invalid linkage pointers!");

		#if defined(_MSC_VER)
		ZSTL_ASSERT(current != (void*)0xfeeefeee && current != (void*)0xcdcdcdcd, "ZList Error: Contains invalid pointers!");
		#endif

		previous = current->Previous;

		ZSTL_ASSERT(previous == NULL, "First linkage invalid!");

		for(;;) //Stupid MSVC warning
		{
			ZSTL_ASSERT(current != NULL, "ZList Error: Contains invalid linkage pointers!");

			#if defined(_MSC_VER)
			ZSTL_ASSERT(current != (void*)0xfeeefeee && current != (void*)0xcdcdcdcd, "ZList Error: Contains invalid linkage pointers!");
			#endif

			previous = current;
			current = current->Next;

			if (current == NULL)
				break;

			ZSTL_ASSERT(current != NULL, "ZList Error: Contains invalid linkage pointers!");

			#if defined(_MSC_VER)
			ZSTL_ASSERT(current != (void*)0xfeeefeee && current != (void*)0xcdcdcdcd, "ZList Error: Contains invalid linkage pointers!");
			#endif

			ZSTL_ASSERT(previous == current->Previous, "ZList Error: Linkage invalid!");
		}

		ZSTL_ASSERT(previous == Last, "ZList Error: End node invalid!");

		#endif //ZSTL_CHECK_INTEGRITY
	}

public:
	//Typedef for ZListIterator
	typedef ZListIterator<T> Iterator;

	/*
	Default Constructor.
	*/
	ZList();

	/*
	Parameterized Constructor.

	@param _allocator - the allocator to use
	*/
	ZList(ZListAllocator<T>* _allocator);

	/*
	Sub-List Constructor.  Constructs a list containing the elements between two given iterators.

	@param _begin - the iterator at which to start the list
	@param _end - the iterator at which to end the list (exclusive)
	*/
	ZList(const Iterator& _begin, const Iterator& _end);

	/*
	Sub-List Constructor.  Constructs a list containing the elements between two given iterators.

	@param _begin - the iterator at which to start the list
	@param _end - the iterator at which to end the list (exclusive)
	@param _allocator - the allocator to use
	*/
	ZList(const Iterator& _begin, const Iterator& _end, ZListAllocator<T>* _allocator);

	/*
	Copy Constructor.  Makes a copy of the list.

	@param _other - the other list to copy.
	*/
	ZList(const ZList<T>& _other);

	/*
	Destructor.
	*/
	~ZList();

	/*
	[] Operator overload.  Gets an iterator to a specific index in the list.
	Not very efficient, as this kind of operation is best suited to ZArray.

	@param _index - index into the list
	@return (ZList<T>::Iterator) - iterator to the given index
	*/
	Iterator operator [] (const int _index) const
	{
		int i;
		Iterator itr;

		if (_index < 0)
		{
			#if ZSTL_DISABLE_NEGATIVE_INDEXING
			ZSTL_ERROR("Negative index used while ZSTL_DISABLE_NEGATIVE_INDEX defined!");
			#endif

			for (i = 0, itr = End(); i > _index; i--)
				itr--;
		}		
		else
		{
			for (i = 0, itr = Begin(); i < _index; i++)
				itr++;
		}

		return itr;
	}

	/*
	= Operator overload.  Sets this list equal to the other by making a copy 
	of all the contained nodes.

	@param _other - the other list to copy.
	@return (ZList<T>&) - this list
	*/
	ZList<T>& operator = (const ZList<T>& _other);

	/*
	== Operator overload.  Performs an element-wise comparison of lists.

	@param _other - the list to compare against
	@return (bool) - true if they are equal, false otherwise
	*/
	bool operator == (const ZList<T>& _other) const;

	/*
	!= Operator overload.  Performs an element-wise comparison of lists.

	@param _other - the list to compare against
	@return (bool) - true if they are not equal, false if they are
	*/
	bool operator != (const ZList<T>& _other) const;

	/*
	public ZList<T>::Back

	Gets a reference to the value at the back of the list.

	@return (T&) - last element in the list
	*/
	T& Back() const;

	/*
	public ZList<T>::Begin

	Gets an iterator to the beginning of the list.

	@return (ZList<T>::Iterator) - iterator pointing to the first list node
	*/
	Iterator Begin() const { return Iterator(First); }

	/*
	public ZList<T>::Clear

	Clears the list.

	@return (ZList<T>&) - this list
	*/
	ZList<T>& Clear();

	/*
	public ZList<T>::Clear

	Clears the list after a specific iterator location (inclusive).

	@return (ZList<T>&) - this list
	*/
	ZList<T>& Clear(Iterator& _itr);

	/*
	public ZList<T>::Count

	Counts the number of occurrences of a value in the list.

	@param _value - the value to look for
	@return (size_t) - the number of occurrences of the value
	*/
	size_t Count(const T& _value) const;

	/*
	public ZList<T>::Empty

	O(1) operation that determines if the list is empty.

	@return (bool) - true if the list has no elements, false otherwise
	*/
	bool Empty() const;

	/*
	public ZList<T>::End

	Gets an iterator to the 'end' node, which is one element past
	the last element in the list.

	@return (ZList<T>::Iterator) - iterator pointing to the 'end' node
	*/
	Iterator End() const { return Iterator(Last); }

	/*
	public ZList<T>::Erase

	Removes a value from the list at the specified location.

	@param _itr - the iterator location to remove the value from
	@return (T) - the removed value
	*/
	T Erase(Iterator& _itr);

	/*
	public ZList<T>::Erase

	Removes a range of values from the list.

	@param _from - the iterator location to start at
	@param _to - the iterator location to end at (exclusive)
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Erase(Iterator& _from, Iterator& _to);

	/*
	public ZList<T>::Find

	Searches for the first occurrence of the specified value in the list.

	@param _value - the value to search for
	@return (ZList<T>::Iterator) - the iterator location of the value
	*/
	Iterator Find(const T& _value) const
	{
		return Find(_value, Begin());
	}

	/*
	public ZList<T>::Find

	Searches for the first occurrence of the specified value in the list,
	starting at the iterator location.

	@param _value - the value to search for
	@param _itr - the iterator location to start searching at
	@return (ZList<T>::Iterator) - the iterator location where the value is found
	*/
	Iterator Find(const T& _value, const Iterator& _itr) const
	{
		Iterator itr;

		for (itr = _itr; itr != End(); itr++)
			if ((*itr) == _value)
				break;

		return itr;
	}

	/*
	public ZList<T>::Front

	Gets a reference to the value at the front of the list.

	@return (T&) - first value in the list
	*/
	T& Front() const;

	/*
	public ZList<T>::Insert

	Inserts a value at the specified location in the list.

	@param _itr - iterator to the location to insert the value
	@param _value - the value to add to the beginning of the list
	@param _resetIterator - if true, resets the iterator to the inserted value position,
		if false, keeps the iterator at the element after the inserted value
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Insert(Iterator& _itr, const T& _value, bool _resetIterator = false);

	/*
	public ZList<T>::Insert

	Inserts another list at the specified location in the list.  Removes
	the elements from the inserted list.

	@param _itr - iterator to the location to insert the list
	@param _other - the other list to insert
	@param _resetIterator - if true, resets the iterator to the beginning of the inserted
		list; if false, keeps the iterator at the element after the inserted value
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Insert(Iterator& _itr, ZList<T>& _other, bool _resetIterator = false);

	/*
	public ZList<T>::PopBack

	Pops a value from the end of the list.

	@return (T) - the value at the back of the list
	*/
	T PopBack();

	/*
	public ZList<T>::PopFront

	Removes and returns the value from the beginning of the list.

	@return (T) - the value at the front of the list
	*/
	T PopFront();

	/*
	public ZList<T>::PushBack

	Pushes a value onto the back of the list.

	@param _value - the value to place in the list
	@return (ZList<T>&) - this list
	*/
	ZList<T>& PushBack(const T& _value);

	/*
	public ZList<T>::PushFront

	Pushes a value onto the front of the list.

	@param _value - the value to place in the list
	@return (ZList<T>&) - this list
	*/
	ZList<T>& PushFront(const T& _value);

	/*
	public ZList<T>::Remove
	
	Removes the first occurence of the specified value from the list.
	
	@param _value - 
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Remove(const T& _value);

	/*
	public ZList<T>::Remove

	Removes the first occurrence of the specified value from the list starting at
	the given iterator location.

	@param _value - the value to remove
	@param _itr - the iterator location to start searching at; will point to the location
				  where the first occurrence was found after calling this function
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Remove(const T& _value, Iterator& _itr);

	/*
	public ZList<T>::RemoveAll

	Removes all occurrences of the specified value from the list.

	@param _value - the value to remove
	@return (ZList<T>&) - this list
	*/
	ZList<T>& RemoveAll(const T& _value);

	/*
	public ZList<T>::RemoveAll

	Removes all occurrences of the specified value from the list, starting 
	at the given iterator location.

	@param _value - the value to remove
	@param _itr - the iterator location to start searching at; will point to the list
		end after calling this function
	@return (ZList<T>&) - this list
	*/
	ZList<T>& RemoveAll(const T& _value, Iterator& _itr);

	/*
	public ZList<T>::Size

	O(n) operation that gives the size of the list.

	@return (size_t) - list length  (number of contained elements)
	*/
	size_t Size() const;

	/*
	public ZList<T>::Sort

	Sorts the list in place.

	@param _comparator - comparator to use to compare values
	@param _algorithm - the sort algorithm to use to sort the list
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Sort(const ZComparator<T>& _comparator = ZComparator<T>(), const ZListSortAlgorithm<T>& _algorithm = ZListMergeSort<T>());

	/*
	public ZList<T>::Sort
	
	Sorts the list in place with template defined comparator.
	
	@param C - ZComparator type to use (default uses standard ZComparator)
	@param _algorithm - algorithm type to use
	@return (ZList<T>&) - this list
	*/
	template <typename C>
	ZList<T>& Sort(const ZListSortAlgorithm<T>& _algorithm);

	/*
	public ZList<T>::Sort
	
	Sorts the list in place with template defined sort algorithm.
	
	@param A - ZListSortAlgorithm type to use
	@param _comparator - comparator instance to use
	@return (ZList<T>&)
	*/
	template <typename A>
	ZList<T>& Sort(const ZComparator<T>& _comparator);

	/*
	public ZList<T>::Sort
	
	Sorts the list in place with template defined comparator and sort algorithm.
	
	@param C - ZComparator type to use
	@param A - ZListSortAlgorithm type to use
	@return (ZList<T>&) - this list
	*/
	template <typename C, typename A>
	ZList<T>& Sort();

	/*
	public ZList<T>::Split

	Splits the list at the specified location.  The current list is the original list,
	the returned list is the split list.

	@param _itr - iterator location to split (set to first element of split list after this operation)
	@param _mode - integer signifying what to do with the element at _itr.  If negative, includes it
		in the original list.  If 0, discards it, and if positive, includes it in the split list.
	@return - the split list
	*/
	ZList<T> Split(Iterator& _itr, const int _mode);

	/*
	public ZList<T>::Split
	
	Splits the list at the specified location.  The current list is the original list, and the returned
	list is the split list.  The element at _itr is included in the split list.
	
	@param _itr - iterator location to split (set to first element of split list after this operation)
	@return (ZList<T>) - the split list
	*/
	ZList<T> Split(Iterator& _itr);

	/*
	public ZList<T>::Swap

	Swaps the contents of this list with the contents of another.
	
	@param _other - the list to swap contents with
	@return (ZList<T>&) - this list
	*/
	ZList<T>& Swap(ZList<T>& _other);

	/*
	public ZList<T>::SwapElements

	Swaps the elements at two locations in this list.  Both iterators are modified
	to point to ZNew nodes, but they remain at the same location.

	@param _itr1 - iterator to the first element
	@param _itr2 - iterator to the second element
	@return (ZList<T>&) - this list
	*/
	ZList<T>& SwapElements(Iterator& _itr1, Iterator& _itr2);
};

template <typename T>
ZList<T>::ZList()
: AllocatorInstance(NULL)
{
	First = &EmptyNode;
	Last = First; 

	CheckIntegrity();
}

template <typename T>
ZList<T>::ZList(ZListAllocator<T>* _allocator)
: AllocatorInstance(_allocator)
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(AllocatorInstance != NULL, "ZList constructed using NULL allocator!");
	#endif

	First = &EmptyNode;
	Last = First; 

	CheckIntegrity();
}

template <typename T>
ZList<T>::ZList(const Iterator& _begin, const Iterator& _end)
: AllocatorInstance(NULL)
{
	typename ZList<T>::Iterator itr;

	First = &EmptyNode;
	Last = First; 

	for (itr = _begin; itr != _end; itr++)
		PushBack((*itr));

	CheckIntegrity();
}

template <typename T>
ZList<T>::ZList(const Iterator& _begin, const Iterator& _end, ZListAllocator<T>* _allocator)
: AllocatorInstance(_allocator)
{
	typename ZList<T>::Iterator itr;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(AllocatorInstance != NULL, "ZList constructed using NULL allocator!");
	#endif

	First = EmptyNode;
	Last = First; 

	for (itr = _begin; itr != _end; itr++)
		PushBack((*itr));
	
	CheckIntegrity();
}

template <typename T>
ZList<T>::ZList(const ZList<T>& _other)
: AllocatorInstance(_other.AllocatorInstance == NULL ? NULL : _other.AllocatorInstance->Clone())
{
	typename ZList<T>::Iterator itr;

	First = &EmptyNode;
	Last = First; 

	_other.CheckIntegrity();

	for (itr = _other.Begin(); itr != _other.End(); itr++)
		PushBack((*itr));
	
	CheckIntegrity();
}

template <typename T>
ZList<T>::~ZList()
{
	//This deletes all the nodes
	Clear();

	if (AllocatorInstance != NULL)
		AllocatorInstance->Destroy();
}

template <typename T>
ZList<T>& ZList<T>::operator = (const ZList<T>& _other)
{
	typename ZList<T>::Iterator itr;

	if (this == &_other)
		return *this;

	Clear(); //Calls CheckIntegrity

	for (itr = _other.Begin(); itr != _other.End(); itr++)
		PushBack((*itr)); //Calls Check Integrity

	return *this;
}

template <typename T>
bool ZList<T>::operator == (const ZList<T>& _other) const
{
	typename ZList<T>::Iterator itr1;
	typename ZList<T>::Iterator itr2;

	if (this == &_other)
		return true;

	itr1 = Begin();
	itr2 = _other.Begin();

	for (itr1 = Begin(), itr2 = _other.Begin(); itr1 != End() && itr2 != _other.End(); itr1++, itr2++)
	{
		if ((*itr1) != (*itr2))
			return false;
	}

	if (itr1 == End() && itr2 == _other.End())
		return true;
	
	return false;
}

template <typename T>
bool ZList<T>::operator != (const ZList<T>& _other) const
{
	return !(*this == _other);
}

template <typename T>
T& ZList<T>::Back() const
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(!Empty(), "No Back element in ZList!");
	#endif

	return Last->Previous->Element;
}

template <typename T>
ZList<T>& ZList<T>::Clear()
{
	typename ZList<T>::Iterator temp = Begin();

	return Clear(temp);
}

template <typename T>
ZList<T>& ZList<T>::Clear(Iterator& _itr)
{
	ZListNode<T>* next;
	ZListNode<T>* current = _itr.GetNode();

	CheckIntegrity();

	//Early out
	if (Empty())
		return *this;

	//Start clearing elements and getting rid of pointers, but check those corner cases!
	if (current == First)
	{
		First = Last;
		First->Previous = NULL;
	}
	else
	{
		current->Previous->Next = Last;
		Last->Previous = current->Previous;
	}

	while (current != NULL && current != Last)
	{
		next = current->Next;
		DeallocateNode(current);
		current = next;
	}

	_itr.SetNode(Last);

	CheckIntegrity();

	return *this;
}

template <typename T>
size_t ZList<T>::Count(const T& _value) const
{
	size_t i;
	typename ZList<T>::Iterator itr;

	//Pretty simple really.  Iterate and count.
	for (i = 0, itr = Begin(); itr != End(); itr++)
	{
		if ((*itr) == _value)
			i++;
	}

	return i;
}

template <typename T>
bool ZList<T>::Empty() const
{
	return (First == Last);
}

template <typename T>
T ZList<T>::Erase(Iterator& _itr)
{
	T elem;
	ZListNode<T> *node = _itr.GetNode();

	ZSTL_ASSERT(_itr.GetNode() != NULL, "ZList Iterator is invalid!");
	ZSTL_ASSERT(_itr.GetNode()->Next != NULL, "Cannot erase list end!");

	//Increment the iterator to the next list node to keep the iterator valid
	++_itr;
	
	//Rearrange the pointers
	if (node->Previous != NULL)
		node->Previous->Next = node->Next;
	else
		First = node->Next;

	node->Next->Previous = node->Previous;

	elem = node->Element;

	//Delete the removed node and return
	ZSTL_ASSERT(node != &EmptyNode, "Attempting to delete EmptyNode...");
	
	DeallocateNode(node);

	CheckIntegrity();

	return elem;
}

template <typename T>
ZList<T>& ZList<T>::Erase(Iterator& _from, Iterator& _to)
{
	while (_from != _to)
		Erase(_from);

	return *this;
}

template <typename T>
T& ZList<T>::Front() const
{
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(!Empty(), "No Front element in ZList!");
	#endif

	return First->Element;
}

template <typename T>
ZList<T>& ZList<T>::Insert(Iterator& _itr, const T& _value, bool _resetIterator)
{
	ZListNode<T> *node = AllocateNode();

	//Get our data
	node->Element = _value;
	node->Next = _itr.GetNode();
	node->Previous = _itr.GetNode()->Previous;

	//Swap the pointers
	if (_itr.GetNode()->Previous != NULL)
		_itr.GetNode()->Previous->Next = node;

	_itr.GetNode()->Previous = node;

	//If we should reset, then reset
	if (_resetIterator)
		_itr--;

	CheckIntegrity();

	return *this;
}

template <typename T>
ZList<T>& ZList<T>::Insert(Iterator& _itr, ZList<T>& _other, bool _resetIterator)
{
	if (_other.Empty())
		return *this;

	int size = _other.Size();

	//Time to rearrange the pointers
	if (_itr.GetNode() != First)
	{
		_itr.GetNode()->Previous->Next = _other.First;
		_other.First->Previous = _itr.GetNode()->Previous;
	}

	_itr.GetNode()->Previous = _other.Last->Previous;
	_other.Last->Previous->Next = _itr.GetNode();

	_other.First = _other.Last;
	_other.Last->Previous = NULL;

	//If we are supposed to reset our iterator, do so, then return
	if(_resetIterator)
	{
		while(size > 0)
		{
			--_itr;
			size--;
		}
	}

	CheckIntegrity();

	return *this;
}

template <typename T>
T ZList<T>::PopBack()
{
	T elem;
	ZListNode<T> *node;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(!Empty(), "Cannot pop from empty list!");
	#endif

	node = Last->Previous;
	
	if (node->Previous != NULL)
		node->Previous->Next = Last;
	else
		First = Last;
	
	Last->Previous = node->Previous;

	elem = node->Element;

	DeallocateNode(node);

	CheckIntegrity();

	return elem;
}

template <typename T>
T ZList<T>::PopFront()
{
	T elem;
	ZListNode<T> *node;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(!Empty(), "Cannot pop from empty list!");
	#endif

	node = First;

	node->Next->Previous = NULL;

	elem = node->Element;

	First = node->Next;

	DeallocateNode(node);

	CheckIntegrity();

	return elem;
}

template <typename T>
ZList<T>& ZList<T>::PushBack(const T& _value)
{
	ZListNode<T> *node = AllocateNode();

	node->Element = _value;
	node->Next = Last;
	node->Previous = Last->Previous;

	if (Last->Previous != NULL)
		Last->Previous->Next = node;
	else
		First = node;

	Last->Previous = node;

	CheckIntegrity();

	return *this;
}

template <typename T>
ZList<T>& ZList<T>::PushFront(const T& _value)
{
	ZListNode<T> *node = AllocateNode();

	node->Element = _value;
	node->Next = First;

	First->Previous = node;
	First = node;

	CheckIntegrity();

	return *this;
}

template <typename T>
ZList<T>& ZList<T>::Remove(const T& _value)
{
	//A typename is needed here for non msvc compilers... many programmer lives were lost bringing you this information...
	typename ZList<T>::Iterator temp = Begin();

	return Remove(_value, temp); //Calls Check Integrity
}

template <typename T>
ZList<T>& ZList<T>::Remove(const T& _value, Iterator& _itr)
{
	while (_itr != End())
	{
		if ((*_itr) == _value)
		{
			Erase(_itr);
			return *this;
		}

		_itr++;
	}

	CheckIntegrity();

	return *this;
}

template <typename T>
ZList<T>& ZList<T>::RemoveAll(const T& _value)
{
	//Again...
	typename ZList<T>::Iterator itr = Begin();

	return RemoveAll(_value, itr); //Calls Check Integrity
}

template <typename T>
ZList<T>& ZList<T>::RemoveAll(const T& _value, Iterator& _itr)
{
	while (_itr != End())
		Remove(_value, _itr); //Calls Check Integrity

	return *this;
}

template <typename T>
size_t ZList<T>::Size() const
{
	size_t i;
	ZListNode<T> *node;

	for (i = 0, node = First; node != Last; i++, node = node->Next);

	return i;
}

template <typename T>
ZList<T>& ZList<T>::Sort(const ZComparator<T>& _comparator, const ZListSortAlgorithm<T>& _algorithm)
{
	if ( !Empty() && !(First->Next == Last) )
	{
		_algorithm.Sort(_comparator, First, Last);

		while (First->Previous != NULL)
			First = First->Previous;

		while (Last->Next != NULL)
			Last = Last->Next;

		CheckIntegrity();
	}

	return *this;
}

template <typename T> template <typename C>
ZList<T>& ZList<T>::Sort(const ZListSortAlgorithm<T>& _algorithm)
{
	C comparator;

	return Sort(comparator, _algorithm);
}

template <typename T> template <typename A>
ZList<T>& ZList<T>::Sort(const ZComparator<T>& _comparator)
{
	A algo;

	return Sort(_comparator, algo);
}

template <typename T> template <typename C, typename A>
ZList<T>& ZList<T>::Sort()
{
	C comparator;
	A algo;

	return Sort(comparator, algo);
}

template <typename T>
ZList<T> ZList<T>::Split(Iterator& _itr)
{
	return Split(_itr, 1);
}

template <typename T>
ZList<T> ZList<T>::Split(Iterator& _itr, const int _mode)
{
	if (_itr.GetNode() == Last)
	{
		ZList<T> emptyList;
		
		_itr.SetNode(emptyList.First);

		return emptyList;
	}

	if (_mode > 0)
	{
		//Include _itr.Node in splitList
		ZList<T> splitList(_itr, End());

		Clear(_itr);

		_itr.SetNode(splitList.First);

		CheckIntegrity();

		return splitList;

	}
	else if (_mode == 0)
	{
		//Discard _itr.Node
		ZList<T> splitList(++_itr, End());

		Clear(--_itr);

		_itr.SetNode(splitList.First);

		CheckIntegrity();

		return splitList;
	}
	else
	{
		//Keep _itr.Node in this list
		ZList<T> splitList(++_itr, End());
	
		Clear(_itr);

		CheckIntegrity();

		return splitList;
	}
}

template <typename T>
ZList<T>& ZList<T>::Swap(ZList<T>& _other)
{
	ZListNode<T>* first;
	ZListNode<T>* last;
	ZListAllocator<T>* allocator;

	first = First;
	last = Last->Previous;
	allocator = AllocatorInstance;

	if (Empty() && _other.Empty())
	{
		//Nothing to Swap
	}
	else if (Empty())
	{
		First = _other.First;
		Last->Previous = _other.Last->Previous;
		Last->Previous->Next = Last;

		_other.First = _other.Last;
	}
	else if (_other.Empty())
	{
		_other.First = First;
		_other.Last->Previous = Last->Previous;
		_other.Last->Previous->Next = _other.Last;

		First = Last;
	}
	else
	{
		First = _other.First;
		_other.First = first;

		Last->Previous = _other.Last->Previous;
		Last->Previous->Next = Last;

		_other.Last->Previous = last->Previous;
		_other.Last->Previous->Next = _other.Last->Previous->Next;
	}

	AllocatorInstance = _other.AllocatorInstance;
	_other.AllocatorInstance = allocator;

	CheckIntegrity();
	_other.CheckIntegrity();

	return *this;
}

template <typename T>
ZList<T>& ZList<T>::SwapElements(Iterator& _itr1, Iterator& _itr2)
{
	ZListNode<T>* next;
	ZListNode<T>* prev;

	//Verify neither iterator points to 'End'
	_itr1.CheckNodeNext();
	_itr2.CheckNodeNext();

	//Make sure they aren't the same iterator
	if (_itr1 == _itr2)
		return *this;

	//Start swapping pointers around (a lot of edge cases here)
	if(First == _itr1.GetNode())
		First = _itr2.GetNode();
	else if(First == _itr2.GetNode())
		First = _itr1.GetNode();

	next = _itr1.GetNode()->Next;
	prev = _itr1.GetNode()->Previous;

	_itr1.GetNode()->Next = _itr2.GetNode()->Next;
	_itr1.GetNode()->Previous = _itr2.GetNode()->Previous;

	_itr2.GetNode()->Next = next;
	_itr2.GetNode()->Previous = prev;
	
	if (_itr1.GetNode()->Next != NULL)
	{
		if(_itr1.GetNode()->Next == _itr1.GetNode())
			_itr1.GetNode()->Next = _itr2.GetNode();
		else
			_itr1.GetNode()->Next->Previous = _itr1.GetNode();
	}

	if (_itr1.GetNode()->Previous != NULL)
	{
		if(_itr1.GetNode()->Previous == _itr1.GetNode())
			_itr1.GetNode()->Previous = _itr2.GetNode();
		else
			_itr1.GetNode()->Previous->Next = _itr1.GetNode();
	}

	if (_itr2.GetNode()->Next != NULL)
	{
		if (_itr2.GetNode()->Next == _itr2.GetNode())
			_itr2.GetNode()->Next = _itr1.GetNode();
		else
			_itr2.GetNode()->Next->Previous = _itr2.GetNode();
	}

	if (_itr2.GetNode()->Previous != NULL)
	{
		if (_itr2.GetNode()->Previous == _itr2.GetNode())
			_itr2.GetNode()->Previous = _itr1.GetNode();
		else
			_itr2.GetNode()->Previous->Next = _itr2.GetNode();
	}

	CheckIntegrity();

	return *this;
}

#endif
