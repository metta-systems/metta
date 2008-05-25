#include "DefaultConsole.h"

#define EOL 10

// Screen dimensions (for default 80x25 console)
#define LINE_PITCH 160       // line width in bytes
#define LINE_COUNT 25

DefaultConsole::DefaultConsole()
{
   videoram = (unsigned char *) 0xb8000;
   cursor = (unsigned int *) 0xb903c;
   clear();
}

void DefaultConsole::clear()
{
	for(unsigned int i = 0; i < LINE_PITCH*LINE_COUNT; i++)
		videoram[i] = 0;
	*cursor = 0;
	attr = 0x07;
}

void DefaultConsole::locate(int row, int col)
{
	*cursor = (row * LINE_PITCH) + (col * 2);
}

void DefaultConsole::scroll_up()
{
	print("<TODO>");
}

void DefaultConsole::newline()
{
	print_char(EOL);
}

void DefaultConsole::print_int(int n)
{
	if (n == 0)
	{
		print_char('0');
		return;
	}

	if (n < 0)
	{
		print_char('-');
		n = -n;
	}

	unsigned int div = 1000000000;

	while (n / div == 0)
		div /= 10;

	while (div > 0)
	{
		char c = '0' + (n / div);
		print_char(c);
		n = n % div;
		div /= 10;
	}
}

void DefaultConsole::print_byte(unsigned char n)
{
	const char hexdigits[17] = "0123456789ABCDEF"; // 16+1 for terminating null
	char c = hexdigits[(n >> 4) & 0xF];
	print_char(c);
	c = hexdigits[n & 0xF];
	print_char(c);
}

void DefaultConsole::print_hex(unsigned int n)
{
	for(int i = 4; i > 0; i--)
		print_byte((n >> (i-1)*8) & 0xFF);
}

void DefaultConsole::print_char(char ch)
{
	if (ch == EOL)
	{
		// move cursor to new line and align on line start
		// FIXME: will be optimized out by gcc?
		*cursor = (((*cursor + LINE_PITCH) / LINE_PITCH) * LINE_PITCH);
	}
	else
	{
		videoram[*cursor] = ch; /* character */
		videoram[*cursor+1] = attr; /* foreground, background colors. */
		(*cursor)+=2;
	}

	if(*cursor >= LINE_PITCH*LINE_COUNT)
	{
		scroll_up();
		*cursor = LINE_PITCH * (LINE_COUNT - 1);
	}
}

void DefaultConsole::print(const char *str) //FIXME: use PStrings?
{
	while (*str)
		print_char(*str++);
}

void DefaultConsole::debug_showregs() // FIXME: gcc will trash most of the registers anyway
{
}

void DefaultConsole::debug_showstack()
{
}

// Dump at most 256 bytes of memory.
void DefaultConsole::debug_showmem(void *addr, unsigned int size)
{
	char *ptr = (char *)addr;
	int run;
	newline();
	if (size > 256)
		size = 256;
	while (size > 0)
	{
		print_hex((unsigned int)ptr);
		print("  ");
		run = size < 16 ? size : 16;
		for(int i = 0; i < run; i++)
		{
			print_byte(*(ptr+i));
			print_char(' ');
			if (i == 7)
				print_char(' ');
		}
		if (run < 16)// pad
		{
			if(run < 8)
				print_char(' ');
			for(int i = 0; i < 16-run; i++)
				print("   ");
		}
		print_char(' ');
		for(int i = 0; i < run; i++)
		{
			char c = *(ptr+i);
			if (c == EOL)
				c = ' ';
			print_char(c);
		}
		newline();
		ptr += run;
		size -= run;
	}
}

void DefaultConsole::wait_ack()
{
}
