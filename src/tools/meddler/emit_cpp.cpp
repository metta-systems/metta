//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "ast.h"
#include "macros.h"
#include "logger.h"
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

namespace AST
{

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
        type_map["opaque"] = "void*";
        type_map_built = true;
    }

    if (type_map.find(type) != type_map.end())
        return type_map[type];
    else
        return string();
}

/*!
 * If a given type needs include directive, return one, otherwise return empty string.
 */
/*static std::string needs_include(string type)
{
	if (map_type(type).empty())
	{
		return string("#include \"")+type+"_interface.h\"";
	}
	return string();
}*/

static std::vector<std::string> build_forwards(interface_t* intf)
{
    std::vector<std::string> forwards;

    forwards.push_back(intf->name());

    std::for_each(intf->methods.begin(), intf->methods.end(), [&forwards](method_t* m)
    {
        std::for_each(m->params.begin(), m->params.end(), [&forwards](parameter_t* param)
        {
            size_t pos;
            if ((pos = param->type().find_first_of('.')) != std::string::npos) //FIXME: is_qualified_name()
            {
                std::string decl = param->type().substr(0, pos);
                if (std::find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        });

        std::for_each(m->returns.begin(), m->returns.end(), [&forwards](parameter_t* param)
        {
            size_t pos;
            if ((pos = param->type().find_first_of('.')) != std::string::npos) //FIXME: is_qualified_name()
            {
                std::string decl = param->type().substr(0, pos);
                if (std::find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        });
    });

    return forwards;
}

// Replace dots in the name with scope operator (:: for C++)
static std::string replace_dots(std::string input)
{
    size_t pos;
    while ((pos = input.find(".")) != std::string::npos)
    {
        auto begin = input.begin() + pos;
        input.replace(begin, begin + 1, "::");
    }
    return input;
}

/*!
 * Generate a qualified name for a given var decl type.
 */
static std::string emit_type(alias_t& type, bool fully_qualify_type = false)
{
	L(cout << "** EMITTING TYPE ** "; type.dump(""));

    std::string result = type.type();
    if (type.is_builtin_type())
    {
		L(cout << "EMITTING BUILTIN TYPE " << result << endl);
        result = map_type(type.unqualified_name());
		L(cout << " AS " << result << endl);
        if (result.empty())
        {
			cerr << "Error: Unknown mapping for builtin type " << type.type() << endl;
			result = type.type();
		}
	}
	else
	if (type.is_interface_reference())
	{
		result = type.unqualified_name(); // we need first part of the name?!?
		L(cout << "EMITTING INTERFACE REFERENCE " << result << endl);
	}
	else
	if (type.is_local_type())
	{
		result = replace_dots(type.type());
        if (fully_qualify_type)
            result = type.get_root()->name() + "::" + result;
		L(cout << "EMITTING LOCAL TYPE " << result << endl);
	}
	else
	{
		result = replace_dots(type.type());
		L(cout << "EMITTING EXTERNAL QUALIFIED TYPE: " << result << endl);
    }

	if (type.is_interface_reference())
		result += "::closure_t*";
	else
    	if (type.is_reference())
        	result += "&";

    return result;
}

void interface_t::emit_methods_impl_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    // First, emit parent interfaces methods, starting from the deepest parent.
    if (parent)
        parent->emit_methods_impl_h(s, indent_prefix, true);

    // Then, emit own methods.
    std::for_each(methods.begin(), methods.end(), [&s, indent_prefix, fully_qualify_types](method_t* m)
    {
        m->emit_impl_h(s, indent_prefix + "    ", fully_qualify_types);
    });
}

void interface_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << std::endl << std::endl;

    //FIXME: also use forwards from the emit_interface_h? to be self-sufficient header...
    std::vector<std::string> fwd = build_forwards(this);
    if (fwd.size() > 0)
    {
        std::for_each(fwd.begin(), fwd.end(), [&s, indent_prefix](std::string str)
        {
            s << indent_prefix << "namespace " << str << " { struct closure_t; }" << endl;
        });
        s << std::endl;
    }

    if (methods.size() > 0)
    {
        s << indent_prefix << "// ops structure should be exposed to module implementors!" << std::endl;
        s << indent_prefix << "namespace " << name() << std::endl
          << indent_prefix << "{" << std::endl
          << indent_prefix << "    struct ops_t" << std::endl
          << indent_prefix << "    {" << std::endl;

        emit_methods_impl_h(s, indent_prefix);

        s << indent_prefix << "    };" << std::endl
          << indent_prefix << "}" << std::endl;
    }
}

void interface_t::emit_methods_interface_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    // First, emit parent interfaces methods, starting from the deepest parent.
    if (parent)
        parent->emit_methods_interface_h(s, indent_prefix, true);

    // Then, emit own methods.
    std::for_each(methods.begin(), methods.end(), [&s, indent_prefix, fully_qualify_types](method_t* m)
    {
        m->emit_interface_h(s, indent_prefix + "        ", fully_qualify_types);
    });
}

void interface_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << std::endl << std::endl
      << indent_prefix << "#include \"module_interface.h\"" << std::endl;

    // Includes.
    // Do not include interface-only references, as it creates cyclic dependencies.
    // Instead, make a forward declaration down below..
    std::vector<std::string> interface_included;

    std::for_each(imported_types.begin(), imported_types.end(), [&s, indent_prefix, &interface_included](alias_t* t)
    {
        if (!t->is_interface_reference() && (std::find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
        {
            t->emit_include(s, indent_prefix);
            interface_included.push_back(t->base_name());
        }
    });

    cout << "### ==== BASE is  === " << base << endl;

    // Include parent interfaces.
    interface_t* parents = this;
    while (parents->base != "")
    {
        cout << "### Including base interface " << parents->base << endl;
        s << indent_prefix << "#include \"" << parents->base << "_interface.h\"" << std::endl;
        parents = parents->parent;
    }

    std::for_each(types.begin(), types.end(), [&s, indent_prefix](alias_t* t)
    {
        t->emit_include(s, indent_prefix);
    });

    s << std::endl;

    // Forward declarations.
    std::for_each(imported_types.begin(), imported_types.end(), [&s, indent_prefix, &interface_included](alias_t* t)
    {
        if (t->is_interface_reference() && (std::find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
        {
            s << indent_prefix << "namespace " << t->base_name() << " { struct closure_t; }" << endl;
        }
    });

    s << std::endl;

    // Closure.
    
    // This structure acts as a namespace, limiting the scope of all declarations.
    s << indent_prefix << "namespace " << name() << std::endl
      << indent_prefix << "{" << std::endl;

    // Type declarations.
    std::for_each(types.begin(), types.end(), [&s, indent_prefix](alias_t* t)
    {
        t->emit_interface_h(s, indent_prefix + "    ");
        s << std::endl;
    });
    
    s << std::endl;

    s << indent_prefix << "    struct ops_t;   // defined in " << name() << "_impl.h" << std::endl
      << indent_prefix << "    struct state_t; // opaque" << std::endl << endl;

    s << indent_prefix << "    struct closure_t {" << std::endl
      << indent_prefix << "        const " << name() << "::ops_t* d_methods;" << std::endl
      << indent_prefix << "        " << name() << "::state_t* d_state;" << std::endl << std::endl;

    // Methods (part of closure_t).
    emit_methods_interface_h(s, indent_prefix);

    s << indent_prefix << "    };" << std::endl;

    s << indent_prefix << "}" << std::endl;

}

void interface_t::emit_methods_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    // First, emit parent interfaces methods, starting from the deepest parent.
    if (parent)
        parent->emit_methods_interface_cpp(s, indent_prefix, true);

    // Then, emit own methods.
    std::for_each(methods.begin(), methods.end(), [&s, indent_prefix, fully_qualify_types](method_t* m)
    {
        m->emit_interface_cpp(s, indent_prefix, fully_qualify_types);
    });
}

// Currently no need to generate interface.cpp if there are no methods.
void interface_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
    if (methods.size() > 0)
    {
        s << indent_prefix << "#include \"" << name() << "_interface.h\"" << std::endl
          << indent_prefix << "#include \"" << name() << "_impl.h\"" << std::endl << std::endl;

        s << indent_prefix << "namespace " << name() << std::endl
          << indent_prefix << "{" << endl << endl;

        emit_methods_interface_cpp(s, indent_prefix);

        s << indent_prefix << "}" << endl;
    }
}

void method_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    std::string return_value_type;
    if (never_returns || (returns.size() == 0))
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front(), fully_qualify_types);
    }

    s << indent_prefix << return_value_type
      << " (*" << name() << ")(";

    s << parent_interface << "::closure_t* self";

    std::for_each(params.begin(), params.end(), [&s, fully_qualify_types](parameter_t* param)
    {
        s << ", ";
        param->emit_impl_h(s, "", fully_qualify_types);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s, fully_qualify_types](parameter_t* param)
        {
            s << ", ";
            param->emit_impl_h(s, "", fully_qualify_types);
        });
    }

    s << ");" << std::endl;
}

