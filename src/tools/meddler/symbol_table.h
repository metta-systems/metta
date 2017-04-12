//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <map>
#include <string>
#include <vector>
#include "token.h"

//TODO: replace with llvm::TypeSymbolTable?
// or some other symbol table class...
class symbol_table_t
{
    std::map<std::string, token::kind> symbols;
    std::vector<std::string> scope;

public:
    typedef std::map<std::string, token::kind>::iterator iterator;
    iterator begin() { return symbols.begin(); }
    iterator end() { return symbols.end(); }

    symbol_table_t() = default;

    /**
     * Insert key into symbol table with kind type. Return index of inserted symbol or end() if failed.
     */
    iterator insert(const std::string& key, token::kind type);
    /**
     * Insert key into symbol table with kind type. Return true if symbol did not exist.
     */
    bool insert_checked(std::string key, token::kind type)
    {
        return insert(key, type) != end();
    }
    /**
     * Find symbol index by given key. Return index of symbol or end() if not found.
     */
    iterator lookup(const std::string& key);
    /**
     * Return kind of symbol at position idx. Return token::none if symbol doesn't exist.
     */
    token::kind kind(iterator idx);

    /**
     * Return true if symbol at position idx denotes a builtin type.
     */
    bool is_builtin_type(iterator idx);
    bool is_interface_type(iterator idx);
    bool is_exception_type(iterator idx);

    bool is_qualified_type_name(const std::string& identifier);

    /**
     * Return a fully qualified name of identifier in current scope.
     */
    std::string qualify(const std::string& identifier);

    void clear() { symbols.clear(); }
    void dump();

private: friend class local_scope_t;
    /**
     * Enter nested scope.
     */
    void enter_scope(const std::string& name);
    /**
     * Leave nested scope.
     */
    void leave_scope();
};

class local_scope_t
{
    symbol_table_t& symbols;
public:
    local_scope_t(symbol_table_t& syms, std::string scope) : symbols(syms) { symbols.enter_scope(scope); }
    ~local_scope_t() { symbols.leave_scope(); }
};
