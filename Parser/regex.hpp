#ifndef regex_hpp
#define regex_hpp

#include "finite.hpp"

#include <string>
#include <iostream>
using std::string;
using std::istream;
using std::unique_ptr;

class Regex
{
  public:
    Regex();
    static unique_ptr<Regex> parse(const string& in, Accept* accept);

    Finite* get_start() { return start; }

  private:
    Finite* start;
    
    vector<unique_ptr<Finite>> states;
    Finite* add_state();
    Finite* add_state(Accept* accept);
    
    Finite* parse_term(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_fact(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom(istream& in, vector<Finite::Out*>* outs);
};

#endif
