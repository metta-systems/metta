#include <iostream>
#include "parser.h"
#include "ast.h"

std::string token_to_name(token::kind tok)
{
#define TNAME(tk) \
    case token::kind::tk: return #tk;
    switch (tok)
    {
        TNAME(none)
        TNAME(eof)
        TNAME(error)
        TNAME(equal)
        TNAME(comma)
        TNAME(reference)
        TNAME(lsquare)
        TNAME(rsquare)
        TNAME(lbrace)
        TNAME(rbrace)
        TNAME(less)
        TNAME(greater)
        TNAME(lparen)
        TNAME(rparen)
        TNAME(semicolon)
        TNAME(backslash)
        TNAME(type)
        TNAME(kw_local)
        TNAME(kw_final)
        TNAME(kw_interface)
        TNAME(kw_exception)
        TNAME(kw_in)
        TNAME(kw_inout)
        TNAME(kw_out)
        TNAME(kw_idempotent)
        TNAME(kw_raises)
        TNAME(kw_needs)
        TNAME(kw_extends)
        TNAME(kw_never)
        TNAME(kw_returns)
        TNAME(kw_type)
        TNAME(kw_sequence)
        TNAME(kw_set)
        TNAME(kw_range)
        TNAME(kw_record)
        TNAME(identifier)
    }
    return "UNKNOWN";
}

parser_t::parser_t(llvm::MemoryBuffer *F)
    : is_local(false)
    , is_final(false)
    , lex(F, &symbols)
    , parse_tree(0)
{
    populate_symbol_table();
}

// Store various id types in a symbol table.
void parser_t::populate_symbol_table()
{
    symbols.insert("local", token::kind::kw_local);
    symbols.insert("final", token::kind::kw_final);
    symbols.insert("interface", token::kind::kw_interface);
    symbols.insert("exception", token::kind::kw_exception);
    symbols.insert("in", token::kind::kw_in);
    symbols.insert("inout", token::kind::kw_inout);
    symbols.insert("out", token::kind::kw_out);
    symbols.insert("idempotent", token::kind::kw_idempotent);
    symbols.insert("raises", token::kind::kw_raises);
    symbols.insert("extends", token::kind::kw_extends);
    symbols.insert("never", token::kind::kw_never);
    symbols.insert("returns", token::kind::kw_returns);
    symbols.insert("type", token::kind::kw_type);
    symbols.insert("sequence", token::kind::kw_sequence);
    symbols.insert("set", token::kind::kw_set);
    symbols.insert("range", token::kind::kw_range);
    symbols.insert("record", token::kind::kw_record);
    symbols.insert("int8", token::kind::type/*, builtin_type*/);
    symbols.insert("int16", token::kind::type/*, builtin_type*/);
    symbols.insert("int32", token::kind::type/*, builtin_type*/);
    symbols.insert("int64", token::kind::type/*, builtin_type*/);
    symbols.insert("octet", token::kind::type/*, builtin_type*/);
    symbols.insert("card16", token::kind::type/*, builtin_type*/);
    symbols.insert("card32", token::kind::type/*, builtin_type*/);
    symbols.insert("card64", token::kind::type/*, builtin_type*/);
    symbols.insert("float", token::kind::type/*, builtin_type*/);
    symbols.insert("double", token::kind::type/*, builtin_type*/);
    symbols.insert("boolean", token::kind::type/*, builtin_type*/);
    symbols.insert("string", token::kind::type/*, builtin_type*/);
}

bool parser_t::run()
{
    lex.lex(); // prime the parser
    bool ret = parse_top_level_entities();
    symbols.dump();
    return ret;
}

#define D() std::cout << __FUNCTION__ << ": " << token_to_name(lex.token_kind()) << ": " << lex.current_token() << std::endl

bool parser_t::parse_top_level_entities()
{
    is_local = false;
    is_final = false;
    while (1) {
        D();
        switch (lex.token_kind())
        {
            default:         return false;//error("expected top-level entity");
            case token::kind::eof: return false;
            case token::kind::kw_local:
            case token::kind::kw_final:
            case token::kind::kw_interface:
                if (parse_interface())
                    return true;
                else
                    return false;
                break;
        }
    }
}

// parse interface definition ([local] [final] interface ID...)
bool parser_t::parse_interface()
{
    D();
    if (lex.token_kind() == token::kind::kw_local)
    {
        is_local = true;
        lex.lex();
        return parse_interface(); // expect final or interface
    }
    if (lex.token_kind() == token::kind::kw_final)
    {
        is_final = true;
        lex.lex();
        return parse_interface(); // expect local or interface
    }
    if (lex.token_kind() == token::kind::kw_interface)
    {
        lex.lex();
        if (lex.token_kind() != token::kind::identifier)
            return false;

        AST::interface_t* node = new AST::interface_t(lex.current_token(), is_local, is_final);
        parse_tree = node;

        if (!lex.expect(token::kind::lbrace))
        {
            std::cerr << "{ expected" << std::endl;
            return false;
        }

        if (!parse_interface_body())
            return false;

        if (!lex.expect(token::kind::rbrace))
        {
            std::cerr << "} expected" << std::endl;
            return false;
        }

        node->dump();
        return true;
    }
    std::cerr << "unexpected" << std::endl;
    return false;
}

