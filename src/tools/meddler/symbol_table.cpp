#include "symbol_table.h"
#include <algorithm>
#include <iostream>

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
    return (*idx).second;
}

void symbol_table_t::dump()
{
    std::for_each(begin(), end(), [](std::pair<std::string, token::kind> it){
        std::cout << "symbol: " << it.first << ", value: " << it.second << std::endl;
    });
}
