//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "string.h"
#include "common.h"
#include "macros.h"
#include "memutils.h"

using metta::common::memutils;
using metta::common::string;

namespace metta {
namespace kernel {

const char default_console::eol = 10;

// Screen dimensions (for default 80x25 console)
#define LINE_PITCH 160       // line width in bytes
#define LINE_COUNT 25

default_console& default_console::self()
{
    static default_console console;
    return console;
}

default_console::default_console()
{
    videoram = (volatile unsigned char *) 0xb8000;
    cursor = (volatile unsigned int *) 0xb903c;
    clear();
}

void default_console::clear()
{
    memutils::fill_memory((void*)videoram, 0, LINE_PITCH*LINE_COUNT);
    locate(0,0);
    attr = 0x07;
}

void default_console::set_color(Color col)
{
    attr = (attr & 0xF0) | (col & 0x0F);
}

void default_console::set_background(Color col)
{
    attr = (attr & 0x0F) | ((col & 0x0F) << 8);
}

void default_console::set_attr(Color fore, Color back)
{
    set_color(fore);
    set_background(back);
}

void default_console::locate(int row, int col)
{
    *cursor = (row * LINE_PITCH) + (col * 2);
    // Set VGA hardware cursor
    outb(0x3d4, 14); // Tell the VGA board we are setting the high cursor byte.
    outb(0x3d5, (*cursor) >> 8); // Send the high cursor byte.
    outb(0x3d4, 15); // Tell the VGA board we are setting the low cursor byte.
    outb(0x3d5, (*cursor) & 0xff);      // Send the low cursor byte.
}

void default_console::scroll_up()
{
    memutils::move_memory((void*)videoram, (void*)(videoram+LINE_PITCH), LINE_PITCH*(LINE_COUNT-1));
    memutils::fill_memory((void*)(videoram+LINE_PITCH*(LINE_COUNT-1)), 0, LINE_PITCH);
}

void default_console::newline()
{
    print_char(eol);
}

/** Print decimal integer */
void default_console::print_int(int n)
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

/** Print hexadecimal byte */
void default_console::print_byte(unsigned char n)
{
    const char hexdigits[17] = "0123456789abcdef"; // 16+1 for terminating null
    char c = hexdigits[(n >> 4) & 0xF];
    print_char(c);
    c = hexdigits[n & 0xF];
    print_char(c);
}

/** Print hexadecimal integer */
void default_console::print_hex(unsigned int n)
{
    print_str("0x");
    for(int i = 4; i > 0; i--)
        print_byte((n >> (i-1)*8) & 0xFF);
}

/** Print 64 bit hex integer */
void default_console::print_hex8(unsigned long long n)
{
    print_str("0x");
    for(int i = 8; i > 0; i--)
        print_byte((n >> (i-1)*8) & 0xFF);
}

/** Print a single character */
void default_console::print_char(char ch)
{
    if (ch == eol)
    {
        // move cursor to new line and align on line start
        // FIXME: will be optimized out by gcc?
        *cursor = (((*cursor + LINE_PITCH) / LINE_PITCH) * LINE_PITCH);
    }
    else
    {
        videoram[*cursor] = ch; /* character */
        videoram[*cursor+1] = attr; /* foreground, background colors. */
        (*cursor) += 2;
    }

    if(*cursor >= LINE_PITCH*LINE_COUNT)
    {
        scroll_up();
        *cursor = LINE_PITCH * (LINE_COUNT - 1);
    }
    BochsConsolePrintChar(ch);
}

void default_console::wait_ack()
{
    uint8_t keycode;
    uint8_t irqmask = inb(0x21);
    outb(0x21, irqmask | 0x02); // mask irq1 - keyboard

    /* wait keypress */
    do {
        while ((inb(0x64) & 0x01) == 0) {}
        keycode = inb(0x60);
    } while (keycode != 0x1c); // "make code" == enter

    /* wait keyrelease */
    do {
        while ((inb(0x64) & 0x01) == 0) {}
        keycode = inb(0x60);
    } while (keycode != 0x9c); // "break code" == enter

    if (!(irqmask & 0x02)) // if irq1 was unmasked previously,
        outb(0x21, inb(0x21) & 0xfd); // unmask it now without changing other flags
}

/** Print string without formatting */
void default_console::print_str(const char *str)
{
    char *b = (char *)str;
    while (*b)
        print_char(*b++);
}

void default_console::debug_cp(const char *str)
{
    print_str("\n[DBGCHKPT] ");
    print_str(str);
    print_str("\nPress ENTER to continue...\n");
    wait_ack();
}

void default_console::debug_log(const char *str, ...)
{
    unsigned char old_attr = attr;
    set_attr(WHITE, BLACK);
    print_str(str);
    print_char(eol);
    attr = old_attr;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
