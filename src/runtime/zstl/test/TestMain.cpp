/*
	TestMain.cpp
	Author: Patrick Baggett

	Purpose :	Contains driver for unit tests
*/

#include "ZUnitTest.h"

#include <stdlib.h>
#include <stdio.h>

extern ZUnitTestBlock ZAtomicTests;
extern ZUnitTestBlock ZListTests;
extern ZUnitTestBlock ZArrayTests;
extern ZUnitTestBlock ZHashMapTests;
extern ZUnitTestBlock ZRingBufferTests;
extern ZUnitTestBlock ZStringTests;

int main(int argc, char **argv)
{
	ZUnitTestGlobals::ARGC = argc;
	ZUnitTestGlobals::ARGV = argv;

	for(int i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "-n"))
			ZUnitTestGlobals::NonInteractive = 1;
		else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			printf(
			"-h : help (this screen)\n"
			"-n : non-interactive (don't wait for keys)\n");
			exit(0);	
		}
		else
		{
			printf("ERROR: Option \"%s\" not recognized", argv[1]);
			exit(1);
		}
	}


	bool success = true;

	/***********************/

	/* Add Unit Tests Here */
	
	success &= RunUnitTests(&ZListTests);
	success &= RunUnitTests(&ZArrayTests);
	success &= RunUnitTests(&ZHashMapTests);
	success &= RunUnitTests(&ZRingBufferTests);
	success &= RunUnitTests(&ZStringTests);

	/***********************/

	if (success)
		printf("**************\nALL TESTS SUCCESSFUL!\n**************\n\n");
	else
		printf("**************\nWARNING! SOME TESTS FAILED!\n**************\n\n");

	if(!ZUnitTestGlobals::NonInteractive)
	{
		printf("\nPress ENTER to continue...\n");
		fflush(stdout);
		fgetc(stdin);
	}


}
