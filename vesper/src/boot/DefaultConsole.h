#ifndef __INCLUDED_DEFAULTCONSOLE_H
#define __INCLUDED_DEFAULTCONSOLE_H

#define kconsole DefaultConsole::self()

enum Color {
	BLACK = 0,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGRAY,
	DARKGRAY=8,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE
};

class DefaultConsole
{
public:
	static DefaultConsole &self();

	void set_color(Color col);
	void set_background(Color col);
	void set_attr(Color fore, Color back);// GREEN on BLACK

	void clear();
	void locate(int row, int col);
	void scroll_up();
	void newline();
	void print_int(int n);
	void print_char(char ch);
	void print_byte(unsigned char n);
	void print_hex(unsigned int n);
	void print(const char *str); //FIXME: use PStrings?

	void debug_showmem(void *addr, unsigned int size);
	void wait_ack();
	// unfinished:
	void debug_showregs();
	void debug_showstack();

private:
	DefaultConsole();
	unsigned char *videoram;
	unsigned int *cursor;
	unsigned char attr;
};

#endif
