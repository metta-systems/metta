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

    kw_type, kw_sequence, kw_set, kw_range, kw_record, kw_enum, kw_array
};

} // end namespace token
