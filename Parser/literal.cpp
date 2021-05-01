#include "literal.hpp"

#include <sstream>

/******************************************************************************/
Literal::Literal():
    start(nullptr) {}

std::unique_ptr<Literal>
Literal::build(const std::string& series, Accept* accept)
{
    std::unique_ptr<Literal> result = std::make_unique<Literal>();
    
    std::istringstream input(series);
    
    result->start = result->parse_term(input, accept);
    
    if (!result->start) {
        std::cerr << "Unable to parse expression '" << series << "'.\n";
        result.reset();
    }

    return result;
}

/**
 * Builds a new state and retains ownership.  No memory leaks occur if any
 * exceptions or errors occur during subset construction, as the states vector
 * contains all fragments of the automaton.
 */
Finite*
Literal::add_state() {
    states.emplace_back(std::make_unique<Finite>());
    return states.back().get();
}

Finite*
Literal::add_state(Accept* accept) {
    states.emplace_back(std::make_unique<Finite>(accept));
    return states.back().get();
}

/**
 * Connects each character in the pattern as a sequence of finite states to
 * build the NFA.
 */
Finite*
Literal::parse_term(std::istream& series, Accept* accept)
{
    // TODO Handle the empty string.
    Finite* fact = add_state();
    Finite* term = fact;
    
    while (true)
    {
        int c = series.get();
        if (!isprint(c)) {
            std::cerr << "Expected a printable character.\n";
            return nullptr;
        }
        
        if (c == '\\') {
            c = series.get();
            switch (c) {
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'a': c = '\a'; break;
                case 'b': c = '\b'; break;
                case 'e': c = '\e'; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                case '\'': c = '\''; break;
                case '"':  c = '"' ; break;
                case '?':  c = '?' ; break;
                default: {
                    std::cerr << "Unknown escape sequence.\n";
                    return nullptr;
                }
            }
        }
        
        if (series.peek() == EOF) {
            Finite* next = add_state(accept);
            term->add_out(c, next);
            break;
        }
        else {
            Finite* next = add_state();
            term->add_out(c, next);
            term = next;
        }
    }
    
    return fact;
}
