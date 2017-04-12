//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "ast.h"
#include <iostream>
#include <algorithm>
#include <utility>

namespace AST
{

/**
 * Search for type name in local types list.
 */
bool interface_t::types_lookup(alias_t& type)
{
    std::string tp = type.type();
    for (auto t : types)
    {
        if (t->name() == tp)
            return true;
    }
    return false;
}

/**
 * Search for type name in imported types list.
 */
bool interface_t::imported_types_lookup(alias_t& type)
{
    std::string tp = type.type();
    for (auto t : imported_types)
    {
        if (t->name() == tp)
            return true;
    }
    return false;
}

bool interface_t::add_exception(exception_t* exc)
{
    //std::cout << "interface_t::add_exception()" << std::endl;
    exceptions.push_back(exc);
    return true;
}

/**
 * Add locally defined type to the list.
 */
bool interface_t::add_type(alias_t* t)
{
    //std::cout << "interface_t::add_type()" << std::endl;
    types.push_back(t);
    return true;
}

/**
 * Add imported type to the list.
 */
bool interface_t::add_imported_type(alias_t t)
{
    //std::cout << "interface_t::add_imported_type()" << std::endl;
    auto* copy = new alias_t(std::move(t));
    copy->set_name(copy->type()); // YURGH! to fix this, emit_cpp should use type()/base_type() on imports instead of name()/base_name() - fix it there first! then fix imported_types_lookup() above, too!
    imported_types.push_back(copy);
    return true;
}

bool interface_t::add_method(method_t* m)
{
    //std::cout << "interface_t::add_method()" << std::endl;
    m->parent_interface = name();
    methods.push_back(m);
    return true;
}

bool exception_t::add_field(alias_t* field)
{
    //std::cout << "exception_t::add_field()" << std::endl;
    fields.push_back(field);
    return true;
}

std::string alias_t::unqualified_name()
{
    size_t pos;
    if ((pos = type_.find_last_of('.')) != std::string::npos)
    {
        return type_.substr(pos+1);
    }
    return type_;
}

bool record_alias_t::add_field(alias_t* field)
{
    //std::cout << "record_alias_t::add_field()" << std::endl;
    fields.push_back(field);
    return true;
}

bool choice_alias_t::add_field(alias_t* field)
{
    choices.push_back(field);
    return true;
}

bool enum_alias_t::add_field(alias_t* field)
{
    //std::cout << "enum_alias_t::add_field()" << std::endl;
    fields.push_back(field->name());
    return true;
}

bool method_t::add_parameter(parameter_t* p)
{
    //std::cout << "method_t::add_parameter()" << std::endl;
    params.push_back(p);
    return true;
}

bool method_t::add_return(parameter_t* r)
{
    //std::cout << "method_t::add_return()" << std::endl;
    returns.push_back(r);
    return true;
}

bool method_t::add_exception(exception_t* e)
{
    //std::cout << "method_t::add_exception()" << std::endl;
    raises.push_back(e);
    return true;
}

void alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << type() << (reference ? "& " : " ") << name() << " [kind:" << kind << " intf:" << interface << " local:" << local << " builtin:" << builtin << "]" << std::endl;
}

void parameter_t::dump(std::string indent_prefix)
{
    const char* dirs[] = {"in ", "out ", "inout "};
    std::cout << indent_prefix << dirs[direction]; alias_t::dump("");
}

void interface_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(interface_t(")" << name() << R"(", local:)" << (local ? "true":"false") << ", final:" << (final ? "true":"false") << ")" << (!base.empty() ? std::string(" extends ") + base : "") << std::endl;

    std::cout << indent_prefix << "+-types" << std::endl;
    if (types.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto a : types) { a->dump(indent_prefix + "  "); }

    std::cout << indent_prefix << "+-imported types" << std::endl;
    if (imported_types.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto a : imported_types) { a->dump(indent_prefix + "  "); }

    std::cout << indent_prefix << "+-exceptions" << std::endl;
    if (exceptions.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto e : exceptions) { e->dump(indent_prefix + "  "); }

    std::cout << indent_prefix << "+-methods" << std::endl;
    if (methods.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto m : methods) { m->dump(indent_prefix + "  "); }
}

void exception_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(exception_t(")" << name() << R"("))" << std::endl;
    std::cout << indent_prefix << "+-fields" << std::endl;
    if (fields.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto var : fields) {
            std::cout << indent_prefix << "  "; var->dump(""); std::cout << ";" << std::endl;
        }
}

void record_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(record_t(")" << name() << R"("))" << std::endl;
    std::cout << indent_prefix << "+-fields" << std::endl;
    if (fields.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto var : fields) {
            std::cout << indent_prefix << "  "; var->dump("");
        }
}

void choice_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(choice_t(")" << name() << R"("))" << std::endl;
    std::cout << indent_prefix << "+-selector " << selector << std::endl;
    if (choices.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto var : choices) {
            std::cout << indent_prefix << "  "; var->dump("");
        }
}

void enum_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(enum_t(")" << name() << R"("))" << std::endl;
    std::cout << indent_prefix << "+-fields" << std::endl;
    if (fields.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (const auto& s : fields) {
            std::cout << indent_prefix << "  " << s << std::endl;
        }
}

void range_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(range_t(")" << name() << R"("))" << std::endl;
    std::cout << indent_prefix << "+-start: " << start << std::endl;
    std::cout << indent_prefix << "+-end:   " << end << std::endl;
}

void sequence_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(sequence_t(")" << name() << R"(") of )" << type() << std::endl;
}

void set_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(set_t(")" << name() << R"(") of )" << type() << std::endl;
}

void array_alias_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(array_t(")" << name() << R"(") of )" << count << " times " << type() << std::endl;
}

void method_t::dump(std::string indent_prefix)
{
    std::cout << indent_prefix << R"(method_t(")" << name() << R"("))" << (idempotent ? " idempotent" : "") << std::endl;
    std::cout << indent_prefix << "+-arguments" << std::endl;
    if (params.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto p : params) { p->dump(indent_prefix + "  "); }

    if (never_returns)
        std::cout << indent_prefix << "+-never returns" << std::endl;
    else
    {
        std::cout << indent_prefix << "+-returns" << std::endl;
        if (returns.size() == 0)
            std::cout << indent_prefix << "  [empty]" << std::endl;
        else
            for (auto p : returns) { p->dump(indent_prefix + "  "); }
    }
    std::cout << indent_prefix << "+-exceptions" << std::endl;
    if (raises.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (auto e : raises) { e->dump(indent_prefix + "  "); }

    std::cout << indent_prefix << "+-unresolved exceptions ids" << std::endl;
    if (raises_ids.size() == 0)
        std::cout << indent_prefix << "  [empty]" << std::endl;
    else
        for (const auto& s : raises_ids) { 
            std::cout << indent_prefix << "  " << s << std::endl;
        }
}

} // namespace AST