void method_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    std::string return_value_type;
    if (never_returns || returns.size() == 0)
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front(), fully_qualify_types);
    }

    s << indent_prefix << return_value_type
      << " " << name() << "(";

    bool first = true;
    std::for_each(params.begin(), params.end(), [&s, &first, fully_qualify_types](parameter_t* param)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_h(s, "", fully_qualify_types);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s, &first, fully_qualify_types](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_h(s, "", fully_qualify_types);
        });
    }

    s << ");" << std::endl;
}

void method_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    std::string return_value_type;
    if (never_returns || returns.size() == 0)
        return_value_type = "void";
    else
    {
        return_value_type = emit_type(*returns.front(), fully_qualify_types);
    }

    s << indent_prefix << return_value_type << "  closure_t::" << name() << "(";

    bool first = true;
    std::for_each(params.begin(), params.end(), [&s, &first, fully_qualify_types](parameter_t* param)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_cpp(s, "", fully_qualify_types);
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s, &first, fully_qualify_types](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_cpp(s, "", fully_qualify_types);
        });
    }

    s << ")" << std::endl
      << indent_prefix << "{" << std::endl
      << indent_prefix << "    ";
    if (return_value_type != "void")
        s << "return ";
    s << "d_methods->" << name() << "(";

    s << "reinterpret_cast<" << get_root()->name() << "::closure_t*>(this)"; // butt-ugly, indeed

    std::for_each(params.begin(), params.end(), [&s](parameter_t* param)
    {
        s << ", ";
		s << param->name();
    });

    // TODO: add by-ptr for non-interface returns
    if (returns.size() > 1)
    {
        std::for_each(returns.begin()+1, returns.end(), [&s](parameter_t* param)
        {
            s << ", ";
            s << param->name();
        });
    }

    s << ");" << std::endl
      << indent_prefix << "}" << std::endl << std::endl;
}

