/*******************************************************************************
 * Writes the parse table for a grammar.  The generated parse table can be
 * combined with custom methods for each grammar rule to build parsing functions
 * or compilers.
 */

#ifndef code_hpp
#define code_hpp

#include "grammar.hpp"
#include <iostream>
#include <vector>
using std::vector;
using std::ostream;

/*******************************************************************************
 * Writes source code that defines the parse table for a grammar.  The code
 * provides functions to find the next action based on the current parse state
 * and given input symbol.
 */
class Code
{
  public:
    static void write(const Grammar& grammar, std::ostream& out);
    
    /** After solving for the DFA, call write to generate the source code. */
    static void write(const Lexer& lexer, std::ostream& out);

  private:
    static void write_terms(Term* term, std::ostream& out);
    static void write_eval(Term* term, std::ostream& out);
    static void write_scan(Node* node, std::ostream& out);
    static void write_node(Node* node, std::ostream& out);

    static void write_nonterm(Nonterm* nonterm, std::ostream& out);
    static void write_action(Nonterm::Rule* rule, std::ostream& out);
    static void write_action_call(Nonterm::Rule* rule, std::ostream& out);
    static void write_rules(const Grammar& grammar, ostream& out);

    static void write_actions(vector<State*> states, ostream& out);
    static void write_gotos(vector<State*> states, ostream& out);
    static void write_states(vector<State*> states, ostream& out);
};

#endif
