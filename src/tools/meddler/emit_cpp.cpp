//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "ast.h"
#include "macros.h"
#include "logger.h"
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <openssl/md5.h> // for type_code

using namespace std;

namespace AST
{

/**
 * Convert current interface into a string similar to
 * interface_name{parent_interface{}struct_name{field_type1;field_type2};method_signature(in_type1;in_type2):(out_type1;out_type2);}
 * and generate a fingerprint code from it.
 *
 * We use a classic Nemesis style generation by calculating md5sum, then reducing it from 128 to 48 bit. Low 16 bit are left for (??) information.
 */
static uint64_t generate_fingerprint(interface_t* intf)
{
    std::ostringstream out;

    intf->typecode_representation(out);

    L(cout << "Typecode representation for interface is:" << endl << out.str() << endl;);

    // Calculate md5
    std::string str = out.str();
    unsigned char hash[MD5_DIGEST_LENGTH];
    assert(MD5_DIGEST_LENGTH == 16);

    MD5((const unsigned char*)str.c_str(), str.length(), hash);

    // Reduce to 48 bit
    // there's 16 bits of overlap to use up; we put 8 bits at each end

    hash[0] ^= hash[5];
    hash[1] ^= hash[6];
    hash[2] ^= hash[7];
    hash[3] ^= hash[8];
    hash[4] ^= hash[9];
    hash[5] ^= hash[10];

    hash[0] ^= hash[10];
    hash[1] ^= hash[11];
    hash[2] ^= hash[12];
    hash[3] ^= hash[13];
    hash[4] ^= hash[14];
    hash[5] ^= hash[15];

    /** 
     * Abuse a trick from fourcc Magic64BE to generate a type code :)
     */
    uint64_t type_code = ((((((((((((((uint64_t)hash[0] << 8) | hash[1]) << 8) | hash[2]) << 8) | hash[3]) << 8) | hash[4]) << 8) | hash[5]) << 8) | 0) << 8) | 0;

    return type_code;
}

/**
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

/**
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

    for (auto m : intf->methods)
    {
        for (auto param : m->params)
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
        }

        for (auto param : m->returns)
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
        }
    }

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

/**
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

/**
 * Generate a qualified name to use as a type code prefix.
 */
static std::string emit_type_code_prefix(alias_t& type)
{
    std::string result = type.type();
    if (type.is_builtin_type())
    {
        L(cout << "EMITTING BUILTIN TYPE " << result << endl);
        // Check that mapping built-in type would actually work.
        result = map_type(type.unqualified_name());
        L(cout << " AS " << result << endl);
        if (result.empty())
        {
            cerr << "Error: Unknown mapping for builtin type " << type.type() << endl;
            result = type.type();
        }
        else
            result = type.unqualified_name() + "_"; // Actually assign unmapped, unqualified type name.
    }
    else
    if (type.is_interface_reference())
    {
        result = type.unqualified_name() + "::";
        L(cout << "EMITTING INTERFACE REFERENCE " << result << endl);
    }
    else
    if (type.is_local_type())
    {
        result = replace_dots(type.type());
        result = type.get_root()->name() + "::" + result + "_";
        L(cout << "EMITTING LOCAL TYPE " << result << endl);
    }
    else
    {
        result = replace_dots(type.type()) + "_";
        L(cout << "EMITTING EXTERNAL QUALIFIED TYPE: " << result << endl);
    }

    return result;
}

/**
 * Emit all necessary includes. If @c include_interface_refs is true then include interface-only references (not suitable 
 * for header files, as it creates cyclic dependencies).
 * @returns List of output includes.
 */
static std::vector<std::string> emit_includes(std::ostringstream& s, std::string indent_prefix, interface_t* interface, bool include_interface_refs)
{
    std::vector<std::string> interface_included;

    for (auto t : interface->imported_types)
    {
        bool include_reference = include_interface_refs ? true : !t->is_interface_reference();

        // @todo not_found algorithm?
        if (include_reference && (std::find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
        {
            t->emit_include(s, indent_prefix);
            interface_included.push_back(t->base_name());
        }
    }

    L(cout << "### ==== BASE is  === " << interface->base << endl);

    // Include parent interfaces.
    interface_t* parents = interface;
    while (parents->base != "")
    {
        L(cout << "### Including base interface " << parents->base << endl);
        s << indent_prefix << "#include \"" << parents->base << "_interface.h\"" << std::endl;
        parents = parents->parent;
    }

    for (auto t : interface->types)
        t->emit_include(s, indent_prefix);

    return interface_included;
}

//=====================================================================================================================
// interface_t
//=====================================================================================================================

void interface_t::emit_methods_impl_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    // First, emit parent interfaces methods, starting from the deepest parent.
    if (parent)
        parent->emit_methods_impl_h(s, indent_prefix, true);

    // Then, emit own methods.
    for (auto m : methods)
    {
        m->emit_impl_h(s, indent_prefix + "    ", fully_qualify_types);
    }
}

void interface_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << std::endl << std::endl;

    //FIXME: also use forwards from the emit_interface_h? to be self-sufficient header...
    std::vector<std::string> fwd = build_forwards(this);
    if (fwd.size() > 0)
    {
        for (auto str : fwd)
        {
            s << indent_prefix << "namespace " << str << " { struct closure_t; }" << endl;
        }
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
    for (auto m : methods)
    {
        m->emit_interface_h(s, indent_prefix + "        ", fully_qualify_types);
    }
}

void interface_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << std::endl << std::endl
      << indent_prefix << "#include \"module_interface.h\"" << std::endl;

    // Includes.
    // Do not include interface-only references, as it creates cyclic dependencies.
    // Instead, make a forward declaration down below..
    std::vector<std::string> interface_included = emit_includes(s, indent_prefix, this, false);

    s << std::endl;

    // Forward declarations.
    for (auto t : imported_types)
    {
        // @todo not_found algorithm if doesn't exist?
        if (t->is_interface_reference() && (std::find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
        {
            s << indent_prefix << "namespace " << t->base_name() << " { struct closure_t; }" << endl;
        }
    }

    s << std::endl;

    // Closure.
    
    // This structure acts as a namespace, limiting the scope of all declarations.
    s << indent_prefix << "namespace " << name() << std::endl
      << indent_prefix << "{" << std::endl;

    // Type declarations.
    for (auto t : types)
    {
        t->emit_interface_h(s, indent_prefix + "    ");
        s << std::endl;
    }
    
    s << std::endl;

    s << indent_prefix << "    struct ops_t;   // defined in " << name() << "_impl.h" << std::endl
      << indent_prefix << "    struct state_t; // opaque" << std::endl << endl;

    s << indent_prefix << "    struct closure_t {" << std::endl
      << indent_prefix << "        const " << name() << "::ops_t* d_methods;" << std::endl
      << indent_prefix << "        " << name() << "::state_t* d_state;" << std::endl << std::endl;

    // Methods (part of closure_t).
    emit_methods_interface_h(s, indent_prefix);

    s << indent_prefix << "    };" << std::endl;

    s << std::endl;

    // Type codes.
    uint64_t fp = generate_fingerprint(this);
    s << indent_prefix << "    const uint64_t type_code = 0x" << hex << fp << "ull;" << endl;

    int index = 0;
    for (auto t : types)
    {
        ++index;
        if (index > 0xffff)
        {
            s << "#error This interface has too many subtypes, cannot generate type codes. Please split this interface into smaller chunks." << endl;
            break;
        }
        s << indent_prefix << "    const uint64_t " << t->name() << "_type_code = 0x" << hex << fp + index << "ull;" << endl;
    }

    s << indent_prefix << "}" << std::endl;

}

void interface_t::emit_methods_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    // First, emit parent interfaces methods, starting from the deepest parent.
    if (parent)
        parent->emit_methods_interface_cpp(s, indent_prefix, true);

    // Then, emit own methods.
    for (auto m : methods)
    {
        m->emit_interface_cpp(s, indent_prefix, fully_qualify_types);
    }
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

void interface_t::typecode_representation(std::ostringstream& s)
{
    s << name() << "{";

    if (parent)
        parent->typecode_representation(s);

    // for (auto t : types)
    //     t->typecode_representation(s);

    // for (auto e : exceptions)
    //     e->typecode_representation(s);

    for (auto m : methods)
        m->typecode_representation(s);

    s << "}";
}

int interface_t::renumber_methods()
{
    int last_method = 0;
    if (parent)
        last_method = parent->renumber_methods();

    for (auto m : methods)
    {
        m->method_number = last_method;
        ++last_method;
    }

    return last_method;
}

void interface_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "#include \"interface_v1_state.h\"" << endl
      << indent_prefix << "#include \"" << name() << "_interface.h\"" << std::endl
      << indent_prefix << "#include \"" << name() << "_impl.h\"" << std::endl;

    emit_includes(s, indent_prefix, this, true);

    s << endl;

    // Emit forward declaration.
    s << indent_prefix << "extern interface_v1::state_t " << name() << "__intf_typeinfo;" << endl;

    // Emit types, exceptions and operations type defs.
    for (auto t : types)
        t->emit_typedef_cpp(s, indent_prefix, fully_qualify_types);

    for (auto e : exceptions)
        e->emit_typedef_cpp(s, indent_prefix, fully_qualify_types);

    for (auto m : methods)
        m->emit_typedef_cpp(s, indent_prefix, fully_qualify_types);

    // Now emit the interface typedef itself.
    s << indent_prefix << "//=====================================================================================================================" << endl
      << indent_prefix << "// Interface: " << name() << endl
      << indent_prefix << "//=====================================================================================================================" << endl
      << endl;

    if (methods.size() > 0)
    {
        s << indent_prefix << "static operation_t* " << name() << "_methods[] = {" << endl;
        for (auto m : methods)
            s << indent_prefix << "    &" << m->name() << "_method," << endl;
        s << indent_prefix << "    NULL" << endl;
        s << indent_prefix << "};" << endl;
    }

    s << indent_prefix << "interface_v1::state_t " << name() << "__intf_typeinfo {};" << endl;
}

//=====================================================================================================================
// method_t
//=====================================================================================================================

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

    for (auto param : params)
    {
        s << ", ";
        param->emit_impl_h(s, "", fully_qualify_types);
    }

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
    for (auto param : params)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_h(s, "", fully_qualify_types);
    }

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
    for (auto param : params)
    {
        if (!first)
            s << ", ";
        else
            first = false;
        param->emit_interface_cpp(s, "", fully_qualify_types);
    }

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

