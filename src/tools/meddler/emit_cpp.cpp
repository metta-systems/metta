//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
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
    ostringstream out;

    intf->typecode_representation(out);

    L(cout << "Typecode representation for interface is:" << endl << out.str() << endl;);

    // Calculate md5
    string str = out.str();
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

/// @Return in escaped for use inside C string quotes.
static string string_escape(string in)
{
    ostringstream str;
    for (auto c : in)
    {
        switch (c)
        {
            case '"':
                str << "\\\"";
                break;
            case '\n':
                str << "\\n";
                break;
            case '\\':
                str << "\\\\";
                break;
            default:
                str << c;
        }
    }
    return str.str();
}

/**
 * If a given type needs include directive, return one, otherwise return empty string.
 */
/*static string needs_include(string type)
{
    if (map_type(type).empty())
    {
        return string("#include \"")+type+"_interface.h\"";
    }
    return string();
}*/

static vector<string> build_forwards(interface_t* intf)
{
    vector<string> forwards;

    forwards.push_back(intf->name());

    for (auto m : intf->methods)
    {
        for (auto param : m->params)
        {
            size_t pos;
            if ((pos = param->type().find_first_of('.')) != string::npos) //FIXME: is_qualified_name()
            {
                string decl = param->type().substr(0, pos);
                if (find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        }

        for (auto param : m->returns)
        {
            size_t pos;
            if ((pos = param->type().find_first_of('.')) != string::npos) //FIXME: is_qualified_name()
            {
                string decl = param->type().substr(0, pos);
                if (find(forwards.begin(), forwards.end(), decl) == forwards.end())
                {
                    forwards.push_back(decl);
                }
            }
        }
    }

    return forwards;
}

// Replace dots in the name with scope operator (:: for C++)
static string replace_dots(string input)
{
    size_t pos;
    while ((pos = input.find(".")) != string::npos)
    {
        auto begin = input.begin() + pos;
        input.replace(begin, begin + 1, "::");
    }
    return input;
}

/**
 * Generate a qualified name for a given var decl type.
 */
static string emit_type(alias_t& type, bool fully_qualify_type = false)
{
    L(cout << "** EMITTING TYPE ** "; type.dump(""));

    string result = type.type();
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
            result += "*";

    return result;
}

/**
 * Generate a qualified name to use as a type code prefix.
 */
static string emit_type_code_prefix(alias_t& type)
{
    string result = type.type();
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
static vector<string> emit_includes(ostringstream& s, string indent_prefix, interface_t* interface, bool include_interface_refs)
{
    vector<string> interface_included;

    for (auto t : interface->imported_types)
    {
        bool include_reference = include_interface_refs ? true : !t->is_interface_reference();

        // @todo not_found algorithm?
        if (include_reference && (find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
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
        s << indent_prefix << "#include \"" << parents->base << "_interface.h\"" << endl;
        parents = parents->parent;
    }

    for (auto t : interface->types)
        t->emit_include(s, indent_prefix);

    return interface_included;
}

//=====================================================================================================================
// interface_t
//=====================================================================================================================

void interface_t::emit_methods_impl_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
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

void interface_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << endl << endl;

    //FIXME: also use forwards from the emit_interface_h? to be self-sufficient header...
    vector<string> fwd = build_forwards(this);
    if (fwd.size() > 0)
    {
        for (auto str : fwd)
        {
            s << indent_prefix << "namespace " << str << " { struct closure_t; }" << endl;
        }
        s << endl;
    }

    if (methods.size() > 0)
    {
        s << indent_prefix << "// ops structure should be exposed to module implementors!" << endl;
        s << indent_prefix << "namespace " << name() << endl
          << indent_prefix << "{" << endl
          << indent_prefix << "    struct ops_t" << endl
          << indent_prefix << "    {" << endl;

        emit_methods_impl_h(s, indent_prefix);

        s << indent_prefix << "    };" << endl
          << indent_prefix << "}" << endl;
    }
}

void interface_t::emit_methods_interface_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
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

void interface_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "#pragma once" << endl << endl
      << indent_prefix << "#include \"module_interface.h\"" << endl;

    // @todo Special case for types::any:
    if (name() == "types")
    {
        s << indent_prefix << "#include \"any.h\"" << endl;
    }

    // Includes.
    // Do not include interface-only references, as it creates cyclic dependencies.
    // Instead, make a forward declaration down below..
    vector<string> interface_included = emit_includes(s, indent_prefix, this, false);

    s << endl;

    // Forward declarations.
    for (auto t : imported_types)
    {
        // @todo not_found algorithm if doesn't exist?
        if (t->is_interface_reference() && (find(interface_included.begin(), interface_included.end(), t->base_name()) == interface_included.end()))
        {
            s << indent_prefix << "namespace " << t->base_name() << " { struct closure_t; }" << endl;
        }
    }

    s << endl;

    // Closure.

    if (!get_autodoc().empty())
        s << indent_prefix << "/**\n" << get_autodoc() << "\n*/" << endl;

    // This structure acts as a namespace, limiting the scope of all declarations.
    s << indent_prefix << "namespace " << name() << endl
      << indent_prefix << "{" << endl;

    // Type declarations.
    for (auto t : types)
    {
        t->emit_interface_h(s, indent_prefix + "    ");
        s << endl;
    }

    s << endl;

    s << indent_prefix << "    struct ops_t;   // defined in " << name() << "_impl.h" << endl
      << indent_prefix << "    struct state_t; // opaque" << endl << endl;

    s << indent_prefix << "    struct closure_t {" << endl
      << indent_prefix << "        const " << name() << "::ops_t* d_methods;" << endl
      << indent_prefix << "        " << name() << "::state_t* d_state;" << endl << endl;

    // Methods (part of closure_t).
    emit_methods_interface_h(s, indent_prefix);

    s << indent_prefix << "    };" << endl;

    s << endl;

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

    s << indent_prefix << "}" << endl;

}

void interface_t::emit_methods_interface_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
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
void interface_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
    if (methods.size() > 0)
    {
        s << indent_prefix << "#include \"" << name() << "_interface.h\"" << endl
          << indent_prefix << "#include \"" << name() << "_impl.h\"" << endl << endl;

        s << indent_prefix << "namespace " << name() << endl
          << indent_prefix << "{" << endl << endl;

        emit_methods_interface_cpp(s, indent_prefix);

        s << indent_prefix << "}" << endl;
    }
}

void interface_t::typecode_representation(ostringstream& s)
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

void interface_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "#include \"interface_v1_state.h\"" << endl
      << indent_prefix << "#include \"type_system_v1_interface.h\"" << endl
      << indent_prefix << "#include \"record_v1_interface.h\"" << endl //@todo see record type emitter for info
      << indent_prefix << "#include \"choice_v1_interface.h\"" << endl //@todo see choice type emitter for info
      << indent_prefix << "#include \"enum_v1_interface.h\"" << endl //@todo see enum type emitter for info
      << indent_prefix << "#include \"" << name() << "_interface.h\"" << endl
      << indent_prefix << "#include \"" << name() << "_impl.h\"" << endl;

    emit_includes(s, indent_prefix, this, true);

    s << endl;

    // Emit forward declaration.
    s << indent_prefix << "extern interface_v1::state_t " << name() << "__intf_typeinfo;" << endl << endl;

    s << indent_prefix << "namespace { // start anon namespace" << endl << endl;

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

    if (types.size() > 0)
    {
        s << indent_prefix << "type_representation_t* " << name() << "_types[] = {" << endl;
        bool first = true;
        for (auto t : types)
        {
            if (first)
                first = false;
            else
                s << "," << endl;
            s << indent_prefix << "    &" << t->name() << "_type_rep";
        }
        s << endl
          << indent_prefix << "};" << endl << endl;
    }

    if (methods.size() > 0)
    {
        s << indent_prefix << "operation_t* " << name() << "_methods[] = {" << endl;
        bool first = true;
        for (auto m : methods)
        {
            if (first)
                first = false;
            else
                s << "," << endl;
            s << indent_prefix << "    &" << m->name() << "_method";
        }
        s << endl
          << indent_prefix << "};" << endl << endl;
    }

    // emit the closure
    s << indent_prefix << "interface_v1::closure_t " << name() << "_typeinfo_closure = {" << endl
      << indent_prefix << "    nullptr, // Will be patched to &interface_ops" << endl
      << indent_prefix << "    reinterpret_cast<interface_v1::state_t*>(&" << name() << "__intf_typeinfo)," << endl
      << indent_prefix << "};" << endl
      << endl;

    s << indent_prefix << "} // end anon namespace" << endl << endl;

    // Exported typeinfo.
    s << indent_prefix << "interface_v1::state_t " << name() << "__intf_typeinfo" << endl
      << indent_prefix << "{" << endl
      << indent_prefix << "    { // representation" << endl
      << indent_prefix << "        { type_system_v1::iref_type_code, { .ptr32value = &" << name() << "_typeinfo_closure } }, // any" << endl
      << indent_prefix << "        { types::code_type_code, { " << name() << "::type_code } }, // code" << endl
      << indent_prefix << "        \"" << name() << "\", // name" << endl
      << indent_prefix << "        \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "        nullptr, // Will be patched to meta_interface" << endl
      << indent_prefix << "        sizeof(" << name() << "::closure_t*)" << endl
      << indent_prefix << "    }, // end representation" << endl
      << indent_prefix << "    nullptr, // Needs" << endl
      << indent_prefix << "    0, // Number of needs" << endl
      << indent_prefix << "    "; if (types.size() > 0) s << name() << "_types"; else s << "nullptr"; s << ", // Types" << endl
      << indent_prefix << "    " << types.size() << ", // Number of types" << endl
      << indent_prefix << "    " << (local ? "true" : "false") << ", // Is this interface local?" << endl
      << indent_prefix << "    "; if (parent) s << parent->name() << "::type_code"; else s << "0"; s << ", // Supertype" << endl
      << indent_prefix << "    "; if (methods.size() > 0) s << name() << "_methods"; else s << "nullptr"; s << ", // Table of methods" << endl
      << indent_prefix << "    " << methods.size() << ", // Number of methods" << endl
      << indent_prefix << "    nullptr, // Exceptions" << endl
      << indent_prefix << "    0, // Number of exceptions" << endl
      << indent_prefix << "};" << endl << endl;
}

//=====================================================================================================================
// method_t
//=====================================================================================================================

void method_t::emit_impl_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string return_value_type;
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
        for_each(returns.begin()+1, returns.end(), [&s, fully_qualify_types](parameter_t* param)
        {
            s << ", ";
            param->emit_impl_h(s, "", fully_qualify_types);
        });
    }

    s << ");" << endl;
}

