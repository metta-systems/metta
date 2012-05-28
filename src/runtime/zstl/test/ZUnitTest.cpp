/*
	ZUnitTest.cpp
	Author: Patrick Baggett

	Purpose :	Code for automated ZUnitTest running

	Changelog :
	12/18/2011 - Removed dependency on ZString (cre1)
	02/19/2011 - Added 'wait' after failed test (jcr2)
	12/15/2010 - Created (ptb1)
*/

#include "ZUnitTest.h"
#include <stdio.h>

int ZUnitTestGlobals::ARGC = 0;
char** ZUnitTestGlobals::ARGV = NULL;
int ZUnitTestGlobals::NonInteractive = 0;

bool RunUnitTests(ZUnitTestBlock* block)
{

	bool passedAllTests = true; //Passed all tests?

	printf("*** BEGINNING TEST BLOCK FOR \"%s\" ***\n\n", block->blockName);
	for(int i=0; i<block->nrTests; i++)
	{
		const char* result;
		printf("Test %2d/%2d - %s: ", i+1, block->nrTests, block->tests[i].testName);
		fflush(stdout);
		result = block->tests[i].testFunc();
		if( strcmp(result,ZTEST_SUCCESS) != 0)
		{
			printf("FAIL.\n  -> Error message: %s\n", result);
			passedAllTests = false;
		}
		else
			printf("PASS.\n");
	}
	printf("\n*** OVERALL RESULT FOR \"%s\": %s ***\n\n", block->blockName, passedAllTests ? "PASS" : "FAIL");

	if (!passedAllTests) 
	{
		if(!ZUnitTestGlobals::NonInteractive)
		{
			printf("Press ENTER to keep running tests...\n");
			fflush(stdout);
			fgetc(stdin);
		}
	}

	return passedAllTests;
}
