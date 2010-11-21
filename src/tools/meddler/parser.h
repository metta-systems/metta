#pragma once

#include "lexer.h"
#include "ast.h"
#include <llvm/Support/MemoryBuffer.h>

class parser_t
{
    bool is_local, is_final; // TODO: move to private class parser_state_t
    lexer_t lex;
    AST::node_t* parse_tree;

    bool parse_top_level_entities();
    bool parse_interface();
    bool parse_interface_body();

public:
    parser_t(llvm::MemoryBuffer *F) : is_local(false), is_final(false), lex(F), parse_tree(0) {}
    bool run();
};
