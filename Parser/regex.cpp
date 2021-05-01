#include "regex.hpp"

#include <sstream>

/******************************************************************************/
Regex::Regex() :
    start(nullptr) {}

/**
 * Builds the NFA for the given regular expression using subset construction.
 * All unconnected outputs from the final automaton are connect to a provided
 * accept condition.
 */
std::unique_ptr<Regex>
Regex::parse(const std::string& in, Accept* accept)
{
    std::unique_ptr<Regex> result(std::make_unique<Regex>());
    
    std::istringstream input(in);
    
    std::vector<Finite::Out*> outs;
    result->start = result->parse_expr(input, &outs);
    
    if (result->start) {
        Finite* target = result->add_state(accept);
        for (Finite::Out* out : outs) {
            out->next = target;
        }
    } else {
        std::cerr << "Unable to parse expression '" << in << "'.\n";
        result.reset();
    }

    return result;
}

Finite* Regex::get_start() { return start; }

/**
 * Builds a new state and retains ownership.  No memory leaks occur if any
 * exceptions or errors occur during subset construction, as the states vector
 * contains all fragments of the automaton.
 */
Finite*
Regex::add_state() {
    states.emplace_back(std::make_unique<Finite>());
    return states.back().get();
}

Finite*
Regex::add_state(Accept* accept) {
    states.emplace_back(std::make_unique<Finite>(accept));
    return states.back().get();
}

/** Parses the lowest precedence operator, the vertical bar. */
Finite*
Regex::parse_expr(std::istream& in, std::vector<Finite::Out*>* outs)
{
    Finite* expr = add_state();
    
    while (in.peek() != EOF)
    {
        Finite* term = parse_term(in, outs);
        if (!term) {
            return nullptr;
        }

        expr->add_epsilon(term);
        if (in.peek() == '|') {
            in.get();
        } else {
            break;
        }
    }
    
    return expr;
}

/** Parses a list of character terminals that are in a row between bars. */
Finite*
Regex::parse_term(std::istream& in, std::vector<Finite::Out*>* outs)
{
    std::vector<Finite::Out*> fact_in;
    std::vector<Finite::Out*> fact_out;
    Finite* term = parse_fact(in, &fact_out);
    if (!term) {
        return nullptr;
    }
    
    while (true)
    {
        int c = in.peek();
        if (c == EOF || c == ')' || c == '|') {
            break;
        }
        
        fact_in = fact_out;
        fact_out.clear();
        
        Finite* fact = parse_fact(in, &fact_out);
        if (!fact) {
            return nullptr;
        }
        
        for (Finite::Out* input : fact_in) {
            input->next = fact;
        }
    }

    outs->insert(outs->end(), fact_out.begin(), fact_out.end());
    return term;
}

/** Parses the operators, + * ?, for repeated characters. */
Finite*
Regex::parse_fact(std::istream& in, std::vector<Finite::Out*>* outs)
{
    std::vector<Finite::Out*> atom_outs;
    Finite* atom = parse_atom(in, &atom_outs);
    if (!atom) {
        return nullptr;
    }
    
    int c = in.peek();
    if (c != '+' && c != '*' && c != '?') {
        outs->insert(outs->end(), atom_outs.begin(), atom_outs.end());
        return atom;
    }
    
    Finite* state = add_state();
    state->add_epsilon(atom);
    Finite::Out* out2 = state->add_epsilon(nullptr);

    outs->push_back(out2);
    
    switch (in.get()) {
        case '+': {
            for (Finite::Out* out : atom_outs) {
                out->next = state;
            }
            return atom;
        }
        case '*': {
            for (Finite::Out* out : atom_outs) {
                out->next = state;
            }
            return state;
        }
        case '?': {
            outs->insert(outs->end(), atom_outs.begin(), atom_outs.end());
            return state;
        }
        default: {
            return nullptr;
        }
    }
}

