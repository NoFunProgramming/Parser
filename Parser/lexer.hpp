#ifndef lexer_hpp
#define lexer_hpp

#include "regex.hpp"

#include <iostream>
using std::ostream;

class Lexer
{
  public:
    Lexer();

    bool add(Accept* accept, const string& regex);
    void solve();

    void write(ostream& out);
};

#endif
