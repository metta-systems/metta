//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "debugmacros.h" // define D() and V()
#include "console.h"

#define kconsole default_console_t::self()
#define null_console null_console_t::self()

//TODO:
//add kdebug as kconsole or null_console depending on boot flags

// // kc_debug() << "Hello world"; will print only in debug mode
// inline default_console_t& kc_debug()
// {
//     default_console_t& ref = default_console_t::self();
//     ref.set_log_level(default_console_t::debug);
//     return ref;
// }

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
    virtual void print_unprintable(char ch);
    virtual void print_byte(unsigned char n);
    virtual void print_hex(uint32_t n);
    virtual void print_hex2(uint16_t n);
    virtual void print_hex8(uint64_t n);
    virtual void print_str(const char *s);

    virtual void wait_ack();

    virtual void debug_log(const char *str, ...);

private:
    default_console_t();

    void blit(); // Blit rambuf to videoram
    void print_byte_internal(unsigned char n);

    uint8_t rambuf[160*25];
    unsigned char* videoram{(unsigned char*)0xb8000};
    unsigned int            cursor;
    unsigned char           attr;
};


class null_console_t : public console_t
{
public:
    static null_console_t& self();

    virtual void set_color(Color col);
    virtual void set_background(Color col);
    virtual void set_attr(Color fore, Color back);// e.g. GREEN on BLACK

    virtual void clear();
    virtual void locate(int row, int col);
    virtual void scroll_up();
    virtual void newline();

    virtual void print_int(int n);
    virtual void print_char(char ch);
    virtual void print_unprintable(char ch);
    virtual void print_byte(unsigned char n);
    virtual void print_hex(uint32_t n);
    virtual void print_hex2(uint16_t n);
    virtual void print_hex8(uint64_t n);
    virtual void print_str(const char *s);

    virtual void wait_ack();

    virtual void debug_log(const char *str, ...);

private:
    null_console_t();
};
