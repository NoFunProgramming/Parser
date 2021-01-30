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
        if (in.peek() == '\'') {
            read_term(in);
        } else {
            read_rules(in);
        }
    }
    return true;
}

void
Parser::print_grammar(ostream& out) const
{
    for (auto& nonterm : nonterms) {
        nonterm.second->print_rules(out);
        out << "\n";
    }
    for (auto& nonterm : nonterms) {
        nonterm.second->print_firsts(out);
        out << "\n";
    }
    for (auto& nonterm : nonterms) {
        nonterm.second->print_follows(out);
        out << "\n";
    }
}

void
Parser::solve()
{
    lexer.solve();
    
    solve_first();
    solve_follows();
    
    auto state = make_unique<State>(states.size());
    state->add(State::Item(first, 0, &Symbol::Endmark));
    state->closure();
    
    start = state.get();
    states.insert(std::move(state));

    vector<State*> checking;
    checking.push_back(start);

    while (checking.size() > 0)
    {
        State* state = checking.back();
        checking.pop_back();

        for (auto& term : terms) {
            Symbol* sym = term.second.get();
            unique_ptr<State> next = state->solve_next(sym);
            if (next) {
                auto found = states.insert(std::move(next));
                State* target = found.first->get();
                state->add_next(sym, target);
                if (found.second) {
                    checking.push_back(target);
                }
            }
        }

        for (auto& nonterm : nonterms) {
            Symbol* sym = nonterm.second.get();
            unique_ptr<State> next = state->solve_next(sym);
            if (next) {
                auto found = states.insert(std::move(next));
                State* target = found.first->get();
                state->add_next(sym, target);
                if (found.second) {
                    checking.push_back(target);
                }
            }
        }
    }

    State::Item accept(first, first->product.size(), &Symbol::Endmark);

    for (auto& state : states) {
        state->solve_actions(accept);
        state->solve_gotos();
    }
}

static string header =
    "struct Symbol {};\n"
    "struct Term {};\n"
    "struct Nonterm {};\n"
    "struct Rule {};\n"
    "struct State {\n"
    "    State* (*shift)(Symbol*);\n"
    "    Rule* (*accept)(Symbol*);\n"
    "    Rule* (*reduce)(Symbol*);\n"
    "};\n";

void
Parser::write(ostream& out) const
{
    size_t id = 0;
    for (auto& term : terms) {
        term.second->id = id++;
        out << "Term term" << term.second->id << ";\n";
    }
    out << "\n";
    
    lexer.write(out);
    
    out << "Symbol endmark;\n";
    id = 0;
    for (auto& nonterm : nonterms) {
        nonterm.second->id = id++;
        out << "Nonterm nonterm" << nonterm.second->id << ";\n";
    }
    out << "\n";
    
    id = 0;
    for (auto& nonterm : nonterms) {
        for (auto& rule : nonterm.second->rules) {
            rule->id = id++;
            out << "Rule rule" << rule->id;
            out << " = {" << rule->product.size() << ", &";
            rule->nonterm->write(out);
            out << "};\n";
        }
    }
    out << "\n";
    
    id = 0;
    for (auto& state : states) {
        state->id = id++;
        state->write_declare(out);
    }
    out << "\n";
    
    for (auto& state : states) {
        state->write_shift(out);
        state->write_accept(out);
        state->write_reduce(out);
        state->write_next(out);
    }
    out << "\n";

    for (auto& state : states) {
        state->write_define(out);
    }
}

Term*
Parser::intern_term(istream& in)
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
        return nullptr;
    }
    
    if (terms.count(name) == 0) {
        Accept* accept = new Accept(name, terms.size());
        lexer.add(accept, name);
        terms[name] = make_unique<Term>(name);
    }
    
    
    return terms[name].get();
}

Nonterm*
Parser::intern_nonterm(istream& in)
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
Parser::read_term(istream& in)
{
    Term* term = intern_term(in);
    if (!term) {
        return false;
    }
    return true;
}

bool
Parser::read_rules(istream& in)
{
    Nonterm* nonterm = intern_nonterm(in);
    if (!nonterm) {
        cerr << "Unable to read nonterminal name.\n";
    }
    if (in.get() != ':') {
        cerr << "Nontermianals end with a colon.\n";
        return false;
    }
    
    Nonterm::Rule* rule = nonterm->add_rule();
    if (!first) {
        first = rule;
    }
    
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
            Symbol* sym = intern_term(in);
            if (!sym) {
                return false;
            }
            rule->add(sym);
        }
        else {
            Symbol* sym = intern_nonterm(in);
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

