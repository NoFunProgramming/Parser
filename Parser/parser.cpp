#include "parser.hpp"

#include <iostream>
using std::cerr;

using std::make_unique;

/******************************************************************************/
Parser::Parser():
    first(nullptr){}

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
Parser::print_grammar(ostream& out) const
{
    for (auto& nonterm : nonterms) {
        nonterm.second->print(out);
        out << "\n";
    }
}

Term*
Parser::read_term(istream& in)
{
    if (in.get() != '\'') {
        cerr << "Expected quote to start terminal.\n";
        return nullptr;
    }
    string name;
    while (isalpha(in.peek())) {
        name.push_back(in.get());
    }
    if (in.get() != '\'') {
        cerr << "Expected quote to end terminal.\n";
        return nullptr;
    }
    
    if (name.size() < 1) {
        cerr << "Terminals require at least one character.\n";
    }
    
    if (terms.count(name) == 0) {
        terms[name] = make_unique<Term>(name);
    }
    return terms[name].get();
}

Nonterm*
Parser::read_nonterm(istream& in)
{
    string name;
    while (isalpha(in.peek())) {
        name.push_back(in.get());
    }

    if (name.size() < 1) {
        cerr << "Nonterminals require at least one character.\n";
    }

    if (nonterms.count(name) == 0) {
        nonterms[name] = make_unique<Nonterm>(name);
    }
    return nonterms[name].get();
}

bool
Parser::read_rules(istream& in)
{
    Nonterm* nonterm = read_nonterm(in);
    if (!nonterm) {
        cerr << "Unable to read nonterminal name.\n";
    }
    if (in.get() != ':') {
        cerr << "Nontermianals end with a colon.\n";
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
            if (!sym) {
                return false;
            }
            rule->add(sym);
        }
        else {
            Symbol* sym = read_nonterm(in);
            if (!sym) {
                return false;
            }
            rule->add(sym);
        }
    }
    return true;
}

void
Parser::solve_first()
{
    bool found = false;
    do {
        found = false;
        for (auto& nonterm : nonterms) {
            nonterm.second->solve_first(&found);
        }
    } while (found);
}

void
Parser::solve_follows()
{
    if (first) {
        first->nonterm->insert_follows(&Symbol::Endmark);
    }

    bool found = false;
    do {
        found = false;
        for (auto& nonterm : nonterms) {
            nonterm.second->solve_follows(&found);
        }
    } while (found);
}