void method_t::emit_interface_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string return_value_type;
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
        for_each(returns.begin()+1, returns.end(), [&s, &first, fully_qualify_types](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_h(s, "", fully_qualify_types);
        });
    }

    s << ");" << endl;
}

void method_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string return_value_type;
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
        for_each(returns.begin()+1, returns.end(), [&s, &first, fully_qualify_types](parameter_t* param)
        {
            if (!first)
                s << ", ";
            else
                first = false;
            param->emit_interface_cpp(s, "", fully_qualify_types);
        });
    }

    s << ")" << endl
      << indent_prefix << "{" << endl
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
        for_each(returns.begin()+1, returns.end(), [&s](parameter_t* param)
        {
            s << ", ";
            s << param->name();
        });
    }

    s << ");" << endl
      << indent_prefix << "}" << endl << endl;
}

void method_t::typecode_representation(ostringstream& s)
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
// vector<exception_t*> raises;
// vector<string>  raises_ids;
    if (idempotent)
        s << "idempotent;";
    if (never_returns)
        s << "never_returns;";
    s << ");";
}

void method_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "//=====================================================================================================================" << endl
      << indent_prefix << "// Operation: " << name() << endl
      << indent_prefix << "//=====================================================================================================================" << endl
      << endl;

    size_t n_params = params.size() + returns.size();

    if (n_params > 0)
    {
        bool first = true;
        s << indent_prefix << "param_t " << name() << "_params[] = {" << endl;
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

    s << indent_prefix << "extern operation_t " << name() << "_method;" << endl << endl; // forward declaration

    // emit the closure
    s << indent_prefix << "operation_v1::closure_t " << name() << "_method_closure = {" << endl
      << indent_prefix << "    nullptr, /* Will be patched to &operation_ops */" << endl
      << indent_prefix << "    reinterpret_cast<operation_v1::state_t*>(&" << name() << "_method)," << endl
      << indent_prefix << "};" << endl
      << endl;

    // now emit the actual operation description
    s << indent_prefix << "operation_t " << name() << "_method = {" << endl
      << indent_prefix << "    \"" << name() << "\", /* Name */" << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    operation_v1::kind_proc, /* Kind */" << endl
      << indent_prefix << "    "; if (n_params > 0) s << name() << "_params"; else s << "nullptr"; s << ", /* Parameter list */" << endl
      << indent_prefix << "    " << params.size() << ", /* Number of arguments */" << endl
      << indent_prefix << "    " << returns.size() << ", /* Number of results */" << endl
      << indent_prefix << "    " << method_number << ", /* Operation index */" << endl
      << indent_prefix << "    nullptr, /* Array of exceptions */" << endl
      << indent_prefix << "    0, /* Number of exceptions */" << endl
      << indent_prefix << "    &" << name() << "_method_closure /* Closure for operation */" << endl
      << indent_prefix << "};" << endl << endl;
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
void exception_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void exception_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "typedef int xcp_" << replace_dots(name()) << ";" << endl; //TEMP hack
}

