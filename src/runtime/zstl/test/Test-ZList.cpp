/*
	Test-ZList.cpp
	Author: Patrick Baggett

	Purpose :	Unit tests for ZList implementation to ensure correct behavior

	Changelog :
	12/18/2011 - Removed dependency on ZString (cre1)
	12/15/2010 - Created (ptb1)
*/

#include "ZUnitTest.h"
#include <ZSTL/ZList.hpp>
#include <stdio.h>
#include <stdlib.h>

static const char* testPushPop();
static const char* testClearEmpty();
static const char* testCount();
static const char* testErase();
static const char* testIterator();
static const char* testFind();
static const char* testFrontBack();
static const char* testRemove();
static const char* testInsert();
static const char* testSort();
static const char* testSplit();
static const char* testSwapElements();
static const char* testOperators();

//List of unit tests
ZUnitTest ZListUnitTests[] =
{
	{
		"List [Push/Pop][Back/Front]()",
		testPushPop,
	},
	{
		"List Clear()/Empty()",
		testClearEmpty
	},
	{
		"List Count()",
		testCount
	},
	{
		"List Erase()",
		testErase
	},
	{
		"List iterator functions",
		testIterator
	},
	{
		"List Find()",
		testFind
	},
	{
		"List Front()/Back()",
		testFrontBack
	},
	{
		"List Remove()",
		testRemove
	},
	{
		"List Insert()",
		testInsert
	},
	{
		"List Sort()",
		testSort
	},
	{
		"List Split()",
		testSplit
	},
	{
		"List SwapElements()",
		testSwapElements
	},
	{
		"List operators",
		testOperators
	}
};

//Now declare the ZUnitTestBlock associated with this.
DECLARE_ZTESTBLOCK(ZList);

template <typename T>
class ZListTestAllocator : public ZListAllocator<T>
{
public:
	//Static Allocation Counter
	static int Count;	

	virtual ZListNode<T>* Allocate() 
	{
		ZListTestAllocator<T>::Count++;

		return new (std::nothrow) ZListNode<T>();
	}

	virtual ZListAllocator<T>* Clone() 
	{
		return new ZListTestAllocator<T>();
	}

	virtual void Deallocate( ZListNode<T>* _node ) 
	{
		ZListTestAllocator<T>::Count--;

		delete _node;
	}

	virtual void Destroy()
	{
		delete this;
	}

};

template <>
int ZListTestAllocator<int>::Count = 0;

/******************************************************************************/

