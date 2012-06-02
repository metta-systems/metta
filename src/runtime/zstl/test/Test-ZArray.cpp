/*
	Test-ZArray.cpp
	Author: Patrick Baggett

	Purpose :	Unit tests for ZArray implementation to ensure correct behavior

	Changelog :
	12/18/2011 - Removed dependency on ZString (cre1)
	12/18/2010 - Created (ptb1)
*/

#include "ZUnitTest.h"
#include <stdio.h>
#include <stdlib.h>

static const char* testSizeCapTrim();
static const char* testConcatOperators();
static const char* testOperators();
static const char* testShift();
static const char* testSort();

//List of unit tests
ZUnitTest ZArrayUnitTests[] =
{
	{
		"Array Size(), Capacity(), Trim()",
		testSizeCapTrim
	},
	{
		"Array +/+=/==/= operators",
		testConcatOperators,
	},
	{
		"Array operators",
		testOperators,
	},
	{
		"Array Shift()/Unshift()",
		testShift
	},
	{
		"Array Sort()",
		testSort
	}

};

DECLARE_ZTESTBLOCK(ZArray);

/******************************************************************************/

static const char* testSizeCapTrim()
{
	ZArray<int> arr(5);

	//Verify constructor/Capacity() function worked
	TASSERT(arr.Capacity() == 5, "Capacity() returns wrong size after constructing with explicit capacity");

	//Ensure zero size before we start
	TASSERT(arr.Size() == 0, "Size() gave non-zero value but array is empty");

	arr.Push(1);
	arr.Push(2);

	//Ensure proper size
	TASSERT(arr.Size() == 2, "Size() gave wrong value after Push()");

	//Trim excess capacity
	arr.Trim();

	TASSERT(arr.Capacity() == 2, "Trim() did not reduce capacity to proper value");


	arr.Reserve(4);

	TASSERT(arr.Capacity() == 4, "Reserve() did not reserve enough capacity");
	TASSERT(arr.Size() == 2, "Reserve() modified the size");
	TASSERT(arr[0] == 1 && arr[1] == 2, "Reserve() modifed elements");


	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testConcatOperators()
{
	ZArray<int> a1, a2, sum;

	//A1 = 1,2 / A2 = 3,4,5
	a1.Push(1);
	a1.Push(2);
	a2.Push(3);
	a2.Push(4);
	a2.Push(5);

	//Check the [] operators
	TASSERT(a1[0] == 1 && a1[1] == 2, "Operator [] returned wrong value");

	//Construct a list by concatenating the two
	sum = a1 + a2;

	//Ensure proper size
	TASSERT(sum.Size() == 5, "Operator + returned array with wrong size");

	//Now ensure element-wise consistency
	for(int i=0; i<5; i++)
	{
		TASSERT(sum[i] == i+1, "Operator + constructed list with wrong values");
	}

	//Copy using = operator
	sum = a1;

	TASSERT(sum.Size() == 2, "Operator = did not copy the correct size");
	TASSERT(sum[0] == 1 && sum[1] == 2, "Operator = did not copy the proper values");

	//Concatenate with A2
	sum += a2;

	TASSERT(sum.Size() == 5, "Operator += did not copy the correct size");

	for(int i=0; i<5; i++)
	{
		TASSERT(sum[i] == i+1, "Operator += did not copy the proper values");
	}

	//Make an equivalent list from scratch
	ZArray<int> test;

	for(int i=0; i<5; i++)
		test.Push(i+1);

	//Compare then using ==
	TASSERT(sum == test, "Operator == returned false on equivalent objects");

	//Make the lists different, then try !=
	test.Pop();

	TASSERT(sum != test, "Operator != returned false on unequal objects");

	//Test casting operator
	int* ptr = (int*)sum;

	for(int i=0; i<5; i++)
	{
		TASSERT(ptr[i] == i+1, "Pointer returned by casting operator points to useless data");
	}

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testOperators()
{
	int i, j, k, l;

	ZArray<int> testArray1;
	ZArray<int> testArray2(50);

	TASSERT(testArray1 == testArray2, "== Operator returning invalid value on empty arrays!");

	i = 1;

	testArray2.Push(i);

	TASSERT(testArray1 != testArray2, "!= Operator returning invalid value on different arrays!");

	i = 5;

	TASSERT(testArray2[0] != 5, "ZArray improperly held reference to pushed value!");

	i = testArray2[0];

	TASSERT(i == 1, "[] operator returned invalid value!");

	testArray1 = testArray2;

	TASSERT(testArray1 == testArray2, "== Operator returned invalid value after = operation!");

	j = testArray1.Pop();

	TASSERT(j == 1, "ZArray::Pop operation returned invalid value!");

	TASSERT(testArray1 != testArray2, "!= Operator returned invalid value after Pop operation!");

	testArray1.Resize(2);

	testArray1[0] = 1;
	testArray1[1] = 5;

	ZArray<int> testArray3 = testArray1 + testArray2;

	testArray1 += testArray2;

	TASSERT(testArray1 == testArray3, "== operator returned invalid value after + and += operation!");

	testArray2.Clear();
	testArray1.Clear();

	TASSERT(testArray1 == testArray2, "== operator returned invalid value after Clear operations!");

	TASSERT(testArray1.Count(1) == 0, "ZArray::Count operation returned invalid count for the number 1 on testArray1!");
	TASSERT(testArray3.Count(1) == 2, "ZArray::Count operation returned invalid count for the number 1 on testArray3!");

	TASSERT(testArray3.Contains(5), "ZArray::Contains returned invalid value for the value 5 on testArray3! (false)");
	TASSERT(testArray3.Find(5) == 1, "ZArray::Find failed to find the value 5 at the correct location on testArray3!");
	TASSERT(testArray3.Find(5, 2) == -1, "ZArray::Find found the value 5 where it was not present in testArray3!");
	
	testArray3.Erase(1);

	TASSERT(testArray3.Contains(5) == false, "ZArray::Contains returned invalid value for 5 on testArray3! (true)");

	testArray3.Insert(1, 5);

	TASSERT(testArray3.Find(5) == 1, "ZArray::Find returned invalid position for 5 on testArray3!");

	k = testArray3.Pop();

	TASSERT(k == 1, "ZArray::Pop returned invalid value from testArray3!");
	TASSERT(testArray3.Size() == 2, "ZArray::Size returned invalid value on testArray3!");

	testArray3.Push(1);
	testArray3.Push(1);
	testArray3.Push(2);
	testArray3.Push(4);
	testArray3.Push(1);

	l = testArray3.Remove(5);

	TASSERT(l == 1, "ZArray::Returned invalid value from Remove on testArray3!");

	ZArray<int> testArray4 = testArray3.Slice(2, 4);

	TASSERT(testArray4[0] == 1, "ZArray::Slice returned invalid value in first pos!");
	TASSERT(testArray4[1] == 2, "ZArray::Slice returned invalid value in second pos!");

	testArray4.SwapElements(0, 1);

	TASSERT(testArray4[0] == 2, "ZArray::SwapElements failed to swap first pos!");
	TASSERT(testArray4[1] == 1, "ZArray::SwapElements failed to swap second pos!");

	testArray3.RemoveAll(1);

	TASSERT(testArray3.Size() == 2, "ZArray::RemoveAll failed to remove all instances of 1 from testArray3!");

	testArray3.Reserve(20);

	TASSERT(testArray3.Capacity() == 20, "ZArray::Reserve failed to properly size testArray3!");

	testArray3.Resize(30);

	TASSERT(testArray3.Capacity() == 30, "ZArray::Resize failed to properly allocate testArray3!");
	TASSERT(testArray3.Size() == 30, "ZArray::Resize failed to properly size testArray3!");

	testArray3.Swap(testArray4);

	TASSERT(testArray4.Capacity() == 30, "ZArray::Swap failed to swap capacity!");
	TASSERT(testArray4.Size() == 30, "ZArray::Swap failed to swap size!");
	TASSERT(testArray3[0] == 2, "ZArray::Swap failed to swap first value!");
	TASSERT(testArray3[1] == 1, "ZArray::Swap failed to swap second value!");

	for(i=0; i<(int)testArray4.Size(); i++)
		testArray4[i] = i;
	
	ZArray<int>::Iterator itr;

	for (itr = testArray4.Begin(), i = 0; itr != testArray4.End(); itr++, i++)
	{
		TASSERT(*itr == testArray4[i], "ZArray::Iterator points to invalid position!");
		TASSERT((int)itr == i, "ZArray::Iterator failed to cast to proper index!");
	}

	itr = itr - 5;

	TASSERT(*itr == testArray4[testArray4.Size() - 5], "ZArray::Iterator points to incorrect location after loop!");
	TASSERT(*itr-- == testArray4[testArray4.Size() - 5], "ZArray::Iterator-- failed to post-decrement!");
	TASSERT(*itr == testArray4[testArray4.Size() - 6], "ZArray::Iterator-- failed to decrement!");

	testArray4.Resize(10);

	testArray4[0] = 5;
	testArray4[1] = 3;
	testArray4[2] = 7;
	testArray4[3] = 6;
	testArray4[4] = 1;
	testArray4[5] = 0;
	testArray4[6] = 2;
	testArray4[7] = 4;
	testArray4[8] = 9;
	testArray4[9] = 8;

	testArray4.Sort();

	testArray4.Erase(5);

	testArray4.Erase(2, 4);

	testArray4.Erase(testArray4.Begin() + 3, testArray4.End());

	testArray4.Insert(1, testArray3);

	TASSERT(testArray4[1] == 2, "ZArray::Erase and Insert operators failed at pos 1!");
	TASSERT(testArray4[2] == 1, "ZArray::Erase and Insert operators failed at pos 2!");

	return ZTEST_SUCCESS;
}

static const char* testShift()
{
	ZArray<int> a1;

	//a1 = [1,2,3,4]
	a1.Push(1);
	a1.Push(2);
	a1.Push(3);
	a1.Push(4);

	//a1 = [0xFF, 1, 2, 3, 4]
	a1.Shift(0xFF);

	TASSERT(a1[0] == 0xFF, "Shift() did not set element [0] to 0xFF");
	TASSERT(a1.Size() == 5, "Shift() did not set the size correctly");

	for(size_t i=1; i<a1.Size(); i++)
	{
		//Elements [1] .. [Size()-1] should be unmodified.
		TASSERT(a1[i] == (int)i, "Shift() did not preserve remaining elements' values");
	}

	int test = a1.Unshift();

	TASSERT(test == 0xFF, "Unshift() returned the wrong value");
	TASSERT(a1.Size() == 4, "Unshift() did not correctly set the size");

	for(size_t i=0; i<a1.Size(); i++)
	{
		//Now elements [0..3] should be the values {1, 2, 3, 4} again.
		TASSERT(a1[i] == (int)(i+1), "Unshift() did not preserve remaining elements' values");
	}
	
	return ZTEST_SUCCESS;
}

/******************************************************************************/

template<typename T>
class TestArrayLToG : public ZComparator<T>
{
	 bool LessThan(const T& _a, const T& _b) const { return _a < _b; }
};

template<typename T>
class TestArrayGToL : public ZComparator<T>
{
	bool LessThan(const T& _a, const T& _b) const { return _a > _b; }
};

static const char* testSort()
{
	ZArray<int> unsorted, sorted;
	ZArray<int> refSorted, revSorted;

	//Set up an array of unsorted integers. 'refSorted' is the manually sorted array.
	// Sorted, it is    { 1, 2, 2, 2, 3, 4, 5, 5 }
	// Backwards, it is { 5, 5, 4, 3, 2, 2, 2, 1 }
	unsorted.Push(5); refSorted.Push(1); revSorted.Push(5);
	unsorted.Push(1); refSorted.Push(2); revSorted.Push(5);
	unsorted.Push(3); refSorted.Push(2); revSorted.Push(4);
	unsorted.Push(5); refSorted.Push(2); revSorted.Push(3);
	unsorted.Push(4); refSorted.Push(3); revSorted.Push(2);
	unsorted.Push(2); refSorted.Push(4); revSorted.Push(2);
	unsorted.Push(2); refSorted.Push(5); revSorted.Push(2);
	unsorted.Push(2); refSorted.Push(5); revSorted.Push(1);

	//Copy the array 
	sorted = unsorted;
	sorted.Sort();

	TASSERT(sorted == refSorted, "Sort(void) did not sort the array properly");

	TestArrayGToL<int> gtol;
	TestArrayLToG<int> ltog;

	sorted = unsorted;
	sorted.Sort(gtol);

	TASSERT(sorted == revSorted, "Sort(ZComparator) did not sort the array properly");

	sorted = unsorted;
	sorted.Sort(ltog);
	TASSERT(sorted == refSorted, "Sort(ZComparator) did not sort the array properly");

	return ZTEST_SUCCESS;
}