void exception_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void exception_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "//=====================================================================================================================" << endl
      << indent_prefix << "// Exception: " << name() << endl
      << indent_prefix << "//=====================================================================================================================" << endl
      << endl;
}

//=====================================================================================================================
// alias_t
//=====================================================================================================================

void alias_t::emit_include(ostringstream& s, string indent_prefix)
{
    // UWAGA: Very ad-hoc patch, please rework this for real includes.
    L(cout << "Emitting include for base alias_t: " << base_name() << endl);
    if (map_type(base_name()).empty())
        s << indent_prefix << "#include \"" << base_name() << "_interface.h\"" << endl;
    else
        s << indent_prefix << "#include \"types.h\"" << endl;
}

void alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << emit_type(*this);
    s << " " << name();
}

void alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << "#error alias_t::emit_typedef_cpp() is called." << endl;
}

//=====================================================================================================================
// parameter_t
//=====================================================================================================================

void parameter_t::emit_impl_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_h(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << emit_type(*this, fully_qualify_types);
    if (direction != in)
        s << "*";
    s << " " << name();
}

void parameter_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    const char* directions[4] = {"input", "output", "in_out", "result"};

    s << indent_prefix << "{ { " << emit_type_code_prefix(*this) << "type_code, operation_v1::param_mode_" << directions[direction] << " }, \"" << name() << "\", \"" << string_escape(get_autodoc()) << "\" }";
}

