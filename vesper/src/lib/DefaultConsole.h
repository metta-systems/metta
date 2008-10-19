//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Macros.h"

#define kconsole DefaultConsole::self()
#define endl DefaultConsole::EOL

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
	static const char EOL;

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
	void print(const char *str, ...);

	void wait_ack();

	void debug_log(const char *str, ...);

private:
	DefaultConsole();
	unsigned char* videoram;
	unsigned int*  cursor;
	unsigned char  attr;
};

// Define stream io on console.
INLINE DefaultConsole& operator << (DefaultConsole& con, Color data)
{
	con.set_color(data);
	return con;
}

INLINE DefaultConsole& operator << (DefaultConsole& con, const char* data)
{
	con.print(data);
	return con;
}

INLINE DefaultConsole& operator << (DefaultConsole& con, int data)
{
	con.print_int(data);
	return con;
}

INLINE DefaultConsole& operator << (DefaultConsole& con, unsigned int data)
{
	con.print_hex(data);
	return con;
}

INLINE DefaultConsole& operator << (DefaultConsole& con, char data)
{
	con.print_char(data);
	return con;
}

INLINE DefaultConsole& operator << (DefaultConsole& con, unsigned char data)
{
	con.print_byte(data);
	return con;
}
