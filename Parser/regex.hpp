#ifndef regex_hpp
#define regex_hpp

#include "finite.hpp"

#include <string>
#include <iostream>
using std::string;
using std::istream;
using std::unique_ptr;

/**
 * Regex contains a finite automata that matches a pattern defined by a regular
 * expression.
 */
class Regex
{
  public:
    /**
     * Returns a new Regex object if the input string is a valid, otherwise a
     * null pointer is returned.
     */
    static unique_ptr<Regex> parse(const string& in, Accept* accept);
    Regex();

    Finite* get_start() { return start; }

  private:
    Finite* start;
    
    /** The finite states of the NFA are owned by the regex object. */
    vector<unique_ptr<Finite>> states;
    Finite* add_state();
    Finite* add_state(Accept* accept);
    
    /**
     * Recursive decent parsing methods.  Each methods returns the first state
     * in the NFA and a list of unconnected outputs.  As states are allocated
     * they are added to, and owned by, the state vector of the Regex object.
     */
    Finite* parse_expr(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_term(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_fact(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom(istream& in, vector<Finite::Out*>* outs);
    
    /**
     * Additional methods to find characters in a range, outside of a range, or
     * look for a control character in the input.
     */
    Finite* parse_atom_range(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom_not(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom_escape(istream& in, vector<Finite::Out*>* outs);
};

#endif
