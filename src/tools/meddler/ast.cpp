#include <iostream>
#include "ast.h"

namespace AST
{

void interface_t::dump()
{
    std::cout << "interface_t(\"" << name << "\", local:" << (local ? "true":"false") << ", final:" << (final ? "true":"false") << ")" << std::endl;
}

}
