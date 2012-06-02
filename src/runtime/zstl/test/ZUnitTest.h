/*
	ZUnitTest.h
	Author: Patrick Baggett

	Purpose :	Header for ZUnitTest type and associated functions
				Ignore the ugly C preprocessor hacks. They only make repetitive things
				less repetitive.

	Changelog :
	12/18/2011 - Removed dependency on ZString (cre1)
	12/15/2010 - Created (ptb1)
*/

#pragma once

#ifndef _ZUNITTEST_H
#define _ZUNITTEST_H

#include <ZSTL/ZString.hpp>

namespace ZUnitTestGlobals
{
	extern int ARGC;
	extern char **ARGV;
	extern int NonInteractive;
}

typedef struct ZUnitTest
{
	const char* testName;
	const char* (*testFunc)();
} ZUnitTest;

typedef struct ZUnitTestBlock
{
	ZUnitTest* tests;
	const char* blockName;
	int nrTests;
} ZUnitTestBlock;

#define ZTEST_SUCCESS ""

// Declares a ZUnitTestBlock given a data type name. Ensures ZUnitTestBlocks have consistent names for use in TestMain.cpp
#define DECLARE_ZTESTBLOCK(Type) ZUnitTestBlock Type##Tests = { Type##UnitTests, #Type, sizeof(Type##UnitTests) / sizeof(ZUnitTest) };

// Returns a string if the test condition fails
#define TASSERT(condition, msg)	if (!(condition)) { return msg; }

//Runs a unit test block, returns true if and only if all tests are successful
bool RunUnitTests(ZUnitTestBlock* block);

#endif
