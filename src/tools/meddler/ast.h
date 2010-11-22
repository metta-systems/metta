#pragma once

#include <vector>
#include <string>

namespace AST
{

class var_decl_t;
class exception_t;

class node_t
{
public:
    virtual void dump() = 0;
    virtual bool add_field(var_decl_t*) { return false; }
    virtual bool add_exception_def(exception_t*) { return false; }
};

// Variable or parameter declaration as "type-name" pair.
class var_decl_t : public node_t
{
public:
    var_decl_t(std::string tp) : type(tp) {}
    virtual void dump();

    std::string type; // use known types! check LLVM's Type/TypeBuilder
    std::string name;
};

// Represents both method arguments and returns.
class parameter_t : public var_decl_t
{
public:
    enum { in, out, inout } direction;
};

class alias_t : public var_decl_t
{
};
// subtree of alias defines concrete variants: set_alias, array_alias, record_alias

class exception_t : public node_t
{
public:
    std::string name;
    std::vector<var_decl_t*> fields;

    exception_t(std::string nm) : name(nm) {}
    virtual bool add_field(var_decl_t* field);
    virtual void dump();
};

class method_t : public node_t
{
public:
    virtual void dump();

    bool idempotent; //..., async, oneway(==never_returns?)
    std::string name;
    std::vector<parameter_t*> params;
    std::vector<parameter_t*> returns;
    std::vector<exception_t*> raises;
    bool never_returns;
};

class interface_t : public node_t
{
public:
    interface_t(std::string nm, bool is_local, bool is_final) : local(is_local), final(is_final), name(nm) {}
    virtual bool add_exception_def(exception_t* exc);
    virtual void dump();

    bool local;
    bool final;
    std::string name;
    std::string base;
    std::vector<interface_t*> imports;//implicitly derived from 'types' contents.
    std::vector<alias_t*>     types;
    std::vector<exception_t*> exceptions;
    std::vector<method_t*>    methods;
};

}
