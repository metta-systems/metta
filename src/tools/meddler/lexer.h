#pragma once

class lexer_t
{
    const char *cur_ptr;
    MemoryBuffer *cur_buf;
    // Information about current token.
    const char *token_start;
    token::kind cur_kind;

public:
    explicit lexer_t(MemoryBuffer *StartBuf);//, SourceMgr &SM

    token::kind lex()
    {
        return (cur_kind = get_token());
    }

private:
    token::kind get_token();
    token::kind get_identifier();
};
