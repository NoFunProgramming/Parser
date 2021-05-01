/*******************************************************************************
 * Writes the parse table for a grammar.  The generated parse table can be
 * combined with custom methods for each grammar rule to build parsing functions
 * or compilers.
 */

#ifndef table_hpp
#define table_hpp

#include "grammar.hpp"
#include <iostream>

/*******************************************************************************
 * Writes source code that defines the parse table for a grammar.  The code
 * provides functions to find the next action based on the current parse state
 * and given input symbol.
 */
class Table
{
  public:
    static void write(const Grammar& grammar, std::ostream& out);

  private:
    static void declare_structs(std::ostream& out);
    static void define_functions(std::ostream& out);

    static void declare_terms(Term* term, std::ostream& out);
    static void define_term_actions(Term* term, std::ostream& out);
    
    static void declare_nonterm(Nonterm* nonterm, std::ostream& out);
    static void declare_rule(Nonterm::Rule* rule, std::ostream& out);
    static void declare_action(Nonterm::Rule* rule, std::ostream& out);

    static void define_action(Nonterm::Rule* rule, std::ostream& out);
    static void define_action_call(Nonterm::Rule* rule, std::ostream& out);
    
    /** Each state contains shift and reduce actions for a given symbol. */
    static void declare_state (State* state, std::ostream& out);
    static void define_shifts (State* state, std::ostream& out);
    static void define_accepts(State* state, std::ostream& out);
    static void define_reduces(State* state, std::ostream& out);
    static void define_gotos  (State* state, std::ostream& out);
};

#endif
