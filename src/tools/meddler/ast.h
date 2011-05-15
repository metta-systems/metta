#pragma once

#include <vector>
#include <string>
#include "token.h"

namespace AST
{

class var_decl_t;
class exception_t;
class alias_t;
class method_t;

class node_t
{
	node_t* above;
public:
	node_t(node_t* parent) : above(parent) {}
	node_t* get_root() // for the purpose of this excercise, root will be the interface
	{
		node_t* parent = above;
		while (parent)
		{
			if (!parent->above)
				return parent;
			parent = parent->above;
		}
		return 0;
	}
    virtual std::string name() = 0;

    virtual void emit_impl_h(std::ostringstream& s) = 0;
    virtual void emit_interface_h(std::ostringstream& s) = 0;
    virtual void emit_interface_cpp(std::ostringstream& s) = 0;

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
    alias_t(node_t* parent) : node_t(parent), type(), kind(token::none), name_() {}
    alias_t(node_t* parent, std::string nm) : node_t(parent), type(), kind(token::type), name_(nm) {}
    alias_t(node_t* parent, std::string tp, std::string nm) : node_t(parent), type(tp), kind(token::type), name_(nm) {}
    virtual std::string name() { return name_; }
    virtual std::string unqualified_name();
    virtual void dump(std::string indent_prefix);
    virtual bool is_builtin_type() { return true; }// FIXME
	virtual void emit_include(std::ostringstream& s);
	virtual void emit_impl_h(std::ostringstream&) {/*abort();*/}
    virtual void emit_interface_h(std::ostringstream&) {/*abort();*/}
    virtual void emit_interface_cpp(std::ostringstream&) {/*abort();*/}

    std::string type; // use known types! check LLVM's Type/TypeBuilder
    token::kind kind;
    std::string name_;
};

// Variable or parameter declaration as "type-name" pair.
class var_decl_t : public alias_t
{
public:
    var_decl_t(node_t* parent) : alias_t(parent), reference(false) {}
    void set_reference() { reference = true; }
    bool is_reference() { return reference; }
    virtual void dump(std::string indent_prefix);

    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);

private:
    bool reference;
};

class type_alias_t : public alias_t
{
public:
    type_alias_t(node_t* parent) : alias_t(parent) {}
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

class sequence_alias_t : public alias_t
{
public:
    // type - base type
    // name - sequence type name
    sequence_alias_t(node_t* parent, std::string type, std::string base_type) : alias_t(parent, base_type, type) {}
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

class array_alias_t : public alias_t
{
public:
    // type - base type
    // name - sequence type name
    array_alias_t(node_t* parent, std::string type, std::string base_type, int c) : alias_t(parent, base_type, type), count(c) {}
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);

    int count;
};

class set_alias_t : public alias_t
{
public:
    // type - base type
    // name - set type name
    set_alias_t(node_t* parent, std::string type, std::string base_type) : alias_t(parent, base_type, type) {}
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

class record_alias_t : public alias_t
{
public:
    std::vector<var_decl_t*> fields;

    record_alias_t(node_t* parent, std::string nm) : alias_t(parent) { name_ = nm; }
    virtual bool add_field(var_decl_t* field);
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

class enum_alias_t : public alias_t
{
public:
    std::vector<std::string> fields;

    enum_alias_t(node_t* parent) : alias_t(parent) { }
    virtual bool add_field(var_decl_t* field);
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

class range_alias_t : public alias_t
{
public:
    std::string start, end;

    range_alias_t(node_t* parent, std::string nm, std::string s, std::string e) : alias_t(parent, nm), start(s), end(e) { }
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);
};

// Represents both method arguments and returns.
class parameter_t : public var_decl_t
{
public:
    enum direction_e { in, out, inout } direction;
	parameter_t(node_t* parent) : var_decl_t(parent), direction(inout) {}
    virtual void dump(std::string indent_prefix);
    virtual void emit_include(std::ostringstream&) {}//FIXME
};

class exception_t : public node_t
{
public:
    exception_t(node_t* parent, std::string nm) : node_t(parent), name_(nm) {}
    virtual std::string name() { return name_; }
    virtual bool add_field(var_decl_t* field);
    virtual void dump(std::string indent_prefix);
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);

    std::string name_;
    std::vector<var_decl_t*> fields;
};

class method_t : public node_t
{
public:
    method_t(node_t* parent) : node_t(parent), idempotent(false), never_returns(false) {}
    virtual std::string name() { return name_; }
    virtual void dump(std::string indent_prefix);
    virtual bool add_parameter(parameter_t*);
    virtual bool add_return(parameter_t*);
    virtual bool add_exception(exception_t*);
    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);

    std::string name_;
    std::vector<parameter_t*> params;
    std::vector<parameter_t*> returns;
    std::vector<exception_t*> raises;
    std::vector<std::string>  raises_ids;
    bool idempotent;
    bool never_returns; // oneway
    // generated properties
    std::string parent_interface;
};

class interface_t : public node_t
{
public:
    interface_t(std::string nm, bool is_local, bool is_final) : node_t(0), local(is_local), final(is_final), name_(nm) {}
    virtual std::string name() { return name_; }
    virtual bool add_method(method_t*);
    virtual bool add_exception(exception_t*);
    virtual bool add_imported_type(alias_t*);
    virtual bool add_type(alias_t*);
    virtual void dump(std::string indent_prefix);
    void set_parent(std::string p) { base = p; }

    virtual void emit_impl_h(std::ostringstream& s);
    virtual void emit_interface_h(std::ostringstream& s);
    virtual void emit_interface_cpp(std::ostringstream& s);

    bool local;
    bool final;
    std::string name_;
    std::string base;
    std::vector<alias_t*>     imported_types;//added to this list when we see an unknown fully qualified identifier in var_decls.
	// builtin identifiers should resolve to known types in list.
	// unqualified identifiers should resolve to the following list:
    std::vector<alias_t*>     types;
    std::vector<exception_t*> exceptions;
    std::vector<method_t*>    methods;
};

}
