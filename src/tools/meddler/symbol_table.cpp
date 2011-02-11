#include "symbol_table.h"
#include <algorithm>
#include <iostream>

// symbol_t::qualify()
// if qualified return itself
// else prepend current scope qualifier

// void enter_scope(std::string name)
// {
//     scope.push_back(name);
// }
// void leave_scope()
// {
//     scope.pop_front();
// }

symbol_table_t::symbol_table_t()
{
}

symbol_table_t::iterator symbol_table_t::insert(std::string key, token::kind type)
{
    return symbols.insert(std::make_pair(key, type)).first;
}

symbol_table_t::iterator symbol_table_t::lookup(std::string key)
{
    return symbols.find(key);
}

token::kind symbol_table_t::kind(symbol_table_t::iterator idx)
{
    if (idx == end())
        return token::kind::none;
    token::kind k = (*idx).second;
    if (k == token::kind::_builtin_type || k == token::kind::_interface_type || k == token::kind::_exception_type)
        k = token::kind::type;
    return k;
}

bool symbol_table_t::is_builtin_type(iterator idx)
{
    if (idx == end())
        return false;
    return (*idx).second == token::kind::_builtin_type;
}

bool symbol_table_t::is_interface_type(iterator idx)
{
    if (idx == end())
        return false;
    return (*idx).second == token::kind::_interface_type;
}

bool symbol_table_t::is_exception_type(iterator idx)
{
    if (idx == end())
        return false;
    return (*idx).second == token::kind::_exception_type;
}

extern std::string token_to_name(token::kind tok);

void symbol_table_t::dump()
{
    std::for_each(begin(), end(), [](std::pair<std::string, token::kind> it){
        std::cout << "symbol: " << it.first << ", value: " << it.second << ", name: " << token_to_name(it.second) << std::endl;
    });
}
