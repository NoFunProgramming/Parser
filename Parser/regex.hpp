/*******************************************************************************
 * Converting regular expressions into finite automata for finding patterns in
 * strings.  After building, the Regex object will contain a non-deterministic
 * finite automaton (NFA).  This NFA can be used at run time to find a pattern
 * or help construct a deterministic finite automaton (DFA) for a lexer.
 */

#ifndef regex_hpp
#define regex_hpp

#include "finite.hpp"

#include <string>
#include <iostream>
using std::string;
using std::istream;
using std::unique_ptr;

/*******************************************************************************
 * Contains a finite automaton for pattern matching.  Build a new regex object
 * by calling parse with a regular expression.  After building, get the start
 * state and call its scan method to check for a match.
 */
class Regex
{
  public:
    /**
     * Returns a new Regex object if the input expression is a valid, otherwise
     * a null pointer is returned.
     */
    static unique_ptr<Regex> parse(const string& in, Accept* accept);
    Regex();

    /**
     * First state in the finite automaton.  Call the scan method of the
     * returned finite state to check for a match in a string.
     */
    Finite* get_start();

  private:
    /** States of the NFA, owned by the regex object. */
    Finite* start;
    vector<unique_ptr<Finite>> states;
    Finite* add_state();
    Finite* add_state(Accept* accept);
    
    /**
     * Recursive decent parsing methods.  Each methods returns the first state
     * of the NFA that implements a subset of the regular expression and a list
     * of unconnected outputs.  As states are allocated they are added to the
     * state vector of the Regex object.
     */
    Finite* parse_expr(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_term(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_fact(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom(istream& in, vector<Finite::Out*>* outs);
    
    /**
     * Additional methods to find characters in a range, outside of a range, or
     * look for a control character in the expression.
     */
    Finite* parse_atom_range(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom_not(istream& in, vector<Finite::Out*>* outs);
    Finite* parse_atom_escape(istream& in, vector<Finite::Out*>* outs);
};

#endif
