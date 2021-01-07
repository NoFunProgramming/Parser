#include "regex.hpp"

#include <sstream>
using std::make_unique;
using std::istringstream;

Regex::Regex() :
    start(nullptr) {}

unique_ptr<Regex>
Regex::parse(const string& in, Accept* accept)
{
    unique_ptr<Regex> result(make_unique<Regex>());
    
    istringstream input(in);
    
    vector<Finite::Out*> outs;
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

Finite*
Regex::add_state() {
    states.emplace_back(make_unique<Finite>());
    return states.back().get();
}

Finite*
Regex::add_state(Accept* accept) {
    states.emplace_back(make_unique<Finite>(accept));
    return states.back().get();
}

Finite*
Regex::parse_expr(istream& in, vector<Finite::Out*>* outs)
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

Finite*
Regex::parse_term(istream& in, vector<Finite::Out*>* outs)
{
    vector<Finite::Out*> fact_in;
    vector<Finite::Out*> fact_out;
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

Finite*
Regex::parse_fact(istream& in, vector<Finite::Out*>* outs)
{
    vector<Finite::Out*> atom_outs;
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

Finite*
Regex::parse_atom(istream& in, vector<Finite::Out*>* outs)
{
    int c = in.get();
    if (c == EOF) {
        std::cerr << "Unexpected end of file.\n";
        return nullptr;
    }
    
    if (isalpha(c)) {
        Finite* state = add_state();
        Finite::Out* out = state->add_out(c, nullptr);
        outs->push_back(out);
        return state;
    }
    else if (c == '[') {
        return parse_atom_range(in, outs);
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
            cerr << "Expected ')' to end expression.\n";
            return nullptr;
        }
        return expr;
    }
    else {
        if (c == EOF) {
            cerr << "Unexpected end of file.\n";
        } else if (isprint(c)) {
            cerr << "Unexpected '" << (char)c << "' in expression.\n";
        } else {
            cerr << "Unexpected << c << in expression.\n";
        }
        return nullptr;
    }
}

Finite*
Regex::parse_atom_range(istream& in, vector<Finite::Out*>* outs)
{
    int first = in.get();
    if (!isalpha(first) && !isdigit(first)) {
        cerr << "Expected a letter or number to start range.\n";
        return nullptr;
    }
    if (in.get() != '-') {
        cerr << "Expected a '-' to separate range.\n";
        return nullptr;
    }
    
    int last = in.get();
    if (!isalpha(last) && !isdigit(last)) {
        cerr << "Expected a letter or number to end range.\n";
        return nullptr;
    }
    if (in.get() != ']') {
        cerr << "Expected a ']' to end range.\n";
        return nullptr;
    }
    
    Finite* state = add_state();
    Finite::Out* out = state->add_out(first, last, nullptr);
    outs->push_back(out);
    return state;
}

Finite*
Regex::parse_atom_escape(istream& in, vector<Finite::Out*>* outs)
{
    int c = in.get();
    
    if (c == '[' || c == ']' || c == '(' || c == ')'
            || c == '|' || c == '\\') {
        Finite* state = add_state();
        Finite::Out* out = state->add_out(c, nullptr);
        outs->push_back(out);
        return state;
    }
    else {
        if (c == EOF) {
            cerr << "Unexpected end of file.\n";
        } else if (isprint(c)) {
            cerr << "Unknown escape sequence '" << (char)c << "'.\n";
        } else {
            cerr << "Unexpected control character.\n";
        }
        return nullptr;
    }
}

Finite*
Regex::parse_atom_hex(istream& in, vector<Finite::Out*>* outs)
{
    if (isdigit(in.peek()) {
        char first = in.get();
    }
}

