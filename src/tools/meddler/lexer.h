#pragma once

#include "token.h"
#include <llvm/Support/MemoryBuffer.h>

class lexer_t
{
    const char *cur_ptr;
    llvm::MemoryBuffer *cur_buf;
    // Information about current token.
    const char *token_start;
    token::kind cur_kind;

public:
    explicit lexer_t(llvm::MemoryBuffer *StartBuf);//, SourceMgr &SM

    token::kind lex()
    {
        return (cur_kind = get_token());
    }

private:
    token::kind get_token();
    token::kind get_identifier();
    int get_next_char();
};
