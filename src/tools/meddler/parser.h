#pragma once

#include "lexer.h"
#include <llvm/Support/MemoryBuffer.h>

class parser_t
{
    lexer_t lex;

public:
    parser_t(llvm::MemoryBuffer *F) : lex(F) {}
    bool run();
};
