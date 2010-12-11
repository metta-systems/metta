#include <iostream>
#include <algorithm>
#include "ast.h"

namespace AST
{

bool interface_t::add_exception(exception_t* exc)
{
    std::cout << "interface_t::add_exception()" << std::endl;
    exceptions.push_back(exc);
    return true;
}

bool interface_t::add_type(alias_t* t)
{
    std::cout << "interface_t::add_type()" << std::endl;
    types.push_back(t);
    return true;
}

bool interface_t::add_method(method_t* m)
{
    std::cout << "interface_t::add_method()" << std::endl;
    methods.push_back(m);
    return true;
}

bool exception_t::add_field(var_decl_t* field)
{
    std::cout << "exception_t::add_field()" << std::endl;
    fields.push_back(field);
    return true;
}

bool method_t::add_parameter(parameter_t* p)
{
    std::cout << "method_t::add_parameter()" << std::endl;
    params.push_back(p);
    return true;
}

bool method_t::add_return(parameter_t* r)
{
    std::cout << "method_t::add_return()" << std::endl;
    returns.push_back(r);
    return true;
}

bool method_t::add_exception(exception_t* e)
{
    std::cout << "method_t::add_exception()" << std::endl;
    raises.push_back(e);
    return true;
}

void alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << type << " " << name << std::endl;
}

void var_decl_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << type << (reference ? "& " : " ") << name;
}

void parameter_t::dump(std::string indent_prefix)
{
    const char* dirs[] = {"in ", "out ", "inout "};
    std::cout << indent_prefix << dirs[direction]; var_decl_t::dump(""); std::cout << std::endl;
}

void interface_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << "interface_t(\"" << name << "\", local:" << (local ? "true":"false") << ", final:" << (final ? "true":"false") << ")" << (!base.empty() ? std::string(" extends ") + base : "") << std::endl;

    std::cout << indent_prefix << "+-types" << std::endl;
    if (types.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(types.begin(), types.end(), [indent_prefix](alias_t* a) { a->dump(indent_prefix + "  "); });

    std::cout << indent_prefix << "+-exceptions" << std::endl;
    if (exceptions.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(exceptions.begin(), exceptions.end(), [indent_prefix](exception_t* e) { e->dump(indent_prefix + "  "); });

    std::cout << indent_prefix << "+-methods" << std::endl;
    if (methods.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(methods.begin(), methods.end(), [indent_prefix](method_t* m) { m->dump(indent_prefix + "  "); });
}

void exception_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << "exception_t(\"" << name << "\")" << std::endl;
    std::cout << indent_prefix << "+-fields" << std::endl;
    if (fields.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(fields.begin(), fields.end(), [indent_prefix](var_decl_t* var){
            std::cout << indent_prefix << "  "; var->dump(""); std::cout << ";" << std::endl;
        });
}

void method_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << "method_t(\"" << name << "\")" << (idempotent ? " idempotent" : "") << std::endl;
    std::cout << indent_prefix << "+-arguments" << std::endl;
    if (params.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(params.begin(), params.end(), [indent_prefix](parameter_t* p) { p->dump(indent_prefix + "  "); });
    std::cout << indent_prefix << "+-returns" << std::endl;
    if (returns.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(returns.begin(), returns.end(), [indent_prefix](parameter_t* p) { p->dump(indent_prefix + "  "); });
    std::cout << indent_prefix << "+-exceptions" << std::endl;
    if (raises.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        std::for_each(raises.begin(), raises.end(), [indent_prefix](exception_t* e) { e->dump(indent_prefix + "  "); });
}

}
