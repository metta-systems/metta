//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "console.h"

#define kconsole default_console_t::self()
//TODO:
//add kdebug as kconsole or null_console depending on boot flags

class default_console_t : public console_t
{
public:
    static default_console_t& self();

    virtual void set_color(Color col);
    virtual void set_background(Color col);
    virtual void set_attr(Color fore, Color back);// e.g. GREEN on BLACK

    virtual void clear();
    virtual void locate(int row, int col);
    virtual void scroll_up();
    virtual void newline();

    virtual void print_int(int n);
    virtual void print_char(char ch);
    virtual void print_byte(unsigned char n);
    virtual void print_hex(uint32_t n);
    virtual void print_hex8(uint64_t n);
    virtual void print_str(const char *s);

    virtual void wait_ack();

    virtual void debug_log(const char *str, ...);

private:
    default_console_t();

    volatile unsigned char* videoram;
    volatile unsigned int*  cursor;
    unsigned char           attr;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
