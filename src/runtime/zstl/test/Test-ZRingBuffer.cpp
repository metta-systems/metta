/*
	Test-ZRingBuffer.cpp
	Author: Chris Ertel (cre1)

	Purpose: Unit tests for ZRingBuffer

	Changelog
	2011/12/04 - creation (cre1)
*/

#include "ZUnitTest.h"
#include <stdio.h>
#include <stdlib.h>

#include <ZSTL/ZRingBuffer.hpp>

/* empty can-has tests */
static const char* testIsEmptyWhenEmpty();
static const char* testIsFullWhenEmpty();
static const char* testCanPopEmpty();
static const char* testCanPeekEmpty();
static const char* testCanPushEmpty();

/* full can-has tests */
static const char* testIsEmptyWhenFull();
static const char* testIsFullWhenFull();
static const char* testCanPopFull();
static const char* testCanPeekFull();
static const char* testCanPushFull();

/* filling operations */
static const char* testPushFrontSingle();
static const char* testPushFrontMultiple();
static const char* testPushBackSingle();
static const char* testPushBackMultiple();

/* accessing operations */
static const char* testPeekFront();
static const char* testPeekBack();
static const char* testPopFrontSingle();
static const char* testPopFrontMultiple();
static const char* testPopBackSingle();
static const char* testPopBackMultiple();
static const char* testFillArray();

/* test complex usage */
static const char* testStacking();
static const char* testBackStacking();
static const char* testQueueing();
static const char* testBackQueueing();
static const char* testAlternatingPushes();
static const char* testQueueingWithResize();

//List of unit tests
ZUnitTest ZRingBufferUnitTests[] =
{
	/* empty tests */
	{
		"IsEmpty() when empty",
		testIsEmptyWhenEmpty
	},
	{
		"IsFull() when empty",
		testIsFullWhenEmpty
	},
	{
		"CanPop() when empty",
		testCanPopEmpty
	},
	{
		"CanPeek() when empty",
		testCanPeekEmpty
	},
	{
		"CanPush() when empty",
		testCanPushEmpty
	},

	/* filling tests */
	{
		"Pushing front one element",
		testPushFrontSingle
	},
	{
		"Pushing front many elements",
		testPushFrontMultiple
	},
	{
		"Pushing back one element",
		testPushBackSingle
	},
	{
		"Pushing back many element",
		testPushBackMultiple
	},

	/* full tests */
	{
		"IsEmpty() when full",
		testIsEmptyWhenFull
	},
	{
		"IsFull() when full",
		testIsFullWhenFull
	},
	{
		"CanPop() when full",
		testCanPopFull
	},
	{
		"CanPeek() when full",
		testCanPeekFull
	},
	{
		"CanPush() when full",
		testCanPushFull
	},

	/* accessing tests */
	{
		"Peeking front",
		testPeekFront
	},
	{
		"Peeking back",
		testPeekBack
	},
	{
		"Popping front one element",
		testPopFrontSingle
	},
	{
		"Popping front many elements",
		testPopFrontMultiple
	},
	{
		"Popping back one element",
		testPopBackSingle
	},
	{
		"Popping back many elements",
		testPopBackMultiple
	},
	{
		"Filling an array from a ring buffer",
		testFillArray
	},

	/* complex usage */
	{
		"Using as a stack with 5 elements, pushing front",
		testStacking
	},
	{
		"Using as a stack with 5 elements, pushing back",
		testBackStacking
	},
	{
		"Using as a queue with 5 elements, pushing front",
		testQueueing
	},
	{
		"Using as a queue with 5 elements, pushing back",
		testBackQueueing
	},
	{
		"Alternating Push Front/Back, reading from front",
		testAlternatingPushes
	},
	{
		"Using as a queue with 5 elements, pushing front, resizing to 10, and adding 3",
		testQueueingWithResize
	}
};

DECLARE_ZTESTBLOCK(ZRingBuffer);

/* empty can-has tests */
static const char* testIsEmptyWhenEmpty()
{
	ZRingBuffer<int> buffer(10);

	if (buffer.IsEmpty() == true)
		return ZTEST_SUCCESS;
	else
		return "returned false on IsEmpty() when empty!\n";
}
static const char* testIsFullWhenEmpty()
{
	ZRingBuffer<int> buffer(10);

	if (buffer.IsFull() == false)
		return ZTEST_SUCCESS;
	else
		return "returned true on IsFull() when empty!\n";
}
static const char* testCanPopEmpty()
{
	ZRingBuffer<int> buffer(10);

	if (buffer.CanPop() == false)
		return ZTEST_SUCCESS;
	else
		return "returned true on CanPop() when empty!\n";
}
static const char* testCanPeekEmpty()
{
	ZRingBuffer<int> buffer(10);

	if (buffer.CanPeek() == false)
		return ZTEST_SUCCESS;
	else
		return "returned true on CanPeek() when empty!\n";
}
static const char* testCanPushEmpty()
{
	ZRingBuffer<int> buffer(10);

	if (buffer.CanPush() == true)
		return ZTEST_SUCCESS;
	else
		return "returned false on CanPush() when empty!\n";
}

