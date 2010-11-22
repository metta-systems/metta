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
        TNAME(identifier)
    }
    return "UNKNOWN";
}

bool parser_t::run()
{
    lex.lex(); // prime the parser
    return parse_top_level_entities();
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
        return parse_interface_body();
    }
    lex.lex();
    return false;
}

bool parser_t::parse_interface_body()
{
    D();
    if (lex.token_kind() != token::kind::identifier)
        return false;

    AST::interface_t* node = new AST::interface_t(lex.current_token(), is_local, is_final);
    node->dump();

    parse_tree = node;

    if (!lex.expect(token::kind::lbrace))
    {
        std::cerr << "{ expected" << std::endl;
        return false;
    }

    // Parse body here (should be a loop!)
    //   body = { exception | typedef | method } ;
    lex.lex();

    switch (lex.token_kind())
    {
//         case token::kind::rbrace: leave
        case token::kind::kw_exception:
            parse_exception();
            break;
// if (kind == token::kind::kw_range) parse_range_type_alias();
// if (kind == token::kind::kw_sequence) parse_sequence_type_alias();
// if (kind == token::kind::kw_set) parse_set_type_alias();
// if (kind == token::kind::kw_record) parse_record_type_alias();
// if (kind == token::kind::type_decl) parse_type_alias();
        case token::kind::kw_idempotent:
        case token::kind::identifier:
            parse_method();
            break;
        default:
            std::cerr << "Invalid token....blabla" << std::endl;
            return false;
    }

    if (!lex.expect(token::kind::rbrace))
    {
        std::cerr << "} expected" << std::endl;
        return false;
    }

    return true;
}

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
        return true;//parse_argument_list()
    }
    return false;
}

//! field_list ::= '{' (fieldtype fieldname ';')* '}'
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

// parse_field_list(node_t* node) would call node->add_field(parsed_field)
bool parser_t::parse_field(AST::node_t* parent)
{
    D();
    if (!lex.expect(token::kind::identifier)) //lex.maybe(reference);
    {
        std::cerr << "field type ID expected" << std::endl;
        return false;
    }
    AST::var_decl_t* field = new AST::var_decl_t(lex.current_token());
    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "field name expected" << std::endl;
        return false;
    }
    field->name = lex.current_token();
    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    parent->add_field(field);
    return true;
}