//=====================================================================================================================
// type_alias_t
//=====================================================================================================================

void type_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void type_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "typedef " << emit_type(*this) << " " << replace_dots(name()) << ";";
}

void type_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void type_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Alias: " << fqn << endl
      << indent_prefix << " */" << endl
      << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::alias_type_code, { " << emit_type_code_prefix(*this) << "type_code } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// sequence_alias_t
//=====================================================================================================================

void sequence_alias_t::emit_include(ostringstream& s, string indent_prefix)
{
    s << indent_prefix << "#include <vector>" << endl;
    s << indent_prefix << "#include \"heap_allocator.h\"" << endl;
}

void sequence_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void sequence_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "typedef std::vector<" << emit_type(*this, true) << ", std::heap_allocator<" << emit_type(*this, true) << ">> " << replace_dots(name()) << ";" << endl;
}

void sequence_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void sequence_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Sequence: " << fqn << endl
      << indent_prefix << " */" << endl
      << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::sequence_type_code, { " << emit_type_code_prefix(*this) << "type_code } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// array_alias_t
//=====================================================================================================================

void array_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "typedef int " << replace_dots(get_root()->name() + "." + name()) << ";" << endl; //TEMP hack
}

void array_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
}

void array_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void array_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "#error Should emit array alias here...." << name() << endl;
}

