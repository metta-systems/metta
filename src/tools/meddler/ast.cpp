#include <iostream>
#include <algorithm>
#include "ast.h"

namespace AST
{

void var_decl_t::dump()
{
}

bool interface_t::add_exception_def(exception_t* exc)
{
    exceptions.push_back(exc);
    return true;
}

void interface_t::dump()
{
    std::cout << "interface_t(\"" << name << "\", local:" << (local ? "true":"false") << ", final:" << (final ? "true":"false") << ")" << std::endl;
    std::cout << "+-types" << std::endl;
    if (types.size() == 0)
        std::cout << "  [empty]" << std::endl;
    std::cout << "+-exceptions" << std::endl;
    if (exceptions.size() == 0)
        std::cout << "  [empty]" << std::endl;
    std::cout << "+-methods" << std::endl;
    if (methods.size() == 0)
        std::cout << "  [empty]" << std::endl;
}

void exception_t::dump()
{
    std::cout << "exception_t(\"" << name << "\")" << std::endl;
    std::cout << "+-fields" << std::endl;
    if (fields.size() == 0)
        std::cout << "  [empty]" << std::endl;
    else
        std::for_each(fields.begin(), fields.end(), [](var_decl_t* var){
            std::cout << "  " << var->type << " " << var->name << ";" << std::endl;
        });
}

bool exception_t::add_field(var_decl_t* field)
{
    std::cout << "exception_t::add_field()" << std::endl;
    fields.push_back(field);
    return true;
}

}