static const char* testPushPop()
{
	//Scope so we call the destructor
	{
		ZList<int> list(new ZListTestAllocator<int>());

		int i;

		//Should start empty
		TASSERT(list.Size() == 0, "Size() gave non-zero value but should be empty!\n");

		//Fill in with 5 elements
		for(i=1; i<6; i++)
			list.PushBack(i);

		//So, the size should be 5
		TASSERT(list.Size() == 5, "Size() value wrong after Push()/Pop()!\n");

		//Ensure the elements are correct
		for(i=1; i<6; i++)
		{
			TASSERT(i == list.PopFront(), "PopFront() gave wrong value");
		}

		//Now should be empty
		TASSERT(list.Size() == 0, "Size() gave non-zero value but should be empty!\n");
		TASSERT(ZListTestAllocator<int>::Count == 0, "Failed to correctly deallocate all nodes!");

		//Refill
		for(i=1; i<6; i++)
			list.PushFront(i);

		for(i=1; i<6; i++)
		{
			TASSERT(i == list.PopBack(), "PopBack() gave wrong value");
		}

		//Now should be empty once more
		TASSERT(list.Size() == 0, "Size() gave non-zero value but should be empty!\n");
		TASSERT(ZListTestAllocator<int>::Count == 0, "Failed to correctly deallocate all nodes!");
	}

	TASSERT(ZListTestAllocator<int>::Count == 0, "Failed to correctly deallocate all nodes!");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testClearEmpty()
{
	ZList<int> list;
	ZList<int>::Iterator it;

	list.PushFront(1);
	list.PushBack(2);

	list.Clear();

	//Now should be empty once more
	if(list.Size() != 0)
		return "Size() gave non-zero value but should be empty!\n";

	//Now test Empty()
	if(!list.Empty())
		return "Size() gave 0, but Empty() gave false";
	list.PushFront(1);
	if(list.Empty())
		return "Element just added, but Empty() gave true";

	//Test clear after a certain point
	list.Clear();
	list.PushBack(1);
	list.PushBack(2);
	list.PushBack(3);
	list.PushBack(4);
	list.PushBack(4);
	list.PushBack(4);

	it = list.Begin(); //Points 1
	it++; //Points to 2
	it++; //Points to 3
	it++; //Points to 4 (first one)

	//Clear all 4's
	list.Clear(it);

	//Now test the elements
	if(list.Size() != 3)
		return "Wrong size after Clear() using iterator";
	if(list.Front() != 1)
		return "Wrong front value after Clear() using iterator";
	if(list.Back() != 3)
		return "Wrong back value after Clear() using iterator";


	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testCount()
{
	ZList<int> list;

	//Fill list with 5 items and check for 
	for(int i=0; i<5; i++)
	{
		list.PushBack(1);
	}
	list.PushBack(2); //These add a bit of extra stuff to compare against
	list.PushBack(3);
	if(list.Count(1) != 5)
		return "Count() gave wrong number of items, should be 5";


	//Remove 1 item from the front of the list
	list.PopFront();
	if(list.Count(1) != 4)
		return "Count() gave wrong number of items, should be 4";

	//Empty the list
	list.Clear();
	if(list.Count(1) != 0)
		return "Count() gave wrong number of items, should be 0";

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testErase()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i;

	for(i=0; i<10; i++)
		list.PushBack(i);

	it = list.Begin(); //Points to 0

	it++; //Points to 1
	it++; //Points to 2

	//Remove 2
	list.Erase(it);

	if(list.Size() != 9)
		return "Size() is incorrect after Erase()";

	//Check elements ahead of the removed one.
	if(list.PopFront() != 0) return "Wrong value returned by PopFront() after Erase() (ahead of iterator)";
	if(list.PopFront() != 1) return "Wrong value returned by PopFront() after Erase() (ahead of iterator)";


	//Check elements 3-9 (should be 7 total)
	for(i=0; i<7; i++)
	{
		if(list.PopFront() != i+3)
			return "Wrong value returned by PopFront() after Erase() (behind iterator)";
	}

	
	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testIterator()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i;

	//Add 0-9
	for(i=0; i<10; i++)
		list.PushBack(i);

	//Check going forwards
	for(it=list.Begin(), i=0;  i<10; it++, i++)
	{
		if((*it) != i)
			return "Iterator returned wrong value when dereferenced";
	}

	//Should be at end
	if(list.End() != it)
		return "Iterator should be at start but is not (=/= End())";

	//Check going backwards (End() points past final element)
	it=list.End();
	for(i=9; i>=0; i--)
	{
		it--;
		if((*it) != i)
			return "Iterator returned wrong value when dereferenced";

	}

	//Should be at the beginning now
	if(list.Begin() != it)
		return "Iterator should be at start but is not (=/= Begin())";


	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testFind()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i;

	//Add 0-9
	for(i=0; i<10; i++)
		list.PushBack(i);

	list.PushBack(1000);

	//Find the element with the value '5'
	it = list.Find(5);
	if(it == list.End())
		return "Find() could not find element known to exist";
	if(*it != 5)
		return "Find() found the wrong element";

	//Find an element that does not exist
	it = list.Find(0xdeadc0de);
	if(it != list.End())
		return "Find() erroneously found element known NOT to exist";

	//Find an element 1000, then remove it
	it = list.Find(1000);
	if(it == list.End())
		return "Find() could not find element known to exist";
	if(*it != 1000)
		return "Find() found the wrong element";
	list.Erase(it);

	//Now try to find it
	it = list.Find(1000);
	if(it != list.End())
		return "Find() erroneously found element known NOT to exist";

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testFrontBack()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i;

	//Add 0-9
	for(i=0; i<10; i++)
		list.PushBack(i);

	//Test initial state
	if(list.Front() != 0)
		return "Wrong value from Front()";
	if(list.Back() != 9)
		return "Wrong value from Back()";

	//Add '42' to front, retest
	list.PushFront(42);
	if(list.Front() != 42)
		return "Wrong value from Front()";
	if(list.Back() != 9)
		return "Wrong value from Back()";

	//Remove 42 from front, retest
	list.PopFront();
	if(list.Front() != 0)
		return "Wrong value from Front()";
	if(list.Back() != 9)
		return "Wrong value from Back()";

	//Add '24' to back, retest
	list.PushBack(24);
	if(list.Front() != 0)
		return "Wrong value from Front()";
	if(list.Back() != 24)
		return "Wrong value from Back()";

	//Remove 24 from back, retest
	list.PopBack();
	if(list.Front() != 0)
		return "Wrong value from Front()";
	if(list.Back() != 9)
		return "Wrong value from Back()";


	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testRemove()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i;

	//Add 0-9, then 1, 1, 42 (13 items total)
	for(i=0; i<10; i++)
		list.PushBack(i);
	list.PushBack(1);
	list.PushBack(1);
	list.PushBack(42);
	//List Now: 0,1,2,3,4,5,6,7,8,9,1,1,42

	//Remove 9
	it = list.Begin();
	list.Remove(9, it);
	if(list.Find(9) != list.End())
		return "Singleton element was removed, but Find() found it";
	if(list.Size() != 12)
		return "After remove, Size() is wrong";
	//List Now: 0,1,2,3,4,5,6,7,8,1,1,42

	//Remove the second '1'
	it = list.Find(7);
	list.Remove(1, it);
	it = list.Find(7); //Recompute iterator since changing list typically invalidates
	it = list.Find(1, it);
	if(it == list.End())
		return "Removed one of three elements, unable to other two";
	if(list.Size() != 11)
		return "After remove, Size() is wrong";
	//List Now: 0,1,2,3,4,5,6,7,8,1,42

	//Make sure that Remove() in fact removed the correct '1'
	it++;
	if((*it) != 42)
		return "Remove() removed the wrong element";

	if(list.Front() != 0)
		return "Front() is wrong after Remove()";

	//Remove all '1'
	list.RemoveAll(1);
	if(list.Size() != 9)
		return "Size after RemoveAll() is wrong";
	//List Now: 0,2,3,4,5,6,7,8,42

	if(list.Find(1) != list.End())
		return "Find() found element after RemovalAll() on same type";


	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testInsert()
{
	ZList<int> list, temp;
	ZList<int>::Iterator it;
	int i;

	//Start list with 1,2,3
	list.PushBack(1);
	list.PushBack(2);
	list.PushBack(3);
	it = list.Begin();
	it++;

	//Insert 100, so that the list will look like: 1, 100, 2, 3,
	list.Insert(it, 100, true);
	if(list.Size() != 4)
		return "Wrong size after Insert()";
	if(list.Front() != 1)
		return "Wrong element in front after Insert()";

	//Insert 200 at the end, so that the list will look like: 1, 100, 2, 3, 200
	it = list.End();
	list.Insert(it, 200, false);
	if(list.Size() != 5)
		return "Wrong size after Insert()";
	if(list.Back() != 200)
		return "Wrong element in back after Insert()";

	//Find element 200 (final element)
	it = list.Find(200);
	list.Insert(it, 300, true); //Keep absolute position, iterator should point to 300 not 200
	if(*it != 300)
		return "Incorrectly reset the iterator after inserting value";
	if(list.Size() != 6)
		return "Wrong size after Insert()";
	if(list.Front() != 1)
		return "Wrong element in front after Insert()";
	if(list.Back() != 200)
		return "Wrong element in back after Insert()";


	//Temp = 1000,2000,3000,4000,5000
	for(i=0; i<5; i++)
		temp.PushBack((i+1)*1000);

	//Add sublist behind the 2
	//The list should look like: 1, 100, 1000, 2000, 3000, 4000, 5000, 2, 3, 300, 200
	it = list.Find(2);
	list.Insert(it, temp, true);
	if(list.Size() != 11)
		return "Wrong size after inserting sublist";
	if(*it != 1000)
		return "Incorrectly reset the iterator after inserting list";
	if(!temp.Empty())
		return "After Insert(), sublist should be empty";

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testSort()
{
	ZList<int> list;
	ZList<int>::Iterator it;
	int i, min;


	srand(0xdeadc0de);

	//Add 100 random integers
	for(i=0; i<100; i++)
		list.PushBack( rand() % 128 );

	//Sort
	list.Sort();

	if(list.Size() != 100)
		return "Sort() lost elements";

	//Make sure it is monotonically increasing
	for(it=list.Begin(), min = *it; it != list.End(); it++)
	{
		if(min > (*it))
			return "Found element out of place after Sort()";

		min = *it;
	}

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testSplit()
{
	ZList<int> list1, list2;
	ZList<int>::Iterator it;
	int i;

	//list1 = 1, 2, 3, 4, 5
	for(i=0; i<5; i++)
		list1.PushBack(i+1);

	//Test discard mode
	it = list1.Find(3);
	list2 = list1.Split(it, 0);
	if(list1.Size() != 2)
		return "List 1 after discard split is wrong size";
	if(list2.Size() != 2)
		return "List 2 after discard split is wrong size";
	if(list1.Front() != 1)
		return "List 1 has wrong front value after discard split";
	if(list1.Back() != 2)
		return "List 1 has wrong back value after discard split";
	if(list2.Front() != 4)
		return "List 2 has wrong front value after discard split";
	if(list2.Back() != 5)
		return "List 2 has wrong back value after discard split";

	//Reset list1 = 1, 2, 3, 4, 5
	list1.Clear();
	for(i=0; i<5; i++)
		list1.PushBack(i+1);

	//Test inclusion into original list
	it = list1.Find(3);
	list2 = list1.Split(it, -1);
	if(list1.Size() != 3)
		return "List 1 after negative valued split is wrong size";
	if(list2.Size() != 2)
		return "List 2 after negative valued split is wrong size";
	if(list1.Front() != 1)
		return "List 1 has wrong front value after negative valued split";
	if(list1.Back() != 3)
		return "List 1 has wrong back value after negative valued split";
	if(list2.Front() != 4)
		return "List 2 has wrong front value after negative valued split";
	if(list2.Back() != 5)
		return "List 2 has wrong back value after negative valued split";

	//Reset list1 = 1, 2, 3, 4, 5
	list1.Clear();
	for(i=0; i<5; i++)
		list1.PushBack(i+1);

	//Test inclusion into secondary list
	it = list1.Find(3);
	list2 = list1.Split(it, +1);
	if(list1.Size() != 2)
		return "List 1 after positive valued split is wrong size";
	if(list2.Size() != 3)
		return "List 2 after positive valued split is wrong size";
	if(list1.Front() != 1)
		return "List 1 has wrong front value after positive valued split";
	if(list1.Back() != 2)
		return "List 1 has wrong back value after positive valued split";
	if(list2.Front() != 3)
		return "List 2 has wrong front value after positive valued split";
	if(list2.Back() != 5)
		return "List 2 has wrong back value after positive valued split";

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testSwapElements()
{
	ZList<int> list;
	ZList<int>::Iterator e1, e2, it;
	int i;

	//Create list: 1,2,5,4,3
	//We'll swap 3 & 5 to create: 1,2,3,4,5
	list.PushBack(1);
	list.PushBack(2);
	list.PushBack(5);
	list.PushBack(4);
	list.PushBack(3);

	//Find elements 3 and 5
	e1 = list.Find(5);
	e2 = list.Find(3);

	//Swap them
	list.SwapElements(e1, e2);

	//Verify swap worked, since now they will be sorted
	for(i=1, it=list.Begin(); it != list.End(); it++, i++)
	{
		if((*it) != i)
			return "SwapElements() did not correctly the elements";
	}

	//Try making a list of two elements and swapping them (i.e. swapping front and back)
	list.Clear();
	list.PushBack(100);
	list.PushBack(200);
	e1 = list.Find(100);
	e2 = list.Find(200);
	list.SwapElements(e1, e2);

	if(list.Front() != 200)
		return "SwapElements() did not correctly swap front and back elements";
	if(list.Back() != 100)
		return "SwapElements() did not correctly swap front and back elements";

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testOperators()
{
	ZList<int> list1, list2;
	int i;

	//Setup two identical lists
	for(i=0; i<5; i++)
	{
		list1.PushBack(1+i);
		list2.PushBack(1+i);
	}

	//Test operator []
	for(i=0; i<5; i++)
	{
		if(list1[i].Get() != i+1)
			return "Operator [] returned wrong value";
	}


	//Test operator [] with negative indexing
	#if (ZSTL_DISABLE_NEGATIVE_INDEXING == 0)
	for(i=-1; i>-6; i--) //i goes from -1 to -5
	{
		if(list1[i].Get() != 6+i) //Values go from 5 to 1
			return "Operator [] returned wrong value";
	}
	#endif

	//Test operator ==. Since they are equal, this should return true, so fail if it returns false
	if(!(list1 == list2))
		return "Operator == returned false on two equal objects";

	//Test operator !=. Since they are equal, this should return false, so fail if it returns true
	if(list1 != list2)
		return "Operator != returned true on two equal objects";

	//Okay, now they aren't equal
	list1.PopBack();
	if(list1 == list2)
		return "Operator == returned true on two unequal objects";
	if(!(list1 != list2))
		return "Operator != returned false on two unequal objects";

	return ZTEST_SUCCESS;
}
