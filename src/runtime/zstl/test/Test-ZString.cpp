/*
	Test-ZString.cpp
	Author: 	James Russell (jcr2)

	Purpose: 	Unit tests for ZString.

	Changelog:
	12/18/2011 - Removed dependency on ZString (cre1)
	2011/03/09 - creation (jcr2)
*/

#include "ZUnitTest.h"

static const char* testOperators();
static const char* testFunctions();
static const char* testTokenizers();

//List of unit tests
ZUnitTest ZStringUnitTests[] =
{
	{
		"ZString operators",
			testOperators
	},
	{
		"ZString functions",
			testFunctions,
	},
	{
		"ZString tokenizers",
			testTokenizers,
	}
};

DECLARE_ZTESTBLOCK(ZString);

/******************************************************************************/

static const char* testOperators()
{
	ZString testString1;

	ZString testString2("testString2");

	ZString testString3 = ZString::BuildNumeric(5);

	ZString testString4 = ZString::BuildNumeric(5.0);

	TASSERT(testString2[0] == 't', "[] operator returns incorrect character for pos 0!");

	TASSERT(testString2[1] == 'e', "[] operator returns incorrect character for pos 1!");

	TASSERT(testString2[10] == '2', "[] operator returns incorrect character for pos 10!");

	TASSERT(testString2[-1] == '2', "[] operator returns incorrect character for pos -1!");

	TASSERT(testString2 == "testString2", "== operator failed against const char*!");

	ZString testString5 = testString2 + testString3;

	TASSERT(testString5 == "testString25", "== operator failed against const char* after + and = operations!");

	TASSERT(testString5 != "blahblahblah", "!= operator failed against const char*!");

	testString5.Clear();

	TASSERT(testString5.Empty(), "ZString::Empty function failed after Clear!");

	TASSERT(testString5 == "", "== operator failed after Clear!");

	return ZTEST_SUCCESS;
}

static const char* testFunctions()
{
	ZString testString2("testString2");

	ZString testString4 = ZString::BuildNumeric(5.0);

	TASSERT(testString4 == "5.000000", "ZString failed to parse double correctly!");

	testString4.Erase(0, 1);

	TASSERT(testString4 == ".000000", "ZString failed to Erase first character!");

	testString4.Erase(4, testString4.Length());

	TASSERT(testString4 == ".000", "ZString failed to Erase characters 4-End!");

	TASSERT(testString4.Find(".") == 0, "ZString::Find failed to find '.' substring!");

	TASSERT(testString4.Find(".0") == 0, "ZString::Find failed to find '.0' substring!");

	TASSERT(testString4.Find(".00") == 0, "ZString::Find failed to find '.00' substring!");

	TASSERT(testString4.Find(".000") == 0, "ZString::Find failed to find '.000' substring!");

	TASSERT(testString4.Find("a") == -1, "ZString::Find found 'a' when there wasn't one!");

	TASSERT(testString4.Find("0000") == -1, "ZString::Find found '0000' when there wasn't one!");

	TASSERT(testString2.Find("st") == 2, "ZString::Find failed to find 'st' substring!");

	TASSERT(testString2.Find("st", 4) == -1, "ZString::Find found 'st' substring after index 4 when it doesn't exist!");

	TASSERT(testString2.ToLower().Find("st", 4) == 4, "ZString::Find failed to find 'st' substring after index 4 after ToLower!");

	testString4.Insert(0, "505");

	TASSERT(testString4 == "505.000", "== failed after call to Insert!");

	testString4.Insert(3, ZString::BuildNumeric(0));

	TASSERT(testString4 == "5050.000", "== failed after call to insert with numeric conversion!");

	testString4.Insert(8, ZString::BuildNumeric(5));

	TASSERT(testString4 == "5050.0005", "== failed after call to insert with numeric conversion at end!");

	testString4.Replace(1, 8, ".");

	TASSERT(testString4 == "5.5", "== failed after call to Replace!");

	return ZTEST_SUCCESS;
}

static const char* testTokenizers()
{
	ZString testString4 = ZString::BuildNumeric(5.0);

	testString4.Erase(0, 1);

	TASSERT(testString4.FirstOf("0") == 1, "ZString::FirstOf failed to find first '0' delims!");

	TASSERT(testString4.FirstOf(".") == 0, "ZString::FirstOf failed to find first '.' delims!");

	TASSERT(testString4.FirstOf(".0") == 0, "ZString::FirstOf failed to find first '.0' delims!");

	TASSERT(testString4.FirstOf("0.") == 0, "ZString::FirstOf failed to find first '0.' delims!");

	TASSERT(testString4.FirstNotOf(".") == 1, "ZString::FirstNotOf failed to find first not of '.' delims!");

	TASSERT(testString4.FirstNotOf(".0") == -1, "ZString::FirstNotOf found first not of '.0' delims when it doesn't exist!");

	testString4 = "5.5";

	ZString testString6 = testString4.Slice(1, 2);

	TASSERT(testString6 == ".", "== failed after Slice operation!");

	ZString testString7("  a   b c  ");

	TASSERT(testString7.Strip() == "abc", "== failed after Strip operation!");

	TASSERT(testString7.Strip('a') == "bc", "== failed after Strip operation with explicit delims!");

	ZString testString8("abc,def,ghi");

	TASSERT(testString8.Tokenize(",") == "abc", "ZString::Tokenize failed to return first token!");

	TASSERT(testString8.Tokenize(",") == "def", "ZString::Tokenize failed to return second token!");

	TASSERT(testString8.Tokenize(",") == "ghi", "ZString::Tokenize failed to return third token!");

	ZString testString9("abc,def:ghi:");

	TASSERT(testString9.Tokenize(",:") == "abc", "ZString::Tokenize failed to return first token with multiple delims!");

	TASSERT(testString9.Tokenize(",:") == "def", "ZString::Tokenize failed to return second token with multiple delims!");

	TASSERT(testString9.Tokenize(",:") == "ghi", "ZString::Tokenize failed to return third token with multiple delims!");

	TASSERT(testString9.Tokenize(",:") == "", "ZString::Tokenize failed to return empty string with no tokens remaining!" );

	ZString testString10(":abc,def:ghi:");

	ZArray<ZString> splitString = testString10.Split(",:");

	TASSERT(splitString[0] == "abc", "ZString::Split operation failed to get correct first substring!");

	TASSERT(splitString[1] == "def", "ZString::Split operation failed to get correct second substring!");

	TASSERT(splitString[2] == "ghi", "ZString::Split operation failed to get correct third substring!");

	return ZTEST_SUCCESS;
}



