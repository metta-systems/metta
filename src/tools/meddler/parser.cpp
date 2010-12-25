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
//         TNAME(type) identifier
        TNAME(kw_local)
        TNAME(kw_final)
        TNAME(kw_interface)
        TNAME(kw_exception)
        TNAME(kw_in)
        TNAME(kw_inout)
        TNAME(kw_out)
        TNAME(kw_idempotent)
        TNAME(kw_raises)
        TNAME(kw_extends)
        TNAME(kw_never)
        TNAME(kw_returns)
        TNAME(kw_type)
        TNAME(kw_sequence)
        TNAME(kw_set)
        TNAME(kw_range)
        TNAME(kw_record)
        TNAME(kw_enum)
        TNAME(kw_array)
        TNAME(identifier)
        TNAME(dotdot)
        TNAME(cardinal)
    }
    return "UNKNOWN";
}

parser_t::parser_t()
    : is_local(false)
    , is_final(false)
    , is_idempotent(false)
    , lex()
    , parse_tree(0)
{
}

parser_t::parser_t(llvm::MemoryBuffer *F)
    : is_local(false)
    , is_final(false)
    , is_idempotent(false)
    , lex(F, &symbols)
    , parse_tree(0)
{
    init(F);
}

void parser_t::init(const llvm::MemoryBuffer *F)
{
    is_local = is_final = is_idempotent = false;
    delete parse_tree; parse_tree = 0;
    lex.init(F, &symbols);
    populate_symbol_table();
}

// Store various id types in a symbol table.
void parser_t::populate_symbol_table()
{
    symbols.clear();
    symbols.insert("..", token::kind::dotdot);
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
    symbols.insert("enum", token::kind::kw_enum);
    symbols.insert("array", token::kind::kw_array);
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
//     symbols.dump();
    if (ret)
        std::cout << "** PARSE SUCCESS" << std::endl;
    else
        std::cout << "** PARSE FAILURE" << std::endl;
    return ret;
}

#define D() L(std::cout << __FUNCTION__ << ": " << token_to_name(lex.token_kind()) << ": " << lex.current_token() << std::endl)

// static void error(const char *msg)
// {
//     std::cerr << msg << endl;
//     exit(1);
// }

//! module ::= full_interface_decl
//! full_interface_decl ::= local_interface_decl | final_interface_decl | interface_decl
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

//! local_interface_decl ::= 'local' (final_interface_decl | interface_decl)
//! final_interface_decl ::= 'final' (local_interface_decl | interface_decl)
//! interface_decl ::= 'interface' id ['extends' id] '{' interface_body '}'
bool parser_t::parse_interface()
{
    D();
    if (lex.match(token::kind::kw_local))
    {
        is_local = true;
        lex.lex();
        return parse_interface(); // expect final or interface
    }
    if (lex.match(token::kind::kw_final))
    {
        is_final = true;
        lex.lex();
        return parse_interface(); // expect local or interface
    }
    if (lex.match(token::kind::kw_interface))
    {
        if (!lex.expect(token::kind::identifier))
            return false;

        AST::interface_t* node = new AST::interface_t(lex.current_token(), is_local, is_final);
        parse_tree = node;

        if (lex.maybe(token::kind::kw_extends))
        {
            lex.lex();
            if (lex.token_kind() != token::kind::identifier)
            {
                std::cerr << "'extends' needs interface id" << std::endl;
                return false;
            }
            node->set_parent(lex.current_token());
        }

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

        node->dump("");
        return true;
    }
    std::cerr << "unexpected" << std::endl;
    return false;
}

