#include <string.h>
#include "token.h"
#include "lexer.h"

lexer_t::lexer_t(llvm::MemoryBuffer *StartBuf)
    : cur_buf(StartBuf)
{
    cur_ptr = cur_buf->getBufferStart();
    cur_kind = next_kind = token::none;
}

int lexer_t::get_next_char()
{
    char cur_char = *cur_ptr++;
    switch (cur_char)
    {
        default: return (unsigned char)cur_char;
        case 0:
            // A nul character in the stream is either the end of the current buffer or
            // a random nul in the file.  Disambiguate that here.
            if (cur_ptr - 1 != cur_buf->getBufferEnd())
                return 0;  // Just whitespace.

            // Otherwise, return end of file.
            --cur_ptr;  // Another call to lex will return EOF again.
            return EOF;
    }
}

void lexer_t::skip_line_comment()
{
    while (1)
    {
        if (*cur_ptr == '\n' || *cur_ptr == '\r' || get_next_char() == EOF)
            return;
    }
}

token::kind lexer_t::get_token()
{
    if (next_kind != token::none)
    {
        token::kind t = next_kind;
        next_kind = token::none;
        return t;
    }

    token_start = cur_ptr;

    int cur_char = get_next_char();
    switch (cur_char)
    {
        case EOF: return token::eof;
        case 0: case ' ': case '\t': case '\n': case '\r': // Ignore whitespace.
            return get_token();
        case '#':
            skip_line_comment();
            return get_token();
        case '=': return token::equal;
        case ',': return token::comma;
        case '&': return token::reference;
        case '[': return token::lsquare;
        case ']': return token::rsquare;
        case '{': return token::lbrace;
        case '}': return token::rbrace;
        case '<': return token::less;
        case '>': return token::greater;
        case '(': return token::lparen;
        case ')': return token::rparen;
        case ';': return token::semicolon;
        case '\\': return token::backslash;
        default:
            return get_identifier();
    }
}

/// is_label_char - Return true for [a-zA-Z._0-9].
static bool is_label_char(char c)
{
    return isalnum(c) || c == '.' || c == '_';
}

/// get_identifier: Handle several related productions:
///    keyword         interface, idempotent, ...
///    integer         [0-9]+
///    hex integer     0x[0-9A-Fa-f]+
token::kind lexer_t::get_identifier()
{
    const char *start_ptr = cur_ptr;

    for (; is_label_char(*cur_ptr); ++cur_ptr)
    {
    }

    --start_ptr;
    unsigned int len = cur_ptr - start_ptr;

#define KEYWORD(word) \
    if (len == strlen(#word) && !memcmp(start_ptr, #word, len)) \
        return token::kw_##word;

    KEYWORD(local);
    KEYWORD(final);
    KEYWORD(interface);
    KEYWORD(exception);
    KEYWORD(in);
    KEYWORD(inout);
    KEYWORD(out);
    KEYWORD(idempotent);
    KEYWORD(raises);
    KEYWORD(needs);
    KEYWORD(extends);
    KEYWORD(never);
    KEYWORD(returns);

    // TODO: handle types

    return token::identifier;
}
