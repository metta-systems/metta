//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iostream>
#include "logger.h"
#include "token.h"
#include "symbol_table.h"
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SMLoc.h>

class lexer_t
{
    const char *cur_ptr;
    const llvm::MemoryBuffer *cur_buf;
    symbol_table_t *symbols;
    // Information about current token.
    const char *token_start;
    token::kind cur_kind; // lookahead
    token::kind next_kind;
    unsigned token_val;
	bool verbose;

public:
    lexer_t(bool be_verbose);//, SourceMgr &SM
    explicit lexer_t(const llvm::MemoryBuffer *StartBuf, symbol_table_t* sym, bool be_verbose);//, SourceMgr &SM
    void init(const llvm::MemoryBuffer *StartBuf, symbol_table_t* sym);

    token::kind lex()
    {
        cur_kind = get_token();
        if (verbose)
            std::cerr << "LEX: token " << current_token() << " kind " << cur_kind << std::endl;
        return cur_kind;
    }

    // Put lexed token back into the stream for next lex() to consume.
    // helpful for lex.maybe(token);
    void lexback()
    {
        next_kind = cur_kind;
    }

    token::kind token_kind()
    {
        return cur_kind;
    }

    std::string current_token()
    {
        return std::string(token_start, (int)(cur_ptr - token_start));
    }

    unsigned current_value()
    {
        return token_val;
    }

    llvm::SMLoc current_loc()
    {
        return llvm::SMLoc::getFromPointer(token_start);
    }

    // @todo: add want_lex(kind) method to look up an identifier of expected type (e.g. ignore if expecting an
    // kw_identifier and got a kw_choice - 'choice' can be an id too). Use want_lex() in match/expect/maybe below.

    bool want_lex(token::kind wanted_kind)
    {
        lex();
        // extern std::string token_to_name(token::kind tok);
        // std::cerr << "want_lex(" << token_to_name(wanted_kind) << ") lexed " << token_to_name(token_kind()) << std::endl;
        if (wanted_kind != token_kind())
        {
            if (wanted_kind == token::identifier)
            {
                switch(token_kind())
                {
                    case token::kw_local:
                    case token::kw_final:
                    case token::kw_interface:
                    case token::kw_exception:
                    case token::kw_in:
                    case token::kw_inout:
                    case token::kw_out:
                    case token::kw_idempotent:
                    case token::kw_raises:
                    case token::kw_extends:
                    case token::kw_never:
                    case token::kw_returns:
                    case token::kw_type:
                    case token::kw_sequence:
                    case token::kw_set:
                    case token::kw_range:
                    case token::kw_record:
                    case token::kw_choice:
                    case token::kw_on:
                    case token::kw_enum:
                    case token::kw_array:
                    case token::type: // hmmm, if we expect an ident, type name can also be an identifier, or not??
                        return true;
                    default:
                        return false;
                }
            }
            return false;
        }
        return true;
    }

    // Match current token to kind.
    bool match(token::kind kind)
    {
        return token_kind() == kind;
    }

    // Read next token and expect it to be of type kind
    bool expect(token::kind kind)
    {
        return want_lex(kind);
    }

    // Lookup next token and see if it matches kind
    bool maybe(token::kind kind)
    {
        if (want_lex(kind))
            return true;
        lexback();
        return false;
    }

private:
    token::kind get_token();
    token::kind get_identifier();
    token::kind get_cardinal();
    int get_next_char();
    void skip_line_comment();
    void get_autodoc_line();
};