    for (auto param : params)
    {
        s << ", ";
		s << param->name();
    }

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

void method_t::typecode_representation(std::ostringstream& s)
{
    s << name() << "(";
    for (auto param : params)
    {
        s << param->type() << ";";
    }
    for (auto ret : returns)
    {
        s << ret->type() << ";";
    }
// std::vector<exception_t*> raises;
// std::vector<std::string>  raises_ids;
    if (idempotent)
        s << "idempotent;";
    if (never_returns)
        s << "never_returns;";
    s << ");";
}

void method_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "//=====================================================================================================================" << endl
      << indent_prefix << "// Operation: " << name() << endl
      << indent_prefix << "//=====================================================================================================================" << endl
      << endl;

    size_t n_params = params.size() + returns.size();

    if (n_params > 0)
    {
        bool first = true;
        s << indent_prefix << "static param_t " << name() << "_params[] = {" << endl;
        for (auto param : params)
        {
            if (first)
                first = false;
            else
                s << "," << endl;

            // describe every param
            param->emit_typedef_cpp(s, indent_prefix + "    ", fully_qualify_types);
        }
        for (auto ret : returns)
        {
            if (first)
                first = false;
            else
                s << "," << endl;

            // describe every return
            ret->emit_typedef_cpp(s, indent_prefix + "    ", fully_qualify_types);
        }
        s << endl << indent_prefix << "};" << endl << endl;
    }

