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

// Standard string is blergh!
static std::string replace_dots(std::string input)
{
	size_t pos;
	while ((pos = input.find(".")) != std::string::npos)
	{
		input.replace(input.begin()+pos, input.begin()+pos+1, "_");
	}
	return input;
}

/*!
 * Generate a qualified name for a given var decl type.
 */
static std::string emit_type(alias_t& type)
{
	cout << "** EMITTING TYPE ** "; type.dump("");

    std::string result = type.type();
    if (type.is_builtin_type())
    {
		cout << "EMITTING BUILTIN TYPE " << result << endl;
        result = map_type(type.unqualified_name());
		cout << " AS " << result << endl;
        if (result.empty())
        {
			cout << "Error: Unknown mapping for builtin type " << type.type() << endl;
			result = type.type();
		}
	}
	else
	if (type.is_interface_reference())
	{
		result = type.unqualified_name(); // we need first part of the name?!?
		cout << "EMITTING INTERFACE REFERENCE " << result << endl;
	}
	else
	if (type.is_local_type())
	{
		result = replace_dots(type.get_root()->name() + "." + type.type());
		cout << "EMITTING LOCAL TYPE " << result << endl;
	}
	else
	{
		result = replace_dots(type.type());
		cout << "EMITTING EXTERNAL QUALIFIED TYPE: " << result << endl;
    }

	if (type.is_interface_reference())
		result += "_closure*";
	else
    	if (type.is_reference())
        	result += "&";

    return result;
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

    if (methods.size() > 0)
    {
        s << "// ops structure should be exposed to module implementors!" << std::endl;
        s << "struct " << name() << "_ops" << std::endl
          << "{" << std::endl;

        std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
        {
            m->emit_impl_h(s);
        });

        s << "};" << std::endl;
    }
}

void interface_t::emit_interface_h(std::ostringstream& s)
{
    s << "#pragma once" << std::endl << std::endl
	  << "#include \"module_interface.h\"" << std::endl;
	
	std::for_each(imported_types.begin(), imported_types.end(), [&s](alias_t* t)
	{
		t->emit_include(s);
		s << std::endl;
	});

	s << std::endl;
	
	std::for_each(types.begin(), types.end(), [&s](alias_t* t)
    {
        t->emit_interface_h(s);
		s << std::endl;
    });
    
	s << std::endl;
//	if (methods.size() > 0)
	{
        s << "struct " << name() << "_ops;" << std::endl
          << "struct " << name() << "_state;" << std::endl << std::endl
          << "struct " << name() << "_closure" << std::endl
          << "{" << std::endl
          << '\t' << "const " << name() << "_ops* methods;" << std::endl
          << '\t' << name() << "_state* state;" << std::endl << std::endl;

        std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
        {
            m->emit_interface_h(s);
        });

        s << "};" << std::endl;
    }
}

// Currently no need to generate interface.cpp if there are no methods.
void interface_t::emit_interface_cpp(std::ostringstream& s)
{
	if (methods.size() > 0)
	{
        s << "#include \"" << name() << "_interface.h\"" << std::endl
          << "#include \"" << name() << "_impl.h\"" << std::endl << std::endl;

        std::for_each(methods.begin(), methods.end(), [&s](method_t* m)
        {
            m->emit_interface_cpp(s);
        });
    }
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
      << " (*" << name() << ")(";

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
      << " " << name() << "(";

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
      << " " << parent_interface << "_closure::" << name() << "(";

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
    s << "methods->" << name() << "(this";

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
      << "}" << std::endl << std::endl;
}

void exception_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void exception_t::emit_interface_h(std::ostringstream& s)
{
    s << "typedef int xcp_" << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void exception_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void alias_t::emit_include(std::ostringstream& s)
{
	std::string base = name().substr(0, name().find_first_of('.'));
	s << "#include \"" << base << "_interface.h\"";
}

void alias_t::emit_impl_h(std::ostringstream& s)
{
    s << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_h(std::ostringstream& s)
{
    s << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_cpp(std::ostringstream& s)
{
    s << emit_type(*this);
    s << " " << name();
}

void parameter_t::emit_impl_h(std::ostringstream& s)
{
    s << emit_type(*this);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_h(std::ostringstream& s)
{
    s << emit_type(*this);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_cpp(std::ostringstream& s)
{
    s << emit_type(*this);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void type_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void type_alias_t::emit_interface_h(std::ostringstream& s)
{
	s << "typedef " << emit_type(*this) << " " << replace_dots(get_root()->name() + "." + name()) << ";";
}

void type_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void sequence_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void sequence_alias_t::emit_interface_h(std::ostringstream& s)
{
    s << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void sequence_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void array_alias_t::emit_impl_h(std::ostringstream& s)
{
    s << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
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

void set_alias_t::emit_interface_h(std::ostringstream& s)
{
    s << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void set_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void record_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void record_alias_t::emit_interface_h(std::ostringstream& s)
{
	s << "struct " << replace_dots(get_root()->name() + "." + name()) << endl
	  << "{" << endl;
    std::for_each(fields.begin(), fields.end(), [&s](alias_t* field)
    {
        s << '\t';
        field->emit_interface_h(s);
        s << ";" << endl;
    });
    s << "};" << endl;
}

void record_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void enum_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void enum_alias_t::emit_interface_h(std::ostringstream& s)
{
    s << "enum " << replace_dots(get_root()->name() + "." + name()) << " {" << endl;
    std::for_each(fields.begin(), fields.end(), [&s, this](std::string field)
    {
        s << '\t';
        s << replace_dots(this->get_root()->name() + "." + this->name() + "." + field) << "," << endl;
    });
    s << "};" << endl;
}

void enum_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

void range_alias_t::emit_impl_h(std::ostringstream& s UNUSED_ARG)
{
}

void range_alias_t::emit_interface_h(std::ostringstream& s)
{
    s << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void range_alias_t::emit_interface_cpp(std::ostringstream& s UNUSED_ARG)
{
}

}
