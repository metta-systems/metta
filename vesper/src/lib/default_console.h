//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace metta {
namespace kernel {

// FIXME these defines poison global identifier space
#define kconsole default_console::self()
#define endl default_console::eol

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

class default_console
{
public:
    static const char eol;

    static default_console& self();

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
    default_console();

    volatile unsigned char* videoram;
    volatile unsigned int*  cursor;
    unsigned char           attr;
};

// Define stream io on console.
inline default_console& operator << (default_console& con, Color data)
{
    con.set_color(data);
    return con;
}

inline default_console& operator << (default_console& con, const char* data)
{
    con.print(data);
    return con;
}

inline default_console& operator << (default_console& con, int data)
{
    con.print_int(data);
    return con;
}

inline default_console& operator << (default_console& con, unsigned int data)
{
    con.print_hex(data);
    return con;
}

inline default_console& operator << (default_console& con, char data)
{
    con.print_char(data);
    return con;
}

inline default_console& operator << (default_console& con, unsigned char data)
{
    con.print_byte(data);
    return con;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
