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

class logging
{
public:
    template <typename T>
	console_t& operator << (const T& v) {
        if (my_log_level >= log_level)
        {
            kconsole << v;
            return kconsole;
        }
        return null_console;
    }

    enum log_levels {
        trace_level = 0,
        debug_level,
        info_level,
        none_level // warning and fatal are always displayed!
    };
    static void set_verbosity(log_levels verbosity);

protected:
    log_levels my_log_level{none_level};

    logging(log_levels l) : my_log_level(l) {}
    ~logging() { if (my_log_level >= log_level) kconsole << endl; }

    friend class function_scope;

private:
    static log_levels log_level;
};

/**
 * Allow tracing a scope with a name. Optional verbosity setting allows to adjust
 * tracing granularity.
 *
 * @example
 * {
 * 	  function_scope outer("outer scope", logging::debug_level);
 * 	  {
 * 	  	  function_scope inner("inner scope", logging::trace_level);
 * 	  	  return;
 * 	  }
 * }
 * @endexample
 */
class function_scope
{
    const char* name{0};
    logging::log_levels level{logging::trace_level};
public:
    function_scope(const char *fn, logging::log_levels verbosity = logging::trace_level);
    ~function_scope();
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
    warning() : logging(none_level) {
        kconsole << WARNING << "[WARNING] ";
    }
};

class fatal : public logging
{
public:
    fatal() : logging(none_level) {
        kconsole << ERROR << "[FATAL] ";
    }
    ~fatal() { PANIC("fatal error"); }
};

}
