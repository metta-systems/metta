//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace token
{

enum kind
{
    none,

    // Markers
    eof, error,

    // Tokens with no info.
    dotdot,            // ..
    equal, comma,      // =  ,
    reference,         // &
    dblarrow,          // =>
    lsquare, rsquare,  // [  ]
    lbrace, rbrace,    // {  }
    less, greater,     // <  >
    lparen, rparen,    // (  )
    semicolon,         // ;
    backslash,         // \    (not /)
    cardinal,          // a base 8, 10 or 16 positive number
    type,              // int, float, sequence etc
    _builtin_type,     // used only internally by symbol_table_t
    _interface_type,
    _exception_type,
    identifier,        // a generic id (usually variable name)

    kw_local, kw_final, kw_interface, kw_exception,
    kw_in, kw_inout, kw_out, kw_idempotent,
    kw_raises, kw_extends, kw_never, kw_returns,

    kw_type, kw_sequence, kw_set, kw_range, kw_record, kw_choice, kw_on, kw_enum, kw_array,

    autodoc // Autodoc comments
};

} // end namespace token