//! interface_body ::= (exception | typealias | full_method_decl)*
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
            // Exception
            case token::kind::kw_exception:
                if (!parse_exception())
                {
                    std::cerr << "Exception parse failed." << std::endl;
                    return false;
                }
                break;
            // Typealiases
            case token::kind::kw_enum:
                if (!parse_enum_type_alias())
                {
                    std::cerr << "Enum type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_array:
                if (!parse_array_type_alias())
                {
                    std::cerr << "Array type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_range:
                if (!parse_range_type_alias())
                {
                    std::cerr << "Range type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_sequence:
                if (!parse_sequence_type_alias())
                {
                    std::cerr << "Sequence type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_set:
                if (!parse_set_type_alias())
                {
                    std::cerr << "Set type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_record:
                if (!parse_record_type_alias())
                {
                    std::cerr << "Record type parse failed." << std::endl;
                    return false;
                }
                break;
            case token::kind::kw_type:
                if (!parse_type_alias())
                {
                    std::cerr << "Type alias parse failed." << std::endl;
                    return false;
                }
                break;
            // Method declaration
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

//! exception ::= 'exception' id '{' exception_body '}'
//! exception_body ::= field_list
bool parser_t::parse_exception()
{
    D();
    if (!lex.match(token::kind::kw_exception))
        return false;

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

    parse_tree->add_exception(node);
    return true;
}

bool parser_t::parse_method()
{
    D();
    if (lex.match(token::kind::kw_idempotent))
    {
        is_idempotent = true;
        lex.lex();
        return parse_method(); // expect method name
    }
    if (lex.match(token::kind::identifier))
    {
        std::string name = lex.current_token();

        if (!lex.expect(token::kind::lparen))
        {
            std::cerr << "( expected" << std::endl;
            return false;
        }

        AST::method_t m;
        m.name = name;
        m.idempotent = is_idempotent;
        is_idempotent = false;

        std::vector<AST::parameter_t*> params;
        if (!parse_argument_list(params, AST::parameter_t::in))
            return false;

        m.params = params;

        if (!lex.expect(token::kind::rparen))
        {
            std::cerr << ") expected" << std::endl;
            return false;
        }

        if (lex.maybe(token::kind::kw_returns) || lex.maybe(token::kind::kw_never))
            parse_method_returns(m);

        if (lex.maybe(token::kind::kw_raises))
            parse_method_raises(m);

        if (lex.expect(token::kind::semicolon))
        {
            parse_tree->add_method(new AST::method_t(m));
            return true;
        }
    }
    return false;
}

// TODO: return arglist here so we can stash it into right place
bool parser_t::parse_argument_list(std::vector<AST::parameter_t*>& args, AST::parameter_t::direction_e default_dir)
{
    D();
    while (lex.lex() != token::kind::rparen)
    {
        lex.lexback();
        if (!parse_argument(args, default_dir))
            return false;
    }
    lex.lexback();
    return true;
}

//! field_list ::= (var_decl ';')*
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

bool parser_t::parse_id_list(std::vector<std::string>& ids, token::kind delim)
{
    D();
    while (lex.lex() != token::kind::rparen)
    {
        lex.lexback();
        if (!lex.expect(token::kind::type))
        {
            if (!lex.match(token::kind::identifier))
            {
                std::cerr << "type ID expected" << std::endl;
                return false;
            }
        }
        ids.push_back(lex.current_token());
        if (!lex.expect(token::kind::comma))
        {
            if (lex.match(delim))
            {
                lex.lexback();
                return true;
            }
            std::cerr << ", or delimiter expected" << std::endl;
            return false;
        }
    }
    lex.lexback();//??
    return true;
}

//! var_decl ::= typeid [reference] id
bool parser_t::parse_var_decl(AST::var_decl_t& to_get)
{
    D();
    if (!lex.expect(token::kind::type))
    {
        if (!lex.match(token::kind::identifier))
        {
            std::cerr << "field type ID expected" << std::endl;
            return false;
        }
    }
    to_get.type = lex.current_token();
    if (lex.maybe(token::kind::reference))
        to_get.set_reference();
    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "field name expected" << std::endl;
        return false;
    }
    to_get.name = lex.current_token();
    return true;
}

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

bool parser_t::parse_argument(std::vector<AST::parameter_t*>& args, AST::parameter_t::direction_e default_dir)
{
    D();
    AST::parameter_t p;
    p.direction = default_dir;
    if (lex.maybe(token::kind::kw_in))
    {
        p.direction = p.in;
    }
    else if (lex.maybe(token::kind::kw_out))
    {
        p.direction = p.out;
    }
    else if(lex.maybe(token::kind::kw_inout))
    {
        p.direction = p.inout;
    }
    if (!parse_var_decl(p))
    {
        return false;
    }
    args.push_back(new AST::parameter_t(p));
    if (!lex.expect(token::kind::comma))
    {
        if (lex.match(token::rparen))
        {
            lex.lexback();
            return true;
        }
        std::cerr << ", or ) expected" << std::endl;
        return false;
    }
    return true;
}

//! typedef ::= 'type' id id ';'
bool parser_t::parse_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_type))
        return false;
    AST::type_alias_t t;
    if (!lex.expect(token::kind::identifier))
    {
        if (!lex.match(token::kind::type))
        {
            std::cerr << "type ID expected" << std::endl;
            return false;
        }
    }
    t.type = lex.current_token();
    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "type name expected" << std::endl;
        return false;
    }
    t.name = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    parse_tree->add_type(new AST::type_alias_t(t));

    return true;
}

