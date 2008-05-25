#include "DefaultConsole.h"

#define UNUSED(x) ((void)x)

extern "C" void kmain( void* mbd, unsigned int magic );

DefaultConsole console;

void kmain( void* mbd, unsigned int magic )
{
	UNUSED(mbd);
	UNUSED(magic);

	console.putchar('A');
}
