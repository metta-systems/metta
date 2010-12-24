#pragma once

#include <iostream>
#include "logger.h"
#include "token.h"
#include "symbol_table.h"
#include <llvm/Support/MemoryBuffer.h>

class lexer_t
{
    const char *cur_ptr;
    llvm::MemoryBuffer *cur_buf;
    symbol_table_t *symbols;
    // Information about current token.
    const char *token_start;
    token::kind cur_kind; // lookahead
    token::kind next_kind;
    unsigned token_val;

public:
    explicit lexer_t(llvm::MemoryBuffer *StartBuf, symbol_table_t* sym);//, SourceMgr &SM

    token::kind lex()
    {
        cur_kind = get_token();
        L(std::cerr << "LEX: token " << current_token() << " kind " << cur_kind << std::endl);
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

    // Match current token to kind.
    bool match(token::kind kind)
    {
        return token_kind() == kind;
    }

    // Read next token and expect it to be of type kind
    bool expect(token::kind kind)
    {
        return (lex() == kind);
    }

    // Lookup next token and see if it matches kind
    bool maybe(token::kind kind)
    {
        if (lex() == kind)
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
};
