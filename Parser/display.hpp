#ifndef display_hpp
#define display_hpp

#include "lexer.hpp"
#include <iostream>

class Display
{
  public:
    static void print(const Lexer& lexer, std::ostream& out);
    
  private:
    static void print(const Lexer::State& state, std::ostream& out);
    static void print(const Lexer::State::Range& range, std::ostream& out);
};

#endif
