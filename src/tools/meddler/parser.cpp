#include <iostream>
#include "parser.h"

// if (kind == kw_interface) parse_interface(false);
// if (kind == kw_local) parse_interface(true);
// if (kind == kw_needs) parse_imports();
// if (kind == kw_range) parse_range_type_alias();
// if (kind == kw_sequence) parse_sequence_type_alias();
// if (kind == kw_set) parse_set_type_alias();
// if (kind == kw_record) parse_record_type_alias();
// if (kind == kw_exception) parse_exception();
// if (kind == type_decl) parse_type_alias();
// if (kind == kw_idempotent) parse_method();
// if (kind == identifier) parse_method();
// bool parse_interface(bool local);

std::string token_to_name(token::kind tok)
{
#define TNAME(tk) \
    case token::kind::tk: return #tk;
    switch (tok)
    {
        TNAME(eof)
        TNAME(error)
        TNAME(equal)
        TNAME(comma)
        TNAME(reference)
        TNAME(lsquare)
        TNAME(rsquare)
        TNAME(lbrace)
        TNAME(rbrace)
        TNAME(less)
        TNAME(greater)
        TNAME(lparen)
        TNAME(rparen)
        TNAME(semicolon)
        TNAME(backslash)
        TNAME(type)
        TNAME(kw_local)
        TNAME(kw_final)
        TNAME(kw_interface)
        TNAME(kw_exception)
        TNAME(kw_in)
        TNAME(kw_inout)
        TNAME(kw_out)
        TNAME(kw_idempotent)
        TNAME(kw_raises)
        TNAME(kw_needs)
        TNAME(kw_extends)
        TNAME(kw_never)
        TNAME(kw_returns)
        TNAME(identifier)
    }
    return "UNKNOWN";
}

bool parser_t::run()
{
    token::kind t = lex.lex();
    while (t != token::kind::eof)
    {
        std::cout << token_to_name(t) << ": " << lex.current_token() << std::endl;
        t = lex.lex();
    }
    return false;
}
