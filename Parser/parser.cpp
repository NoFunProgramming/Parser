#include "parser.hpp"

using std::make_unique;

/******************************************************************************/
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

void
Parser::print(ostream& out) const
{
    for (auto& nonterm : nonterms) {
        nonterm.second->print(out);
        out << "\n";
    }
}

bool
Parser::read_rules(istream& in)
{
    Nonterm* nonterm = read_nonterm(in);
    if (in.get() != ':') {
        return false;
    }
    
    Nonterm::Rule* rule = nonterm->add_rule();
    while (in.peek() != EOF) {
        read_product(in, rule);
        if (in.peek() == ';') {
            in.get();
            break;
        } else if (in.peek() == '|') {
            in.get();
            rule = nonterm->add_rule();
        }
    }
    return true;
}

bool
Parser::read_product(istream& in, Nonterm::Rule* rule)
{
    while (in.peek() != EOF) {
        in >> std::ws;
        if (in.peek() == ';' || in.peek() == '|') {
            break;
        }
        else if (in.peek() == '\'') {
            Symbol* sym = read_term(in);
            rule->add(sym);
        }
        else {
            Symbol* sym = read_nonterm(in);
            rule->add(sym);
        }
    }
    return true;
}

Term*
Parser::read_term(istream& in)
{
    if (in.get() != '\'') {
        return nullptr;
    }
    string name;
    while (isalpha(in.peek())) {
        name.push_back(in.get());
    }
    if (in.get() != '\'') {
        return nullptr;
    }
    
    terms[name] = make_unique<Term>(name);
    return terms[name].get();
}

Nonterm*
Parser::read_nonterm(istream& in)
{
    string name;
    while (isalpha(in.peek())) {
        name.push_back(in.get());
    }
    
    nonterms[name] = make_unique<Nonterm>(name);
    return nonterms[name].get();
}
