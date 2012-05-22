/*
	Test-ZHashMap.cpp
	Author: James Russell (jcr2)

	Purpose: Unit Test the ZHashMap class.

	Changelog:
	12/18/2011 - Removed dependency on ZString (cre1)
	03/13/2011 - creation (jcr2)
*/

#include "ZUnitTest.h"

#include <ZSTL/ZHashMap.hpp>

static const char* testPutGet();
static const char* testContainsKey();
static const char* testContainsValue();
static const char* testEmpty();
static const char* testKeys();
static const char* testMappings();
static const char* testRemove();
static const char* testSize();
static const char* testTryGetPointer();
static const char* testValues();

//List of unit tests
ZUnitTest ZHashMapUnitTests[] =
{
	{
		"Test contains key",
		testContainsKey
	},
	{
		"Test contains value",
		testContainsValue
	},
	{
		"Test Put(), Get()",
		testPutGet
	},
	{
		"Test Empty",
		testEmpty
	},
	{
		"Test get keys",
		testKeys
	},
	{
		"Test get mappings",
		testMappings
	},
	{
		"Test remove",
		testRemove
	},
	{
		"Test size",
		testSize
	},
	{
		"Test try get pointer",
		testTryGetPointer
	},
	{
		"Test get values",
		testValues
	}
};

//Now declare the ZUnitTestBlock associated with this.
DECLARE_ZTESTBLOCK(ZHashMap);

/******************************************************************************/

static const char* testPutGet()
{
	ZHashMap<const char*,int> map;
	int v1, v2;

	map.Put("wakka", 24);
	map.Put("ooga", 42);
	TASSERT(map.TryGet("wakka", v1), "TryGet()/Put() failed to match correctly");
	TASSERT(map.TryGet("ooga", v2), "TryGet()/Put() failed to match correctly");
	TASSERT(v1 == 24 && v2 == 42, "TryGet() fetched wrong values");
	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testContainsKey()
{
	ZHashMap<const char*,int> map;
	TASSERT(map.ContainsKey("wakka") == false, "ContainsKey() returned true when given empty set");
	map.Put("oonga",2);
	TASSERT(map.ContainsKey("wakka") == false, "ContainsKey() returned true when given set missing key");
	map.Put("wakka",1);
	TASSERT(map.ContainsKey("wakka"), "ContainsKey() returned false when given set containing key");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testContainsValue()
{
	ZHashMap<const char*,int> map;
	map.Put("wakka",1);
	map.Put("wakkawakka",2);
	TASSERT(map.ContainsValue(3) == false, "ContainsValue() claims to contain a value it doesn't have");
	TASSERT(map.ContainsValue(2) == true, "ContainsValue() claims to contain a value it does have");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testEmpty()
{
	ZHashMap<const char*,int> map;

	TASSERT(map.Empty() == true, "Empty() reports false when map has no elements");

	map.Put("wakka",1);
	map.Put("wakkaa",2);
	map.Put("wakkaaa",3);
	TASSERT(map.Empty() != true, "Empty() reports true when map has elements");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testKeys()
{
	ZHashMap<const char*,int> map;
	ZList<const char*> ret;
	int found = 0;
	const char* test[3] = {"wakka","wakkawakka","wakkawakkawakka"};
	map.Put("wakka",11);
	map.Put("wakkawakka",22);
	map.Put("wakkawakkawakka",33);
	ret = map.Keys();
	TASSERT(ret.Size() == 3, "Keys() did not return accurate sized list of keys in hashmap.\n");
	ZList<const char*>::Iterator itr;
	for (itr = ret.Begin(); itr != ret.End(); ++itr)
	{
		const char* tofind = *itr;
		for (int i = 0; i < 3; i++)
		{
			if (strcmp(tofind,test[i]) == 0)
				found++;
		}
	}
	TASSERT(found == 3, "Keys() did not return accurate list of keys for values in hashmap.\n");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testMappings()
{
	ZHashMap<const char*,int> map;
	ZList< ZPair<const char*,int> > ret;
	map.Put("wakka",11);
	ret = map.Mappings();
	TASSERT(ret.Size() == 1, "Mappings() returned list of incorrect size for key/value pairs.\n");
	TASSERT( strcmp(ret.Front().First,"wakka")==0 && ret.Front().Second == 11, "Mappings() returned incorrect key/value pair.\n");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testRemove()
{
	ZHashMap<const char*,int> map;

	map.Put("wakka",1);
	TASSERT(map.ContainsValue(1) && map.ContainsKey("wakka"), "Map has somehow lost our value.\n");
	map.Remove("wakka");
	TASSERT(map.ContainsValue(1) == false && map.ContainsKey("wakka") == false, "Map claims key despite removal.\n");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testSize()
{
	ZHashMap<const char*,int> map;

	TASSERT(map.Size() == 0, "Size() returned nonzero on empty hashmap.\n");

	map.Put("wakka",1);
	map.Put("wakkaa",2);
	map.Put("wakkaaa",3);
	TASSERT(map.Size() == 3, "Size() returned wrong length on map with three entries.\n");

	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testTryGetPointer()
{
	ZHashMap<const char*,int> map;
	TASSERT(map.TryGetPtr("wakka") == NULL, "TryGetPtr() claimed success on missing key.\n");
	map.Put("wakka",1337);
	TASSERT(*(map.TryGetPtr("wakka")) == 1337, "TryGetPtr() gave pointer to bad value.\n");
	return ZTEST_SUCCESS;
}

/******************************************************************************/

static const char* testValues()
{
	ZHashMap<const char*,int> map;
	ZList<int> ret;
	int found = 0;
	int test[3] = {11,22,33};
	map.Put("wakka",11);
	map.Put("wakkawakka",22);
	map.Put("wakkawakkawakka",33);
	ret = map.Values();
	TASSERT(ret.Size() == 3, "Values() did not return accurate sized list of values in hashmap.\n");
	ZList<int>::Iterator itr;
	for (itr = ret.Begin(); itr != ret.End(); ++itr)
	{
		int tofind = *itr;
		for (int i = 0; i < 3; i++)
		{
			if (tofind == test[i])
				found++;
		}
	}
	TASSERT(found == 3, "Values() did not return accurate list of values for values in hashmap.\n");

	return ZTEST_SUCCESS;
}
