#include "grammar.hpp"

#include <iostream>

using std::string;
using std::vector;
using std::istream;
using std::make_unique;

/******************************************************************************/
Grammar::Grammar():
    start(nullptr){}

bool
Grammar::read_grammar(std::istream& in)
{
    while (true) {
        in >> std::ws;
        if (in.peek() == EOF) {
            break;
        }
        if (!read_comment(in)) {
            return false;
        }
                
        in >> std::ws;
        if (in.peek() == '\'') {
            if (!read_term(in)) {
                return false;
            }
        }
        else if (in.peek() == '#') {
            if (!read_include(in)) {
                return false;
            }
        }
        else {
            if (!read_rules(in)) {
                return false;
            }
        }
    }
    return true;
}

/******************************************************************************/
void
Grammar::solve_states()
{
    if (all.size() == 0 || all.front()->rules.size() == 0) {
        return;
    }
    
    lexer.solve();
    
    solve_first();
    solve_follows(&endmark);
    
    auto state = std::make_unique<State>(states.size());
    state->add(State::Item(all.front()->rules.front().get(), 0, &endmark));
    state->closure();
    
    start = state.get();
    states.insert(std::move(state));
    
    std::vector<State*> checking;
    checking.push_back(start);

    while (checking.size() > 0)
    {
        State* state = checking.back();
        checking.pop_back();

        for (auto& term : terms) {
            Symbol* sym = term.second.get();
            std::unique_ptr<State> next = state->solve_next(sym, states.size());
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
            std::unique_ptr<State> next = state->solve_next(sym, states.size());
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

    Nonterm::Rule* rule = all.front()->rules.front().get();
    State::Item accept(rule, rule->product.size(), &endmark);

    for (auto& state : states) {
        state->solve_actions(accept);
        state->solve_gotos();
    }
}

/******************************************************************************/
void
Grammar::print_grammar(std::ostream& out) const
{
    for (auto nonterm : all) {
        nonterm->print_rules(out);
        out << std::endl;
    }
    out << std::endl;
    
    out << "Firsts:\n";
    for (auto nonterm : all) {
        nonterm->print_firsts(out);
        out << std::endl;
    }
    out << std::endl;
    
    out << "Follows:\n";
    for (auto nonterm : all) {
        nonterm->print_follows(out);
        out << std::endl;
    }
    out << std::endl;
}

void
Grammar::print_states(std::ostream& out) const
{
    for (auto& state : states) {
        state->print(out);
        state->print_items(out);
        out << std::endl;
    }
}

/******************************************************************************/
Term*
Grammar::intern_term(std::istream& in)
{
    string name;
    if (!read_term_name(in, &name)) {
        return nullptr;
    }
    if (terms.count(name) == 0) {
        terms[name] = std::make_unique<Term>(name, terms.size());
        Term* term = terms[name].get();

        accepts.emplace_back(std::make_unique<Accept>(name, term->rank));
        lexer.add_literal(accepts.back().get(), name);
    }
    return terms[name].get();
}

Nonterm*
Grammar::intern_nonterm(istream& in)
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

/******************************************************************************/
bool
Grammar::read_term(istream& in)
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
        std::cerr << "Terminals end with a semicolon.\n";
        return false;
    }
        
    if (terms.count(name) == 0) {
        terms[name] = make_unique<Term>(name, terms.size());
    }
    Term* term = terms[name].get();
    term->type = type;
    term->action = action;
    
    accepts.emplace_back(make_unique<Accept>(term->name, term->rank));
    Accept* accept = accepts.back().get();
    
    // TODO Check this location.
    accept->scan = action;
    
    if (!regex.empty()) {
        lexer.add_regex(accept, regex);
    } else {
        lexer.add_literal(accept, term->name);
    }

    return true;
}

/******************************************************************************/
bool
Grammar::read_rules(istream& in)
{
    string name;
    if (!read_nonterm_name(in, &name)) {
        std::cerr << "Rule must start with a nonterminal.\n";
        return false;
    }
    string type;
    if (!read_type(in, &type)) {
        return false;
    }
    if (in.get() != ':') {
        std::cerr << "Expected a colon after the nonterminal name.\n";
        return false;
    }
    
    if (nonterms.count(name) == 0) {
        nonterms[name] = make_unique<Nonterm>(name);
    }
    Nonterm* nonterm = nonterms[name].get();
    nonterm->type = type;
    //nonterm->rank = all.size();
    all.push_back(nonterm);
    
    vector<Symbol*> syms;
        
    while (in.peek() != EOF) {
        if (!read_product(in, &syms)) {
            return false;
        }
        string action;
        if (!read_action(in, &action)) {
            return false;
        }

        in >> std::ws;
        if (in.peek() == ';') {
            in.get();
            Nonterm::Rule* rule = nonterm->add_rule(syms, action);
            rule->id = all_rules.size();
            all_rules.push_back(rule);
            break;
        }
        else if (in.peek() == '|') {
            in.get();
            Nonterm::Rule* rule = nonterm->add_rule(syms, action);
            rule->id = all_rules.size();
            all_rules.push_back(rule);
            syms.clear();
        }
    }
    return true;
}