/** Parses a single or a range of characters. */
Finite*
Regex::parse_atom(std::istream& in, std::vector<Finite::Out*>* outs)
{
    int c = in.get();
    if (c == EOF) {
        std::cerr << "Unexpected end of file.\n";
        return nullptr;
    }
    
    if (isalpha(c) || isdigit(c)) {
        Finite* state = add_state();
        Finite::Out* out = state->add_out(c, nullptr);
        outs->push_back(out);
        return state;
    }
    else if (c == '[') {
        if (in.peek() == '^') {
            in.get();
            return parse_atom_not(in, outs);
        } else {
            return parse_atom_range(in, outs);
        }
    }
    else if (c == '\\') {
        return parse_atom_escape(in, outs);
    }
    else if (c == '(') {
        Finite* expr = parse_expr(in, outs);
        if (!expr) {
            return nullptr;
        }
        if (in.get() != ')') {
            std::cerr << "Expected ')' to end expression.\n";
            return nullptr;
        }
        return expr;
    }
    else if (c == ']' || c == ')' || c == '|') {
        std::cerr << "Unexpected '" << (char)c << "' in expression.\n";
        return nullptr;
    }
    else if (isprint(c)) {
        Finite* state = add_state();
        Finite::Out* out = state->add_out(c, nullptr);
        outs->push_back(out);
        return state;
    }
    else {
        if (c == EOF) {
            std::cerr << "Unexpected end of file.\n";
        } else if (isprint(c)) {
            std::cerr << "Unexpected '" << (char)c << "' in expression.\n";
        } else {
            std::cerr << "Unexpected << c << in expression.\n";
        }
        return nullptr;
    }
}

/** Parses a range of characters, [a-z]. */
Finite*
Regex::parse_atom_range(std::istream& in, std::vector<Finite::Out*>* outs)
{
    int first = in.get();
    if (!isalpha(first) && !isdigit(first)) {
        std::cerr << "Expected a letter or number to start range.\n";
        return nullptr;
    }
    if (in.get() != '-') {
        std::cerr << "Expected a '-' to separate range.\n";
        return nullptr;
    }
    
    int last = in.get();
    if (!isalpha(last) && !isdigit(last)) {
        std::cerr << "Expected a letter or number to end range.\n";
        return nullptr;
    }
    if (in.get() != ']') {
        std::cerr << "Expected a ']' to end range.\n";
        return nullptr;
    }
    
    Finite* state = add_state();
    Finite::Out* out = state->add_out(first, last, nullptr);
    outs->push_back(out);
    return state;
}

/** Parses not within range of characters, [^a] or [^a-z]. */
Finite*
Regex::parse_atom_not(std::istream& in, std::vector<Finite::Out*>* outs)
{
    int first = in.get();
    if (!isalpha(first) && !isdigit(first)) {
        std::cerr << "Expected a letter or number after not.\n";
        return nullptr;
    }
    int last = first;
    if (in.peek() == '-') {
        in.get();
        last = in.get();
        if (!isalpha(last) && !isdigit(last)) {
            std::cerr << "Expected a letter or number to end range.\n";
            return nullptr;
        }
    }
    if (in.get() != ']') {
        std::cerr << "Expected a ']' to end range.\n";
        return nullptr;
    }
    
    Finite* state = add_state();
    Finite::Out* out = state->add_not(first, last, nullptr);
    outs->push_back(out);
    return state;
}

// TODO Allow escape sequnces in ranges.

/** Parses an escape sequence, \[ \(, to match control characters. */
Finite*
Regex::parse_atom_escape(std::istream& in, std::vector<Finite::Out*>* outs)
{
    int c = in.get();
    
    switch (c) {
        case '[': break;
        case ']': break;
        case '(': break;
        case ')': break;
        case '|': break;
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
            if (c == EOF) {
                std::cerr << "Unexpected end of file.\n";
            } else if (isprint(c)) {
                std::cerr << "Unknown escape sequence '" << (char)c << "'.\n";
            } else {
                std::cerr << "Unexpected control character.\n";
            }
            return nullptr;
        }
    }

    Finite* state = add_state();
    Finite::Out* out = state->add_out(c, nullptr);
    outs->push_back(out);
    return state;
}
