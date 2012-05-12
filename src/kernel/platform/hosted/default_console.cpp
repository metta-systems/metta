//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include "default_console.h"
#include "cpu.h"
#include "memutils.h"
#include "debugger.h"

using namespace std;

// Screen dimensions (for default 80x25 console)
#define LINE_PITCH 160       // line width in bytes
#define LINE_COUNT 25
#define TAB_PITCH 16

default_console_t& default_console_t::self()
{
    static default_console_t console;
    return console;
}

default_console_t::default_console_t()
    : console_t()
{
    clear();
}

void default_console_t::clear()
{
}

void default_console_t::set_color(Color col)
{
}

void default_console_t::set_background(Color col)
{
}

void default_console_t::set_attr(Color fore, Color back)
{
    set_color(fore);
    set_background(back);
}

void default_console_t::locate(int row, int col)
{
}

void default_console_t::scroll_up()
{
}

void default_console_t::newline()
{
    print_char(eol);
}

/*! Print decimal integer */
void default_console_t::print_int(int n)
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

/*! Print hexadecimal byte */
void default_console_t::print_byte(unsigned char n)
{
    const char hexdigits[17] = "0123456789abcdef"; // 16+1 for terminating null
    char c = hexdigits[(n >> 4) & 0xF];
    print_char(c);
    c = hexdigits[n & 0xF];
    print_char(c);
}

/*! Print hexadecimal integer */
void default_console_t::print_hex(uint32_t n)
{
    print_str("0x");
    for(int i = 4; i > 0; i--)
        print_byte((n >> (i-1)*8) & 0xFF);
}

/*! Print 64 bit hex integer */
void default_console_t::print_hex8(unsigned long long n)
{
    print_str("0x");
    for(int i = 8; i > 0; i--)
        print_byte((n >> (i-1)*8) & 0xFF);
}

/*! Print a single character */
void default_console_t::print_char(char ch)
{
    switch (ch)
    {
        case '\r':
            break;
        case '\n':
            cout << endl;
            break;
        case '\t':
            cout << '\t';
            break;
        default:
            if (ch < ' ')
                ch = '.'; // replace non-printable chars
            cout << ch;
    }
}

void default_console_t::print_unprintable(char ch)
{
    if (ch == '\n' || ch == '\r' || ch == '\t')
        ch = '.';
    print_char(ch);
}

/*! Wait for Enter key press and release on keyboard */
void default_console_t::wait_ack()
{
    char buf[32];
    cin >> buf;
}

/*! Print string without formatting */
void default_console_t::print_str(const char *str)
{
    char *b = (char *)str;
    while (*b)
        print_char(*b++);
}

void default_console_t::debug_log(const char *str, ...)
{
    unsigned char old_attr = attr;
    set_attr(WHITE, BLACK);
    print_str(str);
    print_char(eol);
    attr = old_attr;
}
