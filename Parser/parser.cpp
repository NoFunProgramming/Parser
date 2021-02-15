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
    while (true) {
        in >> std::ws;
        if (in.peek() == EOF) {
            break;
        }
        
        if (in.peek() == '\'') {
            if (!read_term(in)) {
                return false;
            }
        } else {
            if (!read_rules(in)) {
                return false;
            }
        }
    }
    return true;
}

void
Parser::solve()
{
    lexer.solve();
    
    solve_first();
    solve_follows(&endmark);
    
    auto state = make_unique<State>(states.size());
    state->add(State::Item(first->rules.front().get(), 0, &endmark));
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

    Nonterm::Rule* rule = first->rules.front().get();
    State::Item accept(rule, rule->product.size(), &endmark);

    for (auto& state : states) {
        state->solve_actions(accept);
        state->solve_gotos();
    }
}

void
Parser::write(ostream& out) const
{
    out << "#include \"parser.hpp\"\n";
    out << "#include \"values.hpp\"\n";
    out << "#include <memory>\n";
    out << "using std::unique_ptr;\n";
    out << "\n";
    
    for (auto& term : terms) {
        term.second->write_proto(out);
    }
    out << "\n";
    
    for (auto& term : terms) {
        term.second->write_define(out);
    }
    out << "\n";

    
    for (auto& term : terms) {
        out << "Term ";
        term.second->write_declare(out);
        out << ";\n";
    }
    out << "\n";
    
    lexer.write(out);
    
    out << "Symbol endmark;\n";
    size_t id = 0;
    for (auto& nonterm : nonterms) {
        nonterm.second->rank = id++;
        out << "Nonterm ";
        nonterm.second->write(out);
        out << ";\n";
    }
    out << "\n";
    
    for (auto& nonterm : all) {
        for (auto& rule : nonterm->rules) {
            rule->write_proto(out);
        }
    }
    out << "\n";
    
    
    id = 0;
    for (auto& nonterm : nonterms) {
        for (auto& rule : nonterm.second->rules) {
            rule->id = id++;
            out << "Rule rule" << rule->id;
            out << " = {" << rule->product.size() << ", &";
            rule->nonterm->write(out);
            out << ", ";
            if (!rule->action.empty()) {
                out << "&" << rule->action;
            } else {
                out << "nullptr";
            }
            out << "};\n";
        }
    }
    out << "\n";
    
    id = 0;
    for (auto& state : states) {
        state->id = id++;
        out << "extern State state"  << state->id << ";\n";
    }
    out << "\n";
    
    for (auto& state : states) {
        state->write_shift(out);
        state->write_accept(out);
        state->write_reduce(out);
        state->write_goto(out);
    }
    out << "\n";

    for (auto& state : states) {
        state->write_define(out);
    }
    out << "\n";

    for (auto& nonterm : all) {
        for (auto& rule : nonterm->rules) {
            rule->write_action(out);
        }
    }
    out << "\n";

    for (auto& nonterm : all) {
        for (auto& rule : nonterm->rules) {
            rule->write_define(out);
        }
    }
    out << "\n";
}

void
Parser::print_grammar(ostream& out) const
{
    for (auto nonterm : all) {
        nonterm->print_rules(out);
        out << "\n";
    }
    out << "Firsts:\n";
    for (auto nonterm : all) {
        nonterm->print_firsts(out);
        out << "\n";
    }
    out << "\n";
    out << "Follows:\n";
    for (auto nonterm : all) {
        nonterm->print_follows(out);
        out << "\n";
    }
    out << "\n";
}

void
Parser::print_states(ostream& out) const
{
    // TODO Index the state at build time.
    size_t id = 0;
    for (auto& state : states) {
        state->id = id++;
    }
    for (auto& state : states) {
        state->print(out);
        state->print_items(out);
        out << "\n";
    }
}

/**
 * Reading the definition a new terminal.
 */
Term*
Parser::intern_term(istream& in)
{
    string name;
    if (!read_term_name(in, &name)) {
        return nullptr;
    }
    if (terms.count(name) == 0) {
        
        size_t id = terms.size();
        
        // TODO Store all of the accept values.
        Accept* accept = new Accept(name, id);
        //lexer.add(accept, name);
        lexer.add_series(accept, name);
        terms[name] = make_unique<Term>(name, id);
    }
    return terms[name].get();
}

Nonterm*
Parser::intern_nonterm(istream& in)
{
    string name;
    if (!read_nonterm_name(in, &name)) {
        return nullptr;
    }
    if (nonterms.count(name) == 0) {
        nonterms[name] = make_unique<Nonterm>(name);
    }
    return nonterms[name].get();
}

/**
 * Reading the definition a new terminal.
 */
