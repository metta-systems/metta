#pragma once

#include "map"
#include "string"
#include "token.h"

class symbol_table_t
{
    std::map<std::string, token::kind> symbols;

public:
    typedef std::map<std::string, token::kind>::iterator iterator;
    iterator begin() { return symbols.begin(); }
    iterator end() { return symbols.end(); }

    symbol_table_t();
    /*!
     * Insert key into symbol table with kind type. Return index of inserted symbol or -1 if failed.
     */
    iterator insert(std::string key, token::kind type);
    /*!
     * Find symbol index by given key. Return index of symbol or -1 if not found.
     */
    iterator lookup(std::string key);
    /*!
     * Return kind of symbol at position idx. Return token::none if symbol doesn't exist.
     */
    token::kind kind(iterator idx);

    void dump();
};
