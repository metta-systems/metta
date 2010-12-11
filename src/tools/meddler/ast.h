#pragma once

#include <vector>
#include <string>

namespace AST
{

class var_decl_t;
class exception_t;
class alias_t;
class method_t;

class node_t
{
public:
    //TODO: add detailed position information: source buffer, line, column

    virtual void dump(std::string indent_prefix) = 0;
    virtual bool add_field(var_decl_t*) { return false; }
    virtual bool add_exception(exception_t*) { return false; }
    virtual bool add_type(alias_t*) { return false; }
    virtual bool add_method(method_t*) { return false; }
};

// Type, variable or parameter declaration as "type-name" pair.
// subtree of alias defines concrete variants: set_alias, array_alias, record_alias
class alias_t : public node_t
{
public:
    alias_t() : node_t(), type(), name() {}
    virtual void dump(std::string indent_prefix);

    std::string type; // use known types! check LLVM's Type/TypeBuilder
    std::string name;
};

// Variable or parameter declaration as "type-name" pair.
class var_decl_t : public alias_t
{
public:
    var_decl_t() : alias_t(), reference(false) {}
    void set_reference() { reference = true; }
    virtual void dump(std::string indent_prefix);

    bool reference;
};

class type_alias_t : public alias_t
{
public:
    type_alias_t() : alias_t() {}
};

// Represents both method arguments and returns.
class parameter_t : public var_decl_t
{
public:
    enum direction_e { in, out, inout } direction;
    virtual void dump(std::string indent_prefix);
};

class exception_t : public node_t
{
public:
    std::string name;
    std::vector<var_decl_t*> fields;

    exception_t(std::string nm) : name(nm) {}
    virtual bool add_field(var_decl_t* field);
    virtual void dump(std::string indent_prefix);
};

class method_t : public node_t
{
public:
    virtual void dump(std::string indent_prefix);
    virtual bool add_parameter(parameter_t*);
    virtual bool add_return(parameter_t*);
    virtual bool add_exception(exception_t*);

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
    virtual bool add_method(method_t*);
    virtual bool add_exception(exception_t*);
    virtual bool add_type(alias_t*);
    virtual void dump(std::string indent_prefix);
    void set_parent(std::string p) { base = p; }

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
