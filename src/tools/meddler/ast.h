#pragma once

namespace AST
{

// Variable or parameter declaration as "type-name" pair.
class var_decl_t
{
    std::string type; // use known types!
    std::string name;
};

// Represents both method arguments and returns.
class parameter_t : public var_decl_t
{
    enum { in, out, inout } direction;
};

class alias_t : public var_decl_t
{
};
// subtree of alias defines concrete variants: set_alias, array_alias, record_alias

class exception_t
{
    std::string name;
    std::vector<var_decl_t*> fields;
};

class method_t
{
    bool idempotent;
    std::string name;
    std::vector<parameter_t*> params;
    std::vector<parameter_t*> returns;
    std::vector<exception_t*> raises;
    bool never_returns;
};

class interface_t
{
    bool local;
    bool final;
    std::string name;
    std::string base;
    std::vector<interface_t*> imports;
    std::vector<alias_t*>     types;
    std::vector<exception_t*> exceptions;
    std::vector<method_t*>    methods;
};

}
