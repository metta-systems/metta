//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "panic.h"

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
    void set_attr(Color fore, Color back);// e.g. GREEN on BLACK

    void clear();
    void locate(int row, int col);
    void scroll_up();
    void newline();

    void print_int(int n);
    void print_char(char ch);
    void print_byte(unsigned char n);
    void print_hex(unsigned int n);
    void print_hex8(unsigned long long n);
    void print_str(const char *s);

    // Wrappers for template version
    inline void print(int n) { print_int(n); }
    inline void print(char ch) { print_char(ch); }
    inline void print(unsigned char n) { print_byte(n); }
    inline void print(unsigned int n) { print_hex(n); }
    inline void print(void *p) { print((unsigned int)p); }
    inline void print(unsigned long long n) { print_hex8(n); }
    inline void print(const char* str) { print_str(str); }
    template<typename T, typename... Args>
    void print(const char* str, T value, Args... args);

    void wait_ack();

    void debug_log(const char *str, ...);
    void checkpoint(const char *str); // FIXME: move to debugger_t?

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
    con.print_str(data);
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

template<typename T, typename... Args>
void default_console::print(const char* str, T value, Args... args)
{
    while (*str)
    {
        if (*str == '%' && *(++str) != '%')
        {
            print(value);
            print(*str ? ++str : str, args...); // call even when *s == 0 to detect extra arguments
            return;
        }
        print(*str++);
    }
    PANIC("extra arguments provided to print");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