//! typeid ::= id | builtin_type
//! body ::= ( exception | typealias | method )*
//! typealias ::= rangedef | sequencedef | setdef | recorddef | typedef
bool parser_t::parse_interface_body()
{
    D();

    while (lex.lex() != token::eof)
    {
        switch (lex.token_kind())
        {
            case token::kind::rbrace: // end of interface declaration
                lex.lexback();
                return true;
            case token::kind::kw_exception:
                if (!parse_exception())
                {
                    std::cerr << "Exception parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_range:
                parse_range_type_alias();
                break;
            case token::kind::kw_sequence:
                parse_sequence_type_alias();
                break;
            case token::kind::kw_set:
                parse_set_type_alias();
                break;
            case token::kind::kw_record:
                parse_record_type_alias();
                break;
            case token::kind::kw_type:
                parse_type_alias();
                break;
            case token::kind::kw_idempotent:
            case token::kind::identifier:
                if (!parse_method())
                {
                    std::cerr << "Method parse failed." << std::endl;
                    return false;
                }
                break;
            default:
                std::cerr << "Invalid token....blabla" << std::endl;
                return false;
        }
    }

    return false;
}

//! exception ::= 'exception' id '{' field_list '}'
bool parser_t::parse_exception()
{
    D();
    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "exception ID expected" << std::endl;
        return false;
    }

    AST::exception_t* node = new AST::exception_t(lex.current_token());
    if (!lex.expect(token::kind::lbrace))
    {
        std::cerr << "{ expected" << std::endl;
        return false;
    }

    parse_field_list(node);

    if (!lex.expect(token::kind::rbrace))
    {
        std::cerr << "} expected" << std::endl;
        return false;
    }

    parse_tree->add_exception_def(node);
    node->dump();
    return true;
}

//! method_decl ::= ['idempotent'] id argument_list ['returns' argument_list|'never' 'returns'] ['raises' id_list]
bool parser_t::parse_method()
{
    D();
    if (lex.token_kind() == token::kind::kw_idempotent)
    {
        is_idempotent = true;
        lex.lex();
        return parse_method(); // expect method name
    }
    if (lex.token_kind() == token::kind::identifier)
    {
        if (!lex.expect(token::kind::lparen))
        {
            std::cerr << "( expected" << std::endl;
            return false;
        }

        AST::node_t* method = 0;
        if (!parse_argument_list(method))
            return false;

        if (!lex.expect(token::kind::rparen))
        {
            std::cerr << ") expected" << std::endl;
            return false;
        }

//         if (lex.maybe(token::kind::kw_returns))
//             parse_method_returns();
//         if (lex.maybe(token::kind::kw_raises))
//             parse_method_raises();

        if (lex.expect(token::kind::semicolon))
            return true;

        return false;
    }
    return false;
}

//! argument_list ::= '(' [ var_decl (',' var_decl)* ] ')'
bool parser_t::parse_argument_list(AST::node_t* parent)
{
    D();
    while (lex.lex() != token::kind::rparen)
    {
        lex.lexback();
        if (!parse_argument(parent))
            return false;
    }
    lex.lexback();
    return true;
}

//! field_list ::= '{' (var_decl ';')* '}'
bool parser_t::parse_field_list(AST::node_t* parent)
{
    D();
    while (lex.lex() != token::kind::rbrace)
    {
        lex.lexback();
        if (!parse_field(parent))
            return false;
    }
    lex.lexback();
    return true;
}

//! var_decl ::= typeid [reference] id
bool parser_t::parse_var_decl(AST::var_decl_t& to_get)
{
    if (!lex.expect(token::kind::type))
    {
        std::cerr << "field type ID expected" << std::endl;
        return false;
    }
    to_get.type = lex.current_token();
    //lex.maybe(reference);
    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "field name expected" << std::endl;
        return false;
    }
    to_get.name = lex.current_token();
    return true;
}

//! field ::= var_decl ';'
bool parser_t::parse_field(AST::node_t* parent)
{
    D();
    AST::var_decl_t* field = new AST::var_decl_t;
    if (!parse_var_decl(*field))
    {
        delete field;
        return false;
    }
    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    parent->add_field(field);
    return true;
}

//! argument ::= var_decl [','|')']
bool parser_t::parse_argument(AST::node_t* /*parent*/)
{
    D();
    AST::var_decl_t* field = new AST::var_decl_t;
    if (!parse_var_decl(*field))
    {
        delete field;
        return false;
    }
//     parent->add_field(field);
    if (!lex.expect(token::kind::comma))
    {
        if (lex.token_kind() == token::rparen)
        {
            lex.lexback();
            return true;
        }
        std::cerr << ", expected" << std::endl;
        return false;
    }
    return true;
}

//! rangedef ::=
bool parser_t::parse_range_type_alias()
{
    return false;
}

//! sequencedef ::=
bool parser_t::parse_sequence_type_alias()
{
    return false;
}

//! setdef ::=
bool parser_t::parse_set_type_alias()
{
    return false;
}

//! recorddef ::=
bool parser_t::parse_record_type_alias()
{
    return false;
}

//! typedef ::=
bool parser_t::parse_type_alias()
{
    return false;
}
