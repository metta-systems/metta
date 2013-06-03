//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "cpu.h"
#include "memutils.h"
#include "debugger.h"

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
    memutils::clear_memory((void*)rambuf, sizeof(rambuf));
    blit();
    locate(0,0);
    attr = 0x07;
}

void default_console_t::blit()
{
    memutils::copy_memory((void*)videoram, (void*)rambuf, sizeof(rambuf));
}

void default_console_t::set_color(Color col)
{
    attr = (attr & 0xF0) | (col & 0x0F);
}

void default_console_t::set_background(Color col)
{
    attr = (attr & 0x0F) | ((col & 0x0F) << 8);
}

void default_console_t::set_attr(Color fore, Color back)
{
    set_color(fore);
    set_background(back);
}

void default_console_t::locate(int row, int col)
{
    cursor = (row * LINE_PITCH) + (col * 2);
    // Set VGA hardware cursor
    x86_cpu_t::outb(0x3d4, 14);            // Tell the VGA board we are setting the high cursor byte.
    x86_cpu_t::outb(0x3d5, cursor >> 8);   // Send the high cursor byte.
    x86_cpu_t::outb(0x3d4, 15);            // Tell the VGA board we are setting the low cursor byte.
    x86_cpu_t::outb(0x3d5, cursor & 0xff); // Send the low cursor byte.
}

void default_console_t::scroll_up()
{
    memutils::move_memory((void*)rambuf, (void*)(rambuf+LINE_PITCH), sizeof(rambuf)-LINE_PITCH);
    memutils::fill_memory((void*)(rambuf+LINE_PITCH*(LINE_COUNT-1)), 0, LINE_PITCH);
}

void default_console_t::newline()
{
    print_char(eol);
    blit();
}

/** Print decimal integer */
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
    blit();
}

inline void default_console_t::print_byte_internal(unsigned char n)
{
    const char hexdigits[17] = "0123456789abcdef"; // 16+1 for terminating null
    char c = hexdigits[(n >> 4) & 0xF];
    print_char(c);
    c = hexdigits[n & 0xF];
    print_char(c);
}

/** Print hexadecimal byte */
void default_console_t::print_byte(unsigned char n)
{
    print_byte_internal(n);
    blit();
}

/** Print hexadecimal integer */
void default_console_t::print_hex(uint32_t n)
{
    print_str("0x");
    print_byte_internal((n >> 24) & 0xff);
    print_byte_internal((n >> 16) & 0xff);
    print_byte_internal((n >> 8) & 0xff);
    print_byte_internal(n & 0xff);
    blit();
}

void default_console_t::print_hex2(uint16_t n)
{
    print_str("0x");
    print_byte_internal((n >> 8) & 0xff);
    print_byte_internal(n & 0xff);
    blit();
}

/** Print 64 bit hex integer */
void default_console_t::print_hex8(unsigned long long n)
{
    print_str("0x");
    for(int i = 8; i > 0; i--)
        print_byte_internal((n >> (i-1)*8) & 0xFF);
    blit();
}

/* Minimal support for startup I/O */

#if defined(CONFIG_COMPORT)

#if CONFIG_COMPORT == 0
# define COMPORT 0x3f8
#elif CONFIG_COMPORT == 1
# define COMPORT 0x2f8
#elif CONFIG_COMPORT == 2
# define COMPORT 0x3e8
#elif CONFIG_COMPORT == 3
# define COMPORT 0x2e8
#else
#define COMPORT CONFIG_COMPORT
#endif

static void init_serial(void)
{
    #define IER     (COMPORT+1)
    #define EIR     (COMPORT+2)
    #define LCR     (COMPORT+3)
    #define MCR     (COMPORT+4)
    #define LSR     (COMPORT+5)
    #define MSR     (COMPORT+6)
    #define DLLO    (COMPORT+0)
    #define DLHI    (COMPORT+1)

    x86_cpu_t::outb(LCR, 0x80);          /* select bank 1        */
    for (volatile int i = 10000000; i--; );
    x86_cpu_t::outb(DLLO, (((115200/CONFIG_COMSPEED) >> 0) & 0x00FF));
    x86_cpu_t::outb(DLHI, (((115200/CONFIG_COMSPEED) >> 8) & 0x00FF));
    x86_cpu_t::outb(LCR, 0x03);          /* set 8,N,1            */
    x86_cpu_t::outb(IER, 0x00);          /* disable interrupts   */
    x86_cpu_t::outb(EIR, 0x07);          /* enable FIFOs */
    x86_cpu_t::inb(IER);
    x86_cpu_t::inb(EIR);
    x86_cpu_t::inb(LCR);
    x86_cpu_t::inb(MCR);
    x86_cpu_t::inb(LSR);
    x86_cpu_t::inb(MSR);
}

#endif  /* CONFIG_COMPORT */

/** Print a single character */
void default_console_t::print_char(char ch)
{
#if defined(CONFIG_COMPORT)
    static bool do_init = true;

    if (do_init)
    {
        do_init = false;
        init_serial();
    }
#endif /* CONFIG_COMPORT */

    switch (ch)
    {
        case '\r':
            break;
        case '\n':
            do
            {
                rambuf[cursor++] = ' ';
                rambuf[cursor++] = attr;
            }
            while (cursor % LINE_PITCH != 0);
            break;
        case '\t':
            do
            {
                rambuf[cursor++] = ' ';
                rambuf[cursor++] = attr;
            }
            while (cursor % TAB_PITCH != 0);
            break;
        default:
            if (ch < ' ')
                ch = '.'; // replace non-printable chars
            rambuf[cursor++] = ch; /* character */
            rambuf[cursor++] = attr; /* foreground, background colors. */
    }

    if (cursor >= LINE_PITCH*LINE_COUNT)
    {
        scroll_up();
        cursor = LINE_PITCH * (LINE_COUNT - 1);
    }
    bochs_console_print_char(ch);
//     serial_print_char(ch);
}

void default_console_t::print_unprintable(char ch)
{
    if (ch == '\n' || ch == '\r' || ch == '\t')
        ch = '.';
    print_char(ch);
}

/** Wait for Enter key press and release on keyboard */
void default_console_t::wait_ack()
{
    uint8_t keycode;
    uint8_t irqmask = x86_cpu_t::inb(0x21);
    x86_cpu_t::outb(0x21, irqmask | 0x02); // mask irq1 - keyboard

    /* wait keypress */
    do {
        while ((x86_cpu_t::inb(0x64) & 0x01) == 0) {}
        keycode = x86_cpu_t::inb(0x60);
    } while (keycode != 0x1c); // "make code" == enter

    /* wait keyrelease */
    do {
        while ((x86_cpu_t::inb(0x64) & 0x01) == 0) {}
        keycode = x86_cpu_t::inb(0x60);
    } while (keycode != 0x9c); // "break code" == enter

    if (!(irqmask & 0x02)) // if irq1 was unmasked previously,
        x86_cpu_t::outb(0x21, x86_cpu_t::inb(0x21) & 0xfd); // unmask it now without changing other flags
}

/** Print string without formatting */
void default_console_t::print_str(const char *str)
{
    char *b = (char *)str;
    while (*b)
        print_char(*b++);
    blit();
}

void default_console_t::debug_log(const char *str, ...)
{
    unsigned char old_attr = attr;
    set_attr(WHITE, BLACK);
    print_str(str);
    newline();
    attr = old_attr;
}
