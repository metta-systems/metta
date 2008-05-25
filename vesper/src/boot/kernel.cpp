#include "DefaultConsole.h"

#define UNUSED(x) ((void)x)

extern "C" void kmain( void* mbd, unsigned int magic );

DefaultConsole console;

void kmain( void* mbd, unsigned int magic )
{
	UNUSED(mbd);

	console.locate(7, 20);
	console.print("Hello,\n");
	console.newline();
	console.print_byte(0xAB);
	console.print_char('_');
	console.print("World!\n\n");
	console.print_hex(magic);
	console.locate(11, 22);
	console.print_int(-21954321);
	console.debug_showmem((void*)0x7c00, 135);
}
