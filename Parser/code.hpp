/*******************************************************************************
 * Writes the parse table for a grammar.  The generated parse table can be
 * combined with custom methods for each grammar rule to build parsing functions
 * or compilers.
 */

#ifndef code_hpp
#define code_hpp

#include "grammar.hpp"
#include <iostream>

/*******************************************************************************
 * Writes source code that defines the parse table for a grammar.  The code
 * provides functions to find the next action based on the current parse state
 * and given input symbol.
 */
class Code
{
  public:
    static void write(const Grammar& grammar, std::ostream& out);

  private:
    static void declare_structs(std::ostream& out);

    static void declare_terms(Term* term, std::ostream& out);
    static void define_term_actions(Term* term, std::ostream& out);
    static void declare_nonterm(Nonterm* nonterm, std::ostream& out);

    static void define_action(Nonterm::Rule* rule, std::ostream& out);
    static void define_action_cast(Nonterm::Rule* rule, std::ostream& out);
    static void define_action_call(Nonterm::Rule* rule, std::ostream& out);
    static void define_functions(std::ostream& out);
    
    static void define_actions(State* state, std::ostream& out);
    
    static void declare_rules(const Grammar& grammar, std::ostream& out);
    static void declare_states(const Grammar& grammar, std::ostream& out);
    static void define_gotos(const Grammar& grammar, std::ostream& out);
};

#endif
