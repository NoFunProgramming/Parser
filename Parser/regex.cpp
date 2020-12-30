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
    result->start = result->parse_term(input, &outs);
    
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
Regex::parse_term(istream& in, vector<Finite::Out*>* outs)
{
    vector<Finite::Out*> fact_in;
    vector<Finite::Out*> fact_out;
    Finite* term = parse_atom(in, &fact_out);
    if (!term) {
        return nullptr;
    }
    
    while (true)
    {
        int c = in.peek();
        if (c == EOF) {
            break;
        }
        
        fact_in = fact_out;
        fact_out.clear();
        
        Finite* fact = parse_atom(in, &fact_out);
        if (!fact) {
            return nullptr;
        }
        
        for (Finite::Out* input : fact_in) {
            input->next = fact;
        }
    }

    return term;
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
    else {
        std::cerr << "Unexpected character.\n";
        return nullptr;
    }
}
