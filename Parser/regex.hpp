/*******************************************************************************
 * Converts regular expressions into non-deterministic finite automata (NFA) for
 * matching patterns in strings.  The newly built object contains the linked
 * states of a NFA, call the scan from the state state to check for a match.
 */
#ifndef regex_hpp
#define regex_hpp

#include "finite.hpp"
#include <iostream>

/******************************************************************************/
class Regex
{
  public:
    /** Returns the NFA if the expression is valid, otherwise null. */
    static std::unique_ptr<Regex> parse(const std::string& in, Accept* accept);

    /** After building, call start's scan method to check for a match. */
    Finite* start;
 
    Regex();

  private:
    std::vector<std::unique_ptr<Finite>> states;
    Finite* add_state();
    Finite* add_state(Accept* accept);
    
    /**
     * Recursive decent parsing methods.  Each methods returns the first state
     * of the NFA that implements a subset of the regular expression and a list
     * of unconnected outputs.  As states are allocated they are added to the
     * state vector of the Regex object.
     */
    Finite* parse_expr(std::istream& in, std::vector<Finite::Out*>* outs);
    Finite* parse_term(std::istream& in, std::vector<Finite::Out*>* outs);
    Finite* parse_fact(std::istream& in, std::vector<Finite::Out*>* outs);
    Finite* parse_atom(std::istream& in, std::vector<Finite::Out*>* outs);
    
    /**
     * Additional methods to find characters in a range, outside of a range, or
     * look for a control character in the expression.
     */
    Finite* parse_atom_range(std::istream& in, std::vector<Finite::Out*>* outs);
    Finite* parse_atom_not(std::istream& in, std::vector<Finite::Out*>* outs);
    Finite* parse_atom_escape(std::istream& in, std::vector<Finite::Out*>* outs);
};

#endif