bool parser_t::parse_range_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_range))
        return false;

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "range start ID expected" << std::endl;
        return false;
    }

    std::string range_start = lex.current_token();

    if (!lex.expect(token::kind::dotdot))
    {
        std::cerr << ".. expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "range end ID expected" << std::endl;
        return false;
    }

    std::string range_end = lex.current_token();

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "range type ID expected" << std::endl;
        return false;
    }

    std::string range_id = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    AST::range_alias_t* node = new AST::range_alias_t(range_id, range_start, range_end);

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_sequence_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_sequence))
        return false;

    if (!lex.expect(token::kind::less))
    {
        std::cerr << "< expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "sequence base type ID expected" << std::endl;
        return false;
    }

    std::string base_type = lex.current_token();

    if (!lex.expect(token::kind::greater))
    {
        std::cerr << "> expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "sequence type ID expected" << std::endl;
        return false;
    }

    std::string type = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    AST::sequence_alias_t* node = new AST::sequence_alias_t(type, base_type);

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_set_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_set))
        return false;

    if (!lex.expect(token::kind::less))
    {
        std::cerr << "< expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "set base type ID expected" << std::endl;
        return false;
    }

    std::string base_type = lex.current_token();

    if (!lex.expect(token::kind::greater))
    {
        std::cerr << "> expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "set type ID expected" << std::endl;
        return false;
    }

    std::string type = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    AST::set_alias_t* node = new AST::set_alias_t(type, base_type);

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_record_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_record))
        return false;

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "record ID expected" << std::endl;
        return false;
    }

    AST::record_alias_t* node = new AST::record_alias_t(lex.current_token());

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

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_enum_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_enum))
        return false;

    AST::enum_alias_t* node = new AST::enum_alias_t;//memleaks on errors!

    if (!lex.expect(token::kind::lbrace))
    {
        std::cerr << "{ expected" << std::endl;
        return false;
    }

    std::vector<std::string> ids;
    if (!parse_id_list(ids, token::rbrace))
    {
        std::cerr << "enum list parse failed" << std::endl;
        return false;
    }

    node->fields = ids;

    if (!lex.expect(token::kind::rbrace))
    {
        std::cerr << "} expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "enum ID expected" << std::endl;
        return false;
    }

    node->name = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_array_type_alias()
{
    D();
    if (!lex.match(token::kind::kw_array))
        return false;

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "array base type ID expected" << std::endl;
        return false;
    }

    std::string base_type = lex.current_token();

    if (!lex.expect(token::kind::lsquare))
    {
        std::cerr << "[ expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::cardinal))
    {
        std::cerr << "number of repetitions expected" << std::endl;
        return false;
    }

    int count = lex.current_value();

    if (!lex.expect(token::kind::rsquare))
    {
        std::cerr << "] expected" << std::endl;
        return false;
    }

    if (!lex.expect(token::kind::identifier))
    {
        std::cerr << "array ID expected" << std::endl;
        return false;
    }

    std::string type = lex.current_token();

    if (!lex.expect(token::kind::semicolon))
    {
        std::cerr << "; expected" << std::endl;
        return false;
    }

    AST::array_alias_t* node = new AST::array_alias_t(type, base_type, count);

    parse_tree->add_type(node);
    return true;
}

bool parser_t::parse_method_returns(AST::method_t& m)
{
    D();

    if (lex.match(token::kind::kw_never))
    {
        if (!lex.expect(token::kind::kw_returns))
        {
            std::cerr << "'returns' expected after 'never'" << std::endl;
            return false;
        }
        m.returns.clear();
        m.never_returns = true;
        return true;
    }

    if (!lex.match(token::kind::kw_returns))
        return false;

    if (!lex.expect(token::kind::lparen))
    {
        std::cerr << "( expected" << std::endl;
        return false;
    }

    std::vector<AST::parameter_t*> returns;
    if (!parse_argument_list(returns, AST::parameter_t::out))
        return false;

    m.returns = returns;

    if (!lex.expect(token::kind::rparen))
    {
        std::cerr << ") expected" << std::endl;
        return false;
    }

    return true;
}

bool parser_t::parse_method_raises(AST::method_t& m)
{
    D();
    if (!lex.match(token::kind::kw_raises))
        return false;

    if (!lex.expect(token::kind::lparen))
    {
        std::cerr << "( expected" << std::endl;
        return false;
    }

    std::vector<std::string> exc_ids;
    if (!parse_id_list(exc_ids, token::rparen))
        return false;

    m.raises_ids = exc_ids;

    if (!lex.expect(token::kind::rparen))
    {
        std::cerr << ") expected" << std::endl;
        return false;
    }

    return true;
}
