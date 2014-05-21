// A fake console with no output, used for logger::debug() etc. streams.
//
#include "default_console.h"

null_console_t& null_console_t::self()
{
    static null_console_t instance;
    return instance;
}

null_console_t::null_console_t() {}

void null_console_t::set_color(Color) {}
void null_console_t::set_background(Color) {}
void null_console_t::set_attr(Color, Color) {}
void null_console_t::clear() {}
void null_console_t::locate(int, int) {}
void null_console_t::scroll_up() {}
void null_console_t::newline() {}
void null_console_t::print_int(int) {}
void null_console_t::print_char(char) {}
void null_console_t::print_unprintable(char) {}
void null_console_t::print_byte(unsigned char) {}
void null_console_t::print_hex(uint32_t) {}
void null_console_t::print_hex2(uint16_t) {}
void null_console_t::print_hex8(uint64_t) {}
void null_console_t::print_str(const char *) {}
void null_console_t::wait_ack() {}
void null_console_t::debug_log(const char *, ...) {}
