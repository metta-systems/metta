#include "ast.h"
#include "macros.h"
#include <sstream>
#include <algorithm>

namespace AST
{

void interface_t::emit_impl_h(std::ostringstream& s)
{
    s << "#pragma once" << std::endl << std::endl;
    // TODO: missing forward declarations
    s << "struct " << name << "_ops" << std::endl
      << "{" << std::endl;

    std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
    {
        m->emit_impl_h(s);
    });

    s << "};" << std::endl;
}

void interface_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void interface_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void method_t::emit_impl_h(std::ostringstream& s)
{
    std::string return_value_type;
    if (never_returns || returns.size() == 0)
        return_value_type = "void";
    else
    {
        return_value_type = returns.front()->type;
        if (returns.front()->reference)
            return_value_type += "&";
    }

    s << '\t' << return_value_type
      << " (*" << name << ")(";

    s << parent_interface << "_closure* self";

    std::for_each(params.begin(), params.end(), [&s](parameter_t* param)
    {
        s << ", ";
        param->emit_impl_h(s);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s](parameter_t* param)
        {
            s << ", ";
            param->emit_impl_h(s);
        });
    }

    s << ");" << std::endl;
}

void method_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void method_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void exception_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void exception_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void exception_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void var_decl_t::emit_impl_h(std::ostringstream& s)
{
    s << type;
    if (reference)
        s << "&";
    s << " " << name;
}

void var_decl_t::emit_interface_h(std::ostringstream& s)
{
    s << type;
    if (reference)
        s << "&";
    s << " " << name;
}

void var_decl_t::emit_interface_cpp(std::ostringstream& s)
{
    s << type;
    if (reference)
        s << "&";
    s << " " << name;
}

void type_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void type_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void type_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void sequence_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void sequence_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void sequence_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void array_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void array_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void array_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void set_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void set_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void set_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void record_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void record_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void record_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void enum_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void enum_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void enum_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void range_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void range_alias_t::emit_interface_h(std::ostringstream& s UNUSED_ARG)
{
}

void range_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

}
