#include "parser.hpp"

Parser::Parser() {}

bool
Parser::read_grammar(istream& in)
{
    while (in.peek() != EOF) {
        in >> std::ws;
        read_rules(in);
    }
    return true;
}

bool
Parser::read_rules(istream& in)
{
    read_nonterm(in);
    return true;
}

bool
Parser::read_product(istream& in)
{
    return true;
}

bool
Parser::read_term(istream& in)
{
    return true;
}

bool
Parser::read_nonterm(istream& in)
{    
    return true;
}
