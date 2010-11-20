#pragma once

#include "lexer.h"
#include <llvm/Support/MemoryBuffer.h>

class parser_t
{
    bool is_local, is_final;
    lexer_t lex;
    bool parse_top_level_entities();
    bool parse_interface();
    bool parse_interface_body();

public:
    parser_t(llvm::MemoryBuffer *F) : is_local(false), is_final(false), lex(F) {}
    bool run();
};