bool
Parser::read_term(istream& in)
{
    string name;
    if (!read_term_name(in, &name)) {
        return false;
    }
    string type;
    if (!read_type(in, &type)) {
        return false;
    }
    string regex;
    if (!read_regex(in, &regex)) {
        return false;
    }
    string action;
    if (!read_action(in, &action)) {
        return false;
    }

    in >> std::ws;
    if (in.get() != ';') {
        cerr << "Terminals end with a semicolon.\n";
        return false;
    }
    
    if (terms.count(name) != 0) {
        cerr << "Terminal " << name << " was already defined.\n";
        return false;
    }
    
    size_t id = terms.size();
    
    terms[name] = make_unique<Term>(name, id);
    Term* term = terms[name].get();
    term->type = type;
    term->action = action;
    
    Accept* accept = new Accept(term->name, id);
    
    if (!regex.empty()) {
        lexer.add(accept, regex);
    } else {
        // TODO Change to the series.
        lexer.add(accept, term->name);
    }

    return true;
}

/******************************************************************************/
bool
Parser::read_rules(istream& in)
{
    Nonterm* nonterm = intern_nonterm(in);
    if (!nonterm) {
        cerr << "Rule must start with a nonterminal.\n";
        return false;
    }
    all.push_back(nonterm);
    
    string type;
    if (!read_type(in, &type)) {
        return false;
    }
    if (in.get() != ':') {
        cerr << "Expected a colon before the production.\n";
        return false;
    }
    // TODO Pass during the constructor.
    nonterm->type = type;
    
    if (!first) {
        first = nonterm;
    }
    
    vector<Symbol*> syms;
        
    while (in.peek() != EOF) {
        if (!read_product(in, &syms)) {
            return false;
        }
        string reduce;
        if (!read_action(in, &reduce)) {
            return false;
        }

        in >> std::ws;
        if (in.peek() == ';') {
            in.get();
            nonterm->add_rule(syms, reduce);
            break;
        } else if (in.peek() == '|') {
            in.get();
            nonterm->add_rule(syms, reduce);
            syms.clear();
        }
    }
    return true;
}

bool
Parser::read_product(istream& in, vector<Symbol*>* syms)
{
    while (in.peek() != EOF) {
        in >> std::ws;
        if (in.peek() == ';' || in.peek() == '|' || in.peek() == '&') {
            break;
        }
        
        if (in.peek() == '\'') {
            Term* sym = intern_term(in);
            if (sym) {
                syms->push_back(sym);
            } else {
                return false;
            }
        } else {
            Nonterm* sym = intern_nonterm(in);
            if (sym) {
                syms->push_back(sym);
            } else {
                return false;
            }
        }
    }
    return true;
}

/******************************************************************************/
bool
Parser::read_term_name(istream& in, string* name)
{
    if (in.get() != '\'') {
        cerr << "Expected quote to start terminal.\n";
        return false;
    }
    while (isprint(in.peek()) && in.peek() != '\'') {
        name->push_back(in.get());
    }
    if (in.get() != '\'') {
        cerr << "Expected quote to end terminal.\n";
        return false;
    }
    if (name->empty()) {
        cerr << "Terminals require at least one character.\n";
        return false;
    }
    return true;
}

bool
Parser::read_nonterm_name(istream& in, string* name)
{
    while (isalpha(in.peek())) {
        name->push_back(in.get());
    }
    if (name->empty()) {
        cerr << "Nonterminals require at least one character.\n";
        return false;
    }
    return true;
}

/******************************************************************************/
bool
Parser::read_type(istream& in, string* type)
{
    in >> std::ws;
    if (in.peek() == '<') {
        in.get();
        while (isalpha(in.peek())) {
            type->push_back(in.get());
        }
        if (in.get() != '>') {
            cerr << "Expected '>' after terminal type.\n";
            return false;
        }
    }
    return true;
}

bool
Parser::read_regex(istream& in, string* regex)
{
    in >> std::ws;
    if (in.peek() == '&' || in.peek() == ';') {
        return true;
    }

    while (true) {
        int c = in.peek();
        if (c == ' ' || c == ';') {
            break;
        }

        if (isprint(c)) {
            regex->push_back(in.get());
        } else {
            cerr << "Unexpected character in regex expression.\n";
        }
    }
    
    if (!regex->empty()) {
        return true;
    } else {
        cerr << "Regex pattern must have one character.\n";
        return false;
    }
}

bool
Parser::read_action(istream& in, string* action)
{
    in >> std::ws;
    if (in.peek() == '&') {
        in.get();
    } else {
        return true;
    }
        
    while (true) {
        int c = in.peek();
        if (c == ' ' || c == ';') {
            break;
        }
        
        if (isalpha(c)) {
            action->push_back(in.get());
        } else if (c == '_') {
            action->push_back(in.get());
        } else {
            cerr << "Unexpected character in action name.\n";
        }
    }
    
    if (!action->empty()) {
        return true;
    } else {
        cerr << "Action name must have one character.\n";
        return false;
    }
}

/******************************************************************************/
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
Parser::solve_follows(Symbol* endmark)
{
    if (!first || first->rules.size() == 0) {
        return;
    }
        
    Nonterm::Rule* rule = first->rules.front().get();
    rule->nonterm->follows.insert(endmark);

    bool found = false;
    do {
        found = false;
        for (auto& nonterm : nonterms) {
            nonterm.second->solve_follows(&found);
        }
    } while (found);
}