// TODO:
// Once exception support in the kernel is established,
// emit exceptions into a nested namespace under "name_v1" i.e.
// namespace name_v1 { namespace exceptions { class xcp1; class xcp2; } }
// FQN would be name_v1::exceptions::no_memory for example.
// Other variant is to emit them directly in name_v1, this is shorter but
// less clean I think.
// "raises" corresponds directly to C++ throw() method specification - but
// it should not be used, see http://stackoverflow.com/questions/88573/should-i-use-an-exception-specifier-in-c
// for explanation why. Also http://www.gotw.ca/publications/mill22.htm
void exception_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void exception_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef int xcp_" << replace_dots(name()) << ";" << endl; //TEMP hack
}

void exception_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void alias_t::emit_include(std::ostringstream& s, std::string indent_prefix)
{
	s << indent_prefix << "#include \"" << base_name() << "_interface.h\"" << std::endl;
}

void alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void parameter_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void type_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void type_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
	s << indent_prefix << "typedef " << emit_type(*this) << " " << replace_dots(name()) << ";";
}

void type_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void sequence_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void sequence_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef int " << replace_dots(name()) << ";" << endl; //TEMP hack
}

void sequence_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void array_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void array_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void array_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void set_alias_t::emit_include(std::ostringstream& s, std::string indent_prefix)
{
	s << indent_prefix << "#include \"set_t.h\"" << std::endl;
}

void set_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void set_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef set_t<" << /*FIXME:? emit_*/type() << "> " << name() << ";" << endl;
}

void set_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void record_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void record_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
	s << indent_prefix << "struct " << replace_dots(name()) << endl
	  << indent_prefix << "{" << endl;
    std::for_each(fields.begin(), fields.end(), [&s, indent_prefix](alias_t* field)
    {
        field->emit_interface_h(s, indent_prefix + "    ");
        s << ";" << endl;
    });
    s << indent_prefix << "};" << endl;
}

void record_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void enum_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void enum_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "enum " << replace_dots(name()) << endl
      << indent_prefix << "{" << endl;
    std::for_each(fields.begin(), fields.end(), [&s, indent_prefix, this](std::string field)
    {
        s << indent_prefix << "    " << replace_dots(this->name() + "_" + field) << "," << endl;
    });
    s << indent_prefix << "};" << endl;
}

void enum_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void range_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void range_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef int " << replace_dots(name()) << ";" << endl; //TEMP hack
}

void range_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
