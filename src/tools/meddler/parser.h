#pragma once

#include "lexer.h"

class parser_t
{
    lexer_t lex;

public:
    parser_t(MemoryBuffer *F) : lex(F) {}
    bool run();
};
