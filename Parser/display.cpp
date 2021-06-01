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