    // now emit the actual operation description
    s << indent_prefix << "static operation_t " << name() << "_method = {" << endl
      << indent_prefix << "    \"" << name() << "\", /* Name */" << endl
      << indent_prefix << "    operation_v1::kind_proc, /* Kind */" << endl;
    if (n_params > 0)
    {
        s << indent_prefix << "    " << name() << "_params, /* Parameter list */" << endl;
    }
    else
    {
        s << indent_prefix << "    NULL, /* Parameter list */" << endl;
    }
    s << indent_prefix << "    " << n_params << ", /* Number of arguments */" << endl;
    s << indent_prefix << "    " << returns.size() << ", /* Number of return values */" << endl;
    s << indent_prefix << "    " << method_number << ", /* Operation index */" << endl;
    s << indent_prefix << "    NULL, /* Array of exceptions */" << endl;
    s << indent_prefix << "    0, /* Number of exceptions */" << endl;
    s << indent_prefix << "    NULL /* Closure for operation */" << endl;
    s << indent_prefix << "};" << endl << endl;
}

//=====================================================================================================================
// exception_t
//=====================================================================================================================
//
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

void exception_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "//=====================================================================================================================" << endl
      << indent_prefix << "// Exception: " << name() << endl
      << indent_prefix << "//=====================================================================================================================" << endl
      << endl;
}