/* filling operations */
static const char* testPushFrontSingle()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	count = buffer.GetSize();

	if (count != 1)
		return "after 1 front push, GetSize() returned wrong number of elements!\n";

	return ZTEST_SUCCESS;
}
static const char* testPushFrontMultiple()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(451);
	buffer.PushFront(762);
	count = buffer.GetSize();
	if (count != 3)
		return "after 3 front pushes, GetSize() returned wrong number of elements!\n";

	return ZTEST_SUCCESS;
}
static const char* testPushBackSingle()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushBack(1337);
	count = buffer.GetSize();
	if (count != 1)
		return "after 1 back push, GetSize() returned wrong number of elements!\n";

	return ZTEST_SUCCESS;
}
static const char* testPushBackMultiple()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(451);
	buffer.PushFront(762);
	count = buffer.GetSize();
	if (count != 3)
		return "after 3 back pushes, GetSize() returned wrong number of elements!\n";

	return ZTEST_SUCCESS;
}

/* full can-has tests */
static const char* testIsEmptyWhenFull()
{
	ZRingBuffer<int> buffer(10);
	for (int i = 0; i < 10; i++)
		buffer.PushFront(i);

	if (buffer.IsEmpty() != false)
		return "returned true on IsEmpty() when full!\n";
		
	return ZTEST_SUCCESS;
}
static const char* testIsFullWhenFull()
{
	ZRingBuffer<int> buffer(10);
	for (int i = 0; i < 10; i++)
		buffer.PushFront(i);

	if (buffer.IsFull() == true)
		return ZTEST_SUCCESS;
	else
		return "returned false on IsFull() when full!\n";
}
static const char* testCanPopFull()
{
	ZRingBuffer<int> buffer(10);
	for (int i = 0; i < 10; i++)
		buffer.PushFront(i);

	if (buffer.CanPop() == true)
		return ZTEST_SUCCESS;
	else
		return "returned false on CanPop() when full!\n";
}

static const char* testCanPeekFull()
{
	ZRingBuffer<int> buffer(10);
	for (int i = 0; i < 10; i++)
		buffer.PushFront(i);

	if (buffer.CanPeek() == true)
		return ZTEST_SUCCESS;
	else
		return "returned false on CanPeek() when full!\n";
}

static const char* testCanPushFull()
{
	ZRingBuffer<int> buffer(10);
	for (int i = 0; i < 10; i++)
		buffer.PushFront(i);

	if (buffer.CanPush() == false)
		return ZTEST_SUCCESS;
	else
		return "returned true on CanPush() when full!\n";
}

/* accessing operations */
static const char* testPeekFront()
{
	int temp;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(451);
	temp = buffer.PeekFront();
	if (temp == 451)
		return ZTEST_SUCCESS;
	else
		return "PeekFront() returned something instead of 451!\n";
}

static const char* testPeekBack()
{
	int temp;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(451);
	temp = buffer.PeekBack();
	if (temp == 1337)
		return ZTEST_SUCCESS;
	else
		return "PeekBack() returned something other than 1337!\n";
}

static const char* testPopFrontSingle()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	count = buffer.GetSize();
	if (count != 1)
		return "after 1 front push, GetSize() returned something other than 1!\n";
	
	buffer.PopFront();
	count = buffer.GetSize();
	if (count != 0)
		return "after 1 front pop, GetSize() returned something other than 0!\n";

	return ZTEST_SUCCESS;
}

static const char* testPopFrontMultiple()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(1336);
	buffer.PushFront(1335);
	
	count = buffer.GetSize();
	if (count != 3)
		return "after 3 front push, GetSize() returned something other than 3!\n";

	buffer.PopFront();
	count = buffer.GetSize();
	if (count != 2)
		return "after 1 front pop, GetSize() returned something other than 2!\n";

	buffer.PopFront();
	count = buffer.GetSize();
	if (count != 1)
		return "after 2 front pops, GetSize() returned something other than 1!\n";
	
	buffer.PopFront();
	count = buffer.GetSize();
	if (count != 0)
		return "after 3 front pops, GetSize() returned something other than 0!\n";
	
	return ZTEST_SUCCESS;
}

static const char* testPopBackSingle()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	count = buffer.GetSize();
	if (count != 1)
		return "after 1 front push, GetSize() returned something other than 1!\n";

	buffer.PopBack();
	count = buffer.GetSize();
	if (count != 0)
		return "after 1 back pop, GetSize() returned something other than 0!\n";

	return ZTEST_SUCCESS;
}

static const char* testPopBackMultiple()
{
	int count;
	ZRingBuffer<int> buffer(10);
	buffer.PushFront(1337);
	buffer.PushFront(1336);
	buffer.PushFront(1335);

	count = buffer.GetSize();
	if (count != 3)
		return "after 3 front push, GetSize() returned something other than 3!\n";

	buffer.PopBack();
	count = buffer.GetSize();
	if (count != 2)
		return "after 1 back pop, GetSize() returned something other than 2!\n";

	buffer.PopBack();
	count = buffer.GetSize();
	if (count != 1)
		return "after 2 back pops, GetSize() returned something other than 1!\n";

	buffer.PopBack();
	count = buffer.GetSize();
	if (count != 0)
		return "after 3 back pops, GetSize() returned something other than 0!\n";

	return ZTEST_SUCCESS;
}

