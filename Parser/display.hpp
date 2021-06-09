/*******************************************************************************
 * Prints the parse table for a grammar.
 */
#ifndef display_hpp
#define display_hpp

#include "lexer.hpp"
#include "grammar.hpp"

#include <iostream>

class Display
{
  public:
    /**
     * After solving for the lexer, call print to display the nodes of the token
     * scanner.
     */
    static void print(const Lexer& lexer, std::ostream& out);
    
    /**
     * After solving for the all possible parse states of the grammar, call
     * print to display for the parse table actions.
     */
    static void print(const Grammar& grammar, std::ostream& out);
        
  private:
    /**
     * Displays the nodes that implement a lexer.  The lexer matches patterns
     * by defining nodes that determine the next node based on an input
     * character.  When there is no next node for a given input character, check
     * for a matching term at the current node.
     */
    static void print(const Node& state, std::ostream& out);
    static void print(const Node::Range& range, std::ostream& out);
    
    /**
     * Writes the actions for each symbol and state.  The actions determines if
     * the parser should shift the next terminal onto its stack or reduce the
     * stack by a matched production rule.
     */
    static void print_action(const Symbol* symbol, State::Actions* actions,
                             std::ostream& out);
    static void print_goto(Symbol* symbol, State* state,
                           std::ostream& out);
    
    /** Determine the longest symbol name for justifing the parse table. */
    static size_t max_length(const Grammar& grammar);
};

#endif
