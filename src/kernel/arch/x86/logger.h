//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "default_console.h"

namespace logger {

class function_scope
{
    const char* name;
public:
    function_scope(const char *fn);
    ~function_scope();
};

class logging
{
protected:
    enum log_levels {
        trace_level = 0,
        debug_level,
        info_level,
        warnings_level // warning and fatal are always displayed!
    };
    log_levels my_log_level{warnings_level};

    logging(log_levels l) : my_log_level(l) {}
    ~logging() { if (my_log_level >= log_level) kconsole << endl; }

private:
    static log_levels log_level;

public:
    // static void set_log_level(int v)

    template <typename T>
	console_t& operator << (const T& v) {
        if (my_log_level >= log_level)
        {
            kconsole << v;
            return kconsole;
        }
        return null_console;
    }
};

// Loglevel trace - most detailed information.
class trace : public logging
{
public:
    trace() : logging(trace_level) {}
};

class debug : public logging
{
public:
    debug() : logging(debug_level) {}
};

class info : public logging
{
public:
    info() : logging(info_level) {}
};

class warning : public logging
{
public:
    warning() : logging(warnings_level) {
        kconsole << "[WARNING] ";
    }
};

class fatal : public logging
{
public:
    fatal() : logging(warnings_level) {
        kconsole << "[FATAL] ";
    }
    ~fatal() { PANIC("fatal error"); }
};

}