//=====================================================================================================================
// alias_t
//=====================================================================================================================

void alias_t::emit_include(std::ostringstream& s, std::string indent_prefix)
{
    // UWAGA: Very ad-hoc patch, please rework this for real includes.
    cout << "Emitting include for base alias_t: " << base_name() << endl;
    if (map_type(base_name()).empty())
    	s << indent_prefix << "#include \"" << base_name() << "_interface.h\"" << std::endl;
    else
        s << indent_prefix << "#include \"types.h\"" << std::endl;
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

void alias_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "// emitting some alias here, too...." << name() << endl;
    // should not call this once all subtypes are implemented...
}

//=====================================================================================================================
// parameter_t
//=====================================================================================================================

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

void parameter_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    const char* directions[4] = {"input", "output", "in_out", "result"};

    s << indent_prefix << "{ { " << emit_type_code_prefix(*this) << "type_code, operation_v1::param_mode_" << directions[direction] << " }, \"" << name() << "\" }";
}

//=====================================================================================================================
// type_alias_t
//=====================================================================================================================

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

//=====================================================================================================================
// sequence_alias_t
//=====================================================================================================================

void sequence_alias_t::emit_include(std::ostringstream& s, std::string indent_prefix)
{
    s << indent_prefix << "#include <vector>" << std::endl;
    s << indent_prefix << "#include \"heap_allocator.h\"" << std::endl;
}

void sequence_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void sequence_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "typedef std::vector<" << emit_type(*this, true) << ", std::heap_allocator<" << emit_type(*this, true) << ">> " << replace_dots(name()) << ";" << endl;
}

void sequence_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

//=====================================================================================================================
// array_alias_t
//=====================================================================================================================

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

//=====================================================================================================================
// set_alias_t
//=====================================================================================================================

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

//=====================================================================================================================
// record_alias_t
//=====================================================================================================================

void record_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void record_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
	s << indent_prefix << "struct " << replace_dots(name()) << endl
	  << indent_prefix << "{" << endl;

    for (auto field : fields)
    {
        field->emit_interface_h(s, indent_prefix + "    ");
        s << ";" << endl;
    }

    s << indent_prefix << "};" << endl;
}

void record_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

//=====================================================================================================================
// enum_alias_t
//=====================================================================================================================

void enum_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool)
{
}

void enum_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool)
{
    s << indent_prefix << "enum " << replace_dots(name()) << endl
      << indent_prefix << "{" << endl;

    for (auto field : fields)
    {
        s << indent_prefix << "    " << replace_dots(this->name() + "_" + field) << "," << endl;
    }

    s << indent_prefix << "};" << endl;
}

void enum_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool)
{
}

//=====================================================================================================================
// range_alias_t
//=====================================================================================================================

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

} // namespace AST