bool
Grammar::read_product(istream& in, vector<Symbol*>* syms)
{
    while (in.peek() != EOF) {
        in >> std::ws;
        if (in.peek() == ';'  || in.peek() == '\n'
                || in.peek() == '|' || in.peek() == '&') {
            break;
        }
        
        int c = in.peek();
        if (c == '\'') {
            Term* sym = intern_term(in);
            if (sym) {
                syms->push_back(sym);
            } else {
                return false;
            }
        } else if (isalpha(c)) {
            Nonterm* sym = intern_nonterm(in);
            if (sym) {
                syms->push_back(sym);
            } else {
                return false;
            }
        } else {
            std::cerr << "Expected character in rule.\n";
            return false;
        }
    }
    return true;
}

bool
Grammar::read_include(istream& in)
{
    std::string text;
    while (in.peek() != EOF) {
        int c = in.get();
        if (c == '\n') {
            break;
        } else if (isprint(c)) {
            text.push_back(c);
        } else {
            return false;
        }
    }
    includes.push_back(text);
    return true;
}

bool
Grammar::read_comment(istream& in)
{
    if (in.peek() != '/') {
        return true;
    }

    int c = in.get();
    if (in.peek() != '*') {
        in.putback(c);
        return true;
    } else {
        in.get();
    }
    
    bool star = false;
    while (true) {
        if (in.peek() == EOF) {
            std::cerr << "Unexpected end of file in comment.\n";
            return false;
        }
        if (star) {
            int c = in.get();
            if (c == '/') {
                break;
            } else if (c != '*') {
                star = false;
            }
        } else {
            if (in.get() == '*') {
                star = true;
            }
        }
    }
    
    return true;
}

/******************************************************************************/
bool
Grammar::read_term_name(istream& in, string* name)
{
    if (in.get() != '\'') {
        std::cerr << "Expected quote to start terminal name.\n";
        return false;
    }
    while (isprint(in.peek()) && in.peek() != '\'') {
        name->push_back(in.get());
    }
    if (in.get() != '\'') {
        std::cerr << "Expected quote to end terminal name.\n";
        return false;
    }
    if (name->empty()) {
        std::cerr << "Terminal names require at least one character.\n";
        return false;
    }
    return true;
}

bool
Grammar::read_nonterm_name(istream& in, string* name)
{
    while (isalpha(in.peek())) {
        name->push_back(in.get());
    }
    if (name->empty()) {
        std::cerr << "Nonterminal names require at least one character.\n";
        return false;
    }
    return true;
}

/******************************************************************************/
bool
Grammar::read_type(istream& in, string* type)
{
    in >> std::ws;
    if (in.peek() == '<') {
        in.get();
    } else {
        return true;
    }
    
    while (true) {
        int c = in.peek();
        if (c == '>') {
            in.get();
            break;
        }

        if (isalpha(c)) {
            type->push_back(in.get());
        } else {
            std::cerr << "Unexpected character in type name.\n";
            return false;
        }
    }
    
    return true;
}

bool
Grammar::read_regex(istream& in, string* regex)
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
            std::cerr << "Unexpected character in regular expression.\n";
            return false;
        }
    }
    
    if (!regex->empty()) {
        return true;
    } else {
        std::cerr << "Regex pattern must have one character.\n";
        return false;
    }
}

bool
Grammar::read_action(istream& in, string* action)
{
    in >> std::ws;
    if (in.peek() == '&') {
        in.get();
    } else {
        return true;
    }
        
    while (true) {
        int c = in.peek();
        if (c == ' ' || c == ';' || c == '\n') {
            break;
        }
        
        if (isalpha(c)) {
            action->push_back(in.get());
        } else if (isdigit(c) && action->size() > 0) {
            action->push_back(in.get());
        } else if (c == '_') {
            action->push_back(in.get());
        } else {
            std::cerr << "Unexpected character in action method name.\n";
            return false;
        }
    }
    
    if (!action->empty()) {
        return true;
    } else {
        std::cerr << "Action name must have one character.\n";
        return false;
    }
}

/******************************************************************************/
void
Grammar::solve_first()
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
Grammar::solve_follows(Symbol* endmark)
{
    if (all.size() == 0 || all.front()->rules.size() == 0) {
        return;
    }
        
    Nonterm::Rule* rule = all.front()->rules.front().get();
    rule->nonterm->follows.insert(endmark);

    bool found = false;
    do {
        found = false;
        for (auto& nonterm : nonterms) {
            nonterm.second->solve_follows(&found);
        }
    } while (found);
}

