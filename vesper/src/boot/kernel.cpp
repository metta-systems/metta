#include "DefaultConsole.h"

extern "C" void kmain( void* mbd, unsigned int magic );

void kmain( void* mbd, unsigned int magic )
{
	kconsole.locate(7, 20);
	kconsole.print("Hello,\n");
	kconsole.newline();
	kconsole.print_byte(0xAB);
	kconsole.print_char('_');
	kconsole.print("World!\n\n");
	kconsole.print_hex(magic);
	kconsole.locate(11, 22);
	kconsole.print_int(-21954321);
	kconsole.scroll_up();
	kconsole.set_color(YELLOW);
	kconsole.debug_showmem(mbd, 135);
}
