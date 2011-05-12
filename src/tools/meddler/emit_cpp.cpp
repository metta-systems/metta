#include "ast.h"
#include "macros.h"
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

namespace AST
{

/*!
 * If a given type needs include directive, return one, otherwise return empty string.
 */
static std::string needs_include(string type)
{
	if (map_type(type).empty())
	{
		return string("#include \"")+type+".h\"";
	}
	return string();
}

/*!
 * Map IDL builtin types to C++ emitter types.
 */
static string map_type(string type)
{
    static bool type_map_built = false;
    static map<string, string> type_map;

    if (!type_map_built)
    {
        type_map["int8"] = "int8_t";
        type_map["int16"] = "int16_t";
        type_map["int32"] = "int32_t";
        type_map["int64"] = "int64_t";
        type_map["octet"] = "uint8_t";
        type_map["card16"] = "uint16_t";
        type_map["card32"] = "uint32_t";
        type_map["card64"] = "uint64_t";
        type_map["float"] = "float";
        type_map["double"] = "double";
        type_map["boolean"] = "bool";
        type_map["string"] = "const char*";
        type_map_built = true;
    }

    if (type_map.find(type) != type_map.end())
        return type_map[type];
    else
        return string();
}

static std::vector<std::string> build_forwards(interface_t* intf)
{
    std::vector<std::string> forwards;

    forwards.push_back(intf->name_);

    std::for_each(intf->methods.begin(), intf->methods.end(), [&forwards](method_t* m)
    {
        std::for_each(m->params.begin(), m->params.end(), [&forwards](parameter_t* param)
        {
            size_t pos;
            if ((pos = param->type.find_first_of('.')) != std::string::npos)
            {
                std::string decl = param->type.substr(0, pos);
                if (std::find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        });

        std::for_each(m->returns.begin(), m->returns.end(), [&forwards](parameter_t* param)
        {
            size_t pos;
            if ((pos = param->type.find_first_of('.')) != std::string::npos)
            {
                std::string decl = param->type.substr(0, pos);
                if (std::find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        });
    });

    return forwards;
}

void interface_t::emit_impl_h(std::ostringstream& s)
{
    s << "#pragma once" << std::endl << std::endl;

    std::vector<std::string> fwd = build_forwards(this);
    if (fwd.size() > 0)
    {
        std::for_each(fwd.begin(), fwd.end(), [&s](std::string str)
        {
            s << "struct " << str << "_closure;" << std::endl;
        });
        s << std::endl;
    }

    s << "// ops structure should be exposed to module implementors!" << std::endl;
    s << "struct " << name_ << "_ops" << std::endl
      << "{" << std::endl;

    std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
    {
        m->emit_impl_h(s);
    });

    s << "};" << std::endl;
}

void interface_t::emit_interface_h(std::ostringstream& s)
{
    s << "#pragma once" << std::endl << std::endl
      << "#include \"module_interface.h\"" << std::endl << std::endl
    // TODO: missing forward declarations for parameter types
	// TODO: missing declarations/definitions of interface types ("name_typename")
      << "struct " << name_ << "_ops;" << std::endl
      << "struct " << name_ << "_state;" << std::endl << std::endl
      << "struct " << name_ << "_closure" << std::endl
      << "{" << std::endl
      << '\t' << "const " << name_ << "_ops* methods;" << std::endl
      << '\t' << name_ << "_state* state;" << std::endl << std::endl;

    std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
    {
        m->emit_interface_h(s);
    });

    s << "};" << std::endl;
}

void interface_t::emit_interface_cpp(std::ostringstream& s)
{
    s << "#include \"" << name_ << "_interface.h\"" << std::endl
      << "#include \"" << name_ << "_impl.h\"" << std::endl << std::endl;

    std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
    {
        m->emit_interface_cpp(s);
    });
}

/*!
 * Generate a qualified name for a given var decl type.
 */
static std::string emit_type(var_decl_t& type)
{
    std::string result = type.type;
    if (type.is_builtin_type())
    {
        result = map_type(type.unqualified_name());
        if (result.empty())
        {
            result = type.unqualified_name();
            cout << "Unknown builtin type! " << result << endl;
        }
    }
    if (type.is_reference())
        result += "&";
    return result;
}

void method_t::emit_impl_h(std::ostringstream& s)
{
    std::string return_value_type;
    if (never_returns || (returns.size() == 0))
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front());
    }

    s << '\t' << return_value_type
      << " (*" << name_ << ")(";

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

void method_t::emit_interface_h(std::ostringstream& s)
{
    std::string return_value_type;
    if (never_returns || returns.size() == 0)
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front());
    }

    s << '\t' << return_value_type
      << " " << name_ << "(";

    bool first = true;
    std::for_each(params.begin(), params.end(), [&s, &first](parameter_t* param)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_h(s);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s, &first](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_h(s);
        });
    }

    s << ");" << std::endl;
}

void method_t::emit_interface_cpp(std::ostringstream& s)
{
    std::string return_value_type;
    if (never_returns || returns.size() == 0)
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front());
    }

    s << return_value_type
      << " " << parent_interface << "_closure::" << name_ << "(";

    bool first = true;
    std::for_each(params.begin(), params.end(), [&s, &first](parameter_t* param)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_cpp(s);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s, &first](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_cpp(s);
        });
    }

    s << ")" << std::endl
      << "{" << std::endl
      << '\t';
    if (return_value_type != "void")
        s << "return ";
    s << "methods->" << name_ << "(this";

    std::for_each(params.begin(), params.end(), [&s](parameter_t* param)
    {
        s << ", ";
        s << param->name_;
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s](parameter_t* param)
        {
            s << ", ";
            s << param->name_;
        });
    }

    s << ");" << std::endl
      << "}" << std::endl << std::endl;
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
    s << emit_type(*this);
    s << " " << name_;
}

void var_decl_t::emit_interface_h(std::ostringstream& s)
{
    s << emit_type(*this);
    s << " " << name_;
}

void var_decl_t::emit_interface_cpp(std::ostringstream& s)
{
    s << emit_type(*this);
    s << " " << name_;
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