//=====================================================================================================================
// set_alias_t
//=====================================================================================================================

void set_alias_t::emit_include(ostringstream& s, string indent_prefix)
{
    s << indent_prefix << "#include \"set_t.h\"" << endl;
}

void set_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void set_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    // for bigger enums could potentially be expanded to array of uints..
    // s << indent_prefix << "struct " << name() << " { uint32_t value; };" << endl;
    s << indent_prefix << "typedef set_t<" << type() << "> " << name() << ";" << endl;
}

void set_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void set_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Set: " << fqn << endl
      << indent_prefix << " */" << endl
      << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::set_type_code, { " << emit_type_code_prefix(*this) << "type_code } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// record_alias_t
//=====================================================================================================================

void record_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void record_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    // @todo Special case for types::any:
    if (name() == "any" && get_root()->name() == "types")
    {
        s << indent_prefix << "typedef ::any any;" << endl;
        return;
    }

    s << indent_prefix << "struct " << replace_dots(name()) << endl
      << indent_prefix << "{" << endl;

    for (auto field : fields)
    {
        field->emit_interface_h(s, indent_prefix + "    ");
        s << ";" << endl;
    }

    s << indent_prefix << "};" << endl;
}

void record_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

// @todo: add record_v1_interface.h to includes only if we emit record aliases
void record_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Record: " << fqn << endl
      << indent_prefix << " */" << endl << endl;

    string nameprefix = get_root()->name() + "_" + name() + "_";

    // Describe each field.
    for (auto f : fields)
    {
        s << indent_prefix << "record_v1::field " << nameprefix << f->name() << "_field = {" << endl
          << indent_prefix << "    " << emit_type_code_prefix(*f) << "type_code," << endl
          << indent_prefix << "    offsetof(" << fqn << ", " << f->name() << ")" << endl
          << indent_prefix << "};" << endl;
    }

    s << indent_prefix << "field_t " << name() << "_fields[] = {" << endl;
    for (auto f : fields)
    {
        s << indent_prefix << "    { { record_v1::field_type_code, { .ptr32value = &" << nameprefix << f->name() << "_field } }, \"" << f->name() << "\", \"" << string_escape(f->get_autodoc()) << "\" }," << endl;
    }
    s << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "enum_rec_state_t " << name() << "_state_rec = {" << endl
      << indent_prefix << "    " << fields.size() << ", /* Number of fields */" << endl
      << indent_prefix << "    " << name() << "_fields" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "record_v1::closure_t " << name() << "_state_closure = {" << endl
      << indent_prefix << "    nullptr, /* Will be patched to record_ops. */" << endl
      << indent_prefix << "    reinterpret_cast<record_v1::state_t*>(&" << name() << "_state_rec)" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::record_type_code, { .ptr32value = &" << name() << "_state_closure } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// choice_alias_t
//=====================================================================================================================

void choice_alias_t::emit_impl_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
}

void choice_alias_t::emit_interface_h(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "struct " << replace_dots(name()) << endl
      << indent_prefix << "{" << endl

      << indent_prefix << "    " << selector << " tag;" << endl
      << indent_prefix << "    union choice {" << endl;

    for (auto field : choices)
    {
        field->emit_interface_h(s, indent_prefix + "        ");
        s << ";" << endl;
    }

    s << indent_prefix << "    };" << endl
      << indent_prefix << "};" << endl;

}

void choice_alias_t::emit_interface_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
}

