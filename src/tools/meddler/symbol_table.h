#pragma once

#include "map"
#include "string"
#include "token.h"

//TODO: replace with llvm::TypeSymbolTable?
// or some other symbol table class...
class symbol_table_t
{
    std::map<std::string, token::kind> symbols;

public:
    typedef std::map<std::string, token::kind>::iterator iterator;
    iterator begin() { return symbols.begin(); }
    iterator end() { return symbols.end(); }

    symbol_table_t();
    /*!
     * Insert key into symbol table with kind type. Return index of inserted symbol or end() if failed.
     */
    iterator insert(std::string key, token::kind type);
    /*!
     * Insert key into symbol table with kind type. Return true if symbol did not exist.
     */
    bool insert_checked(std::string key, token::kind type)
    {
        return insert(key, type) != end();
    }
    /*!
     * Find symbol index by given key. Return index of symbol or end() if not found.
     */
    iterator lookup(std::string key);
    /*!
     * Return kind of symbol at position idx. Return token::none if symbol doesn't exist.
     */
    token::kind kind(iterator idx);

    /*!
     * Return true if symbol at position idx denotes a builtin type.
     */
    bool is_builtin_type(iterator idx);
    bool is_interface_type(iterator idx);
    bool is_exception_type(iterator idx);

    void clear() { symbols.clear(); }
    void dump();
};
