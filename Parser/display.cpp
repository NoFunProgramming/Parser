#include "display.hpp"

void
Display::print(const Lexer& lexer, std::ostream& out)
{
    for (auto& state : lexer.primes) {
        print(*state, out);
    }
}

void
Display::print(const Lexer::State& state, std::ostream& out)
{
    out << "State " << state.id;
    if (state.accept) {
        out << " (" << state.accept->name << ")";
    }
    out << "\n";
    for (auto& next : state.nexts) {
        print(next.first, out);
        out << " >> " << next.second->id << "\n";
    }
    out << "\n";
}

void
Display::print(const Lexer::State::Range& range, std::ostream& out)
{
    char first = range.first;
    char last  = range.last;
    
    if (first == last) {
        if (isprint(first) && first != '\'') {
            out << (char)first;
        } else {
            out << first;
        }
    } else {
        if (isprint(first) && isprint(last)
                && first != '\'' && last != '\'') {
            out << (char)first << " - " << (char)last;
        } else {
            out << first << " - " << last;
        }
    }
}


void
Display::print(const Grammar& grammar, std::ostream& out)
{
    print_terms(grammar, out);
}

void
Display::print_terms(const Grammar& grammar, std::ostream& out)
{
    out << "  ";
    for (auto& term : grammar.terms) {
        out << term.second->name << " ";
    }
    out << "$  ";
    for (auto& nonterm : grammar.nonterms) {
        out << nonterm.second->name << " ";
    }

    
    
    out << "\n";
    for (auto& state : grammar.states) {
        print_state(grammar, *state, out);
        out << "\n";
    }
}




void
Display::print_state(const Grammar& grammar,
                     const State& state,
                     std::ostream& out)
{
    State::Actions* actions = state.actions.get();
    
    out << state.id << ")";
    for (auto& term : grammar.terms) {
        auto found = actions->shift.find(term.second.get());
        if (found != actions->shift.end()) {
            out << "S" << found->second->id << " ";
            continue;
        }
        auto reduce = actions->reduce.find(term.second.get());
        if (reduce != actions->reduce.end()) {
            out << "R" << reduce->second->id << " ";
            continue;
        }
        out << "   ";
    }
    auto reduce = actions->reduce.find(&grammar.endmark);
    if (reduce != actions->reduce.end()) {
        out << "R" << reduce->second->id << " ";
    }
    
    for (auto& nonterm : grammar.nonterms) {
        auto found = state.gotos.find(nonterm.second.get());
        if (found != state.gotos.end()) {
            out << found->second->id << " ";
            continue;
        }
        out << "   ";
    }
}

void
Display::print_actions(const Grammar& grammar, std::ostream& out)
{
    size_t len = max_length(grammar);
    std::string name;
    name = "";
    name.insert(name.end(), len - name.size(), ' ');
    out << name << " ";
    for (auto& state : grammar.states) {
        out << state->id << "  ";
    }
    out << "\n";
    
    for (auto& term : grammar.terms) {
        name = term.second->name;
        name.insert(name.end(), len - name.size(), ' ');
        out << name << " ";
        for (auto& state : grammar.states) {
            print_action(term.second.get(), state->actions.get(), out);
        }
        out << "\n";
    }
    name = "$";
    name.insert(name.end(), len - name.size(), ' ');
    out << name << " ";

    for (auto& state : grammar.states) {
        print_action(&grammar.endmark, state->actions.get(), out);
    }
    out << "\n";
    for (auto& nonterm : grammar.nonterms) {
        name = nonterm.second->name;
        name.insert(name.end(), len - name.size(), ' ');
        out << name << " ";
        for (auto& state : grammar.states) {
            print_goto(nonterm.second.get(), state.get(), out);
        }
        out << "\n";
    }
}

void
Display::print_action(const Symbol* symbol, State::Actions* actions, std::ostream& out)
{
    auto found = actions->shift.find(symbol);
    if (found != actions->shift.end()) {
        out << "s" << found->second->id << " ";
        return;
    }
    auto reduce = actions->reduce.find(symbol);
    if (reduce != actions->reduce.end()) {
        out << "r" << reduce->second->id << " ";
        return;
    }
    auto accept = actions->accept.find(symbol);
    if (accept != actions->accept.end()) {
        out << "a" << accept->second->id << " ";
        return;
    }
    out << "   ";
}

void
Display::print_goto(Symbol* symbol, State* state, std::ostream& out)
{
    auto found = state->gotos.find(symbol);
    if (found != state->gotos.end()) {
        out << found->second->id << " ";
        return;
    }
    out << "   ";
}

size_t
Display::max_length(const Grammar& grammar)
{
    size_t result = 0;
    for (auto& term : grammar.terms) {
        if (term.first.length() > result) {
            result = term.first.length();
        }
    }
    for (auto& nonterm : grammar.nonterms) {
        if (nonterm.first.length() > result) {
            result = nonterm.first.length();
        }
    }
    return result;
}