static const char* testFillArray()
{
	ZRingBuffer<int> buffer(10);
	int temp[5];

	buffer.PushFront(0);
	buffer.PushFront(1);
	buffer.PushFront(2);
	buffer.PushFront(3);
	buffer.PushFront(4);
	buffer.CopyToArray(temp);
	
	for (int i = 0; i < 5; i++)
	{
		if (temp[i] != i)
			return "found wrong fill in array!\n";
	}

	return ZTEST_SUCCESS;
}


static const char* testStacking()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(10);
	int testArray[] = {0,1,2,3,4};
	
	for (int i = 0; i < 5; i++)
		buffer.PushFront(testArray[i]);

	count = buffer.GetSize();
	if (count != 5)
		return "after 5 front pushes, GetSize() returned something other than 5!\n";

	for (int i = 0; i < 5; i++)
	{
		temp = buffer.PopFront();
		if (temp != testArray[4-i])
			return "ring buffer corrupted!\n";
	}

	return ZTEST_SUCCESS;
}

static const char* testBackStacking()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(10);
	int testArray[] = {0,1,2,3,4};

	for (int i = 0; i < 5; i++)
		buffer.PushBack(testArray[i]);

	count = buffer.GetSize();
	if (count != 5)
		return "after 5 back pushes, GetSize() returned something other than 5!\n";

	for (int i = 0; i < 5; i++)
	{
		temp = buffer.PopBack();
		if (temp != testArray[4-i])
			return "ring buffer corrupted\n";
	}

	return ZTEST_SUCCESS;
}

static const char* testQueueing()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(10);
	int testArray[] = {0,1,2,3,4};

	for (int i = 0; i < 5; i++)
		buffer.PushFront(testArray[i]);

	count = buffer.GetSize();
	if (count != 5)
		return "after 5 front pushes, GetSize() returned something other than 5!\n";

	for (int i = 0; i < 5; i++)
	{
		temp = buffer.PopBack();
		if (temp != testArray[i])
			return "ring buffer corrupted!\n";
	}

	return ZTEST_SUCCESS;
}

static const char* testBackQueueing()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(10);
	int tempArray[5];
	int testArray[] = {0,1,2,3,4};

	for (int i = 0; i < 5; i++)
		buffer.PushBack(testArray[i]);

	count = buffer.GetSize();
	if (count != 5)
		return "after 5 back pushes, GetSize() returned something other than 5!\n";

	
	buffer.CopyToArray(tempArray);

	for (int i = 0; i < 5; i++)
	{
		temp = buffer.PopFront();
		if (temp != testArray[i])
			return "ring buffer corrupted!\n";
	}

	return ZTEST_SUCCESS;
}

static const char* testAlternatingPushes()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(10);
	int testArray[] = {0,1,2,3,4,5,6,7};
	int correctResults[] = {7,5,3,1,0,2,4,6};
	int tempArray[8];

	buffer.PushFront(testArray[0]);
	buffer.PushBack(testArray[1]);
	buffer.PushFront(testArray[2]);
	buffer.PushBack(testArray[3]);
	buffer.PushFront(testArray[4]);
	buffer.PushBack(testArray[5]);
	buffer.PushFront(testArray[6]);
	buffer.PushBack(testArray[7]);

	count = buffer.GetSize();
	if (count != 8)
		return "after 8 pushes, ring buffer returned something other than 8!\n";

	for (int i = 0; i < 8; i++)
		tempArray[i] = -1;

	buffer.CopyToArray(tempArray);

	for (int i = 0; i < 8; i++)
	{
		temp = buffer.PopBack();
		if (temp != correctResults[i])
			return "ring buffer corrupted!\n";
	}

	return ZTEST_SUCCESS;
}

static const char* testQueueingWithResize()
{
	int count;
	int temp;
	ZRingBuffer<int> buffer(5);
	int testArray[] = {0,1,2,3,4,5,6,7};

	buffer.PushFront(42);
	buffer.PushFront(42);
	buffer.PopBack();
	buffer.PopBack();
	buffer.PushFront(0);
	buffer.PushFront(1);
	buffer.PushFront(2);
	buffer.PushFront(3);
	buffer.PushFront(4);
	count = buffer.GetSize();
	if (count != 5)
		return "after 5 front pushes, GetSize() returned something other than 5!\n";

	buffer.Reserve(10);
	buffer.PushFront(5);
	buffer.PushFront(6);
	buffer.PushFront(7);

	for (int i = 0; i < 8; i++)
	{
		temp = buffer.PopBack();
		if (temp != testArray[i])
			return "ring buffer corrupted!\n";
	}

	return ZTEST_SUCCESS;
}