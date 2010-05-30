#include "token.h"
#include "lexer.h"

token::kind lexer_t::get_token()
{
    token_start = cur_ptr;

    int cur_char = get_next_char();
    switch (cur_char)
    {
        case EOF: return token::eof;
        case '=': return token::equal;
        case ',': return token::comma;
        case '*': return token::star;
        case '[': return token::lsquare;
        case ']': return token::rsquare;
        case '{': return token::lbrace;
        case '}': return token::rbrace;
        case '<': return token::less;
        case '>': return token::greater;
        case '(': return token::lparen;
        case ')': return token::rparen;
        case '\\': return token::backslash;
        default:
            return get_identifier();
    }
}

/// get_identifier: Handle several related productions:
///    keyword         interface, idempotent, ...
///    integer         [0-9]+
///    hex integer     0x[0-9A-Fa-f]+
token::kind lexer_t::get_identifier()
{
    const char *start_ptr = cur_ptr;

    for (; isalpha(*cur_ptr); ++cur_ptr)
    {
    }

    --start_ptr;
    int len = cur_ptr - start_ptr;

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