// @todo: add choice_v1_interface.h to includes only if we emit choice aliases
void choice_alias_t::emit_typedef_cpp(std::ostringstream& s, std::string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Choice: " << fqn << endl
      << indent_prefix << " */" << endl << endl;

    string nameprefix = get_root()->name() + "_" + name() + "_";

    // Describe each field.
    for (auto f : choices)
    {
        s << indent_prefix << "choice_v1::field " << nameprefix << f->name() << "_field = {" << endl
          << indent_prefix << "    " << get_root()->name() << "::" << selector << "_" << f->name() << "," << endl
          << indent_prefix << "    " << emit_type_code_prefix(*f) << "type_code" << endl
          << indent_prefix << "};" << endl;
    }

    s << indent_prefix << "field_t " << name() << "_fields[] = {" << endl;
    for (auto f : choices)
    {
        s << indent_prefix << "    { { choice_v1::field_type_code, { .ptr32value = &" << nameprefix << f->name() << "_field } }, \"" << f->name() << "\", \"" << string_escape(f->get_autodoc()) << "\" }," << endl;
    }
    s << indent_prefix << "};" << endl << endl;

//@todo finish emitting right records for choice

    s << indent_prefix << "choice_state_t " << name() << "_state_rec = {" << endl
      << indent_prefix << "    { " << choices.size() << ", /* Number of fields */" << endl
      << indent_prefix << "    " << name() << "_fields }," << endl
      // @fixme: should emit_type_code_prefix() on selector...
      << indent_prefix << "    " << get_root()->name() << "::" << selector << "_type_code" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "choice_v1::closure_t " << name() << "_state_closure = {" << endl
      << indent_prefix << "    nullptr, /* Will be patched to choice_ops. */" << endl
      << indent_prefix << "    reinterpret_cast<choice_v1::state_t*>(&" << name() << "_state_rec)" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::choice_type_code, { .ptr32value = &" << name() << "_state_closure } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// enum_alias_t
//=====================================================================================================================

void enum_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void enum_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "enum " << replace_dots(name()) << endl
      << indent_prefix << "{" << endl;

    for (auto field : fields)
    {
        s << indent_prefix << "    " << replace_dots(this->name() + "_" + field) << "," << endl;
    }

    s << indent_prefix << "};" << endl;
}

void enum_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

// @todo: add enum_v1_interface.h to includes only if we emit enum aliases
void enum_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    string fqn = replace_dots(get_root()->name() + "." + name());
    s << indent_prefix << "/*" << endl
      << indent_prefix << " * Enum: " << fqn << endl
      << indent_prefix << " */" << endl << endl;

    s << indent_prefix << "field_t " << name() << "_elems[] = {" << endl;
    int value = 0;
    for (auto f : fields)
    {
        s << indent_prefix << "    { { enum_v1::value_type_code, { " << value << " } }, \"" << f << "\", \"\" }," << endl;
        ++value;
    }
    s << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "enum_rec_state_t " << name() << "_state_rec = {" << endl
      << indent_prefix << "    " << fields.size() << ", /* Number of enum elements */" << endl
      << indent_prefix << "    " << name() << "_elems" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "enum_v1::closure_t " << name() << "_state_closure = {" << endl
      << indent_prefix << "    nullptr, // Will be patched to enum_ops." << endl
      << indent_prefix << "    reinterpret_cast<enum_v1::state_t*>(&" << name() << "_state_rec)" << endl
      << indent_prefix << "};" << endl << endl;

    s << indent_prefix << "type_representation_t " << name() << "_type_rep = {" << endl
      << indent_prefix << "    { type_system_v1::enum__type_code, { .ptr32value = &" << name() << "_state_closure } }," << endl
      << indent_prefix << "    { types::code_type_code, { " << fqn << "_type_code } }," << endl
      << indent_prefix << "    \"" << name() << "\"," << endl
      << indent_prefix << "    \"" << string_escape(get_autodoc()) << "\", // autodoc" << endl
      << indent_prefix << "    &" << get_root()->name() << "__intf_typeinfo," << endl
      << indent_prefix << "    sizeof(" << fqn << ")" << endl
      << indent_prefix << "};" << endl
      << endl;
}

//=====================================================================================================================
// range_alias_t
//=====================================================================================================================

void range_alias_t::emit_impl_h(ostringstream& s, string indent_prefix, bool)
{
}

void range_alias_t::emit_interface_h(ostringstream& s, string indent_prefix, bool)
{
    s << indent_prefix << "typedef int " << replace_dots(name()) << ";" << endl; //TEMP hack
}

void range_alias_t::emit_interface_cpp(ostringstream& s, string indent_prefix, bool)
{
}

void range_alias_t::emit_typedef_cpp(ostringstream& s, string indent_prefix, bool fully_qualify_types)
{
    s << indent_prefix << "#error Should emit range alias here...." << name() << endl;
}

} // namespace AST
