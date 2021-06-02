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
    static void print(const Lexer& lexer, std::ostream& out);
    static void print(const Grammar& grammar, std::ostream& out);
    
    static void print_actions(const Grammar& grammar, std::ostream& out);
    static void print_action(const Symbol* symbol, State::Actions* actions, std::ostream& out);
    static void print_goto(Symbol* symbol, State* state, std::ostream& out);

    
  private:
    static void print(const Lexer::State& state, std::ostream& out);
    static void print(const Lexer::State::Range& range, std::ostream& out);
    
    static void print_terms(const Grammar& grammar, std::ostream& out);
    static void print_state(const Grammar& grammar, const State& state, std::ostream& out);
    
    static size_t max_length(const Grammar& grammar);
    
};

#endif
