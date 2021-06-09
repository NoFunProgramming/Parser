/*******************************************************************************
 * Writes the parse table for a grammar.  The generated parse table can be
 * combined with custom functions for each grammar rule to build parsers or
 * compilers.
 */
#ifndef code_hpp
#define code_hpp

#include "grammar.hpp"

#include <iostream>
#include <vector>
using std::ostream;

/*******************************************************************************
 * Writes source code that defines the parse table for a grammar.  The code
 * provides functions to find the next action based on the current parse state
 * and given input symbol.
 */
class Code
{
  public:
    /**
     * After solving for the lexer, call write to output source code for the
     * token scanner.
     */
    static void write(const Lexer& lexer, std::ostream& out);

    /**
     * After solving for the all possible parse states of the grammar, call
     * write to output the source code for the parse table.
     */
    static void write(const Grammar& grammar, std::ostream& out);

  private:
    /**
     * Writes the functions that implement a lexer.  The lexer matches patterns
     * by defining nodes that determine the next node based on an input
     * character.  When there is no next node for a given input character, check
     * for a matching term at the current node.
     */
    static void write_terms(Term* term, ostream& out);
    static void write_eval(Term* term, ostream& out);
    static void write_scan(Node* node, ostream& out);
    static void write_node(Node* node, ostream& out);
    
    /**
     * Writes the functions that call the user defined action for a given rule.
     * These functions get values from the top of the stack and cast those
     * values to their user define classes.  After casting the values, the
     * functions call the user define action with those objects.
     */
    static void write_rule_action(Nonterm::Rule* rule, ostream& out);
    static void write_call_action(Nonterm::Rule* rule, ostream& out);
    
    /**
     * Writes the rules that define which action to call when a sequence of
     * symbols is reduced to a nonterminal.  These written rules provide the
     * number of symbols in a production so that the parser knows the number of
     * values to remove from the stack after the reduction.
     */
    static void write_nonterm(Nonterm* nonterm, ostream& out);
    static void write_rules(const Grammar& grammar, ostream& out);

    /**
     * Writes the actions for each state.  The actions determines if the parser
     * should shift the next terminal onto its stack or reduce the stack by a
     * matched production rule.
     */
    static void write_actions(std::vector<State*> states, ostream& out);
    static void write_gotos(std::vector<State*> states, ostream& out);
    static void write_states(std::vector<State*> states, ostream& out);
};

#endif
