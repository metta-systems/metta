#ifndef __INCLUDED_DEFAULTCONSOLE_H
#define __INCLUDED_DEFAULTCONSOLE_H

#define kconsole DefaultConsole::self()

class DefaultConsole
{
public:
	static DefaultConsole &self();

	void clear();
	void locate(int row, int col);
	void scroll_up();
	void newline();
	void print_int(int n);
	void print_char(char ch);
	void print_byte(unsigned char n);
	void print_hex(unsigned int n);
	void print(const char *str); //FIXME: use PStrings?

	void debug_showregs();
	void debug_showstack();
	void debug_showmem(void *addr, unsigned int size);
	void wait_ack();

private:
	DefaultConsole();
	unsigned char *videoram;
	unsigned int *cursor;
	unsigned char attr;
};

#endif
