#ifndef code_hpp
#define code_hpp

#include "generator.hpp"

#include <iostream>

class Code {
  public:
    static void write(Generator& gen, std::vector<State*>& states, std::ostream& out);
    
    static void write_struct(std::ostream& out);
    static void write_functions(std::ostream& out);
    
    static void write_terms(Generator& gen, std::ostream& out);
    
        
    static void write_shift(State* state, std::ostream& out);
    static void write_accept(State* state, std::ostream& out);
    static void write_reduce(State* state, std::ostream& out);
    static void write_goto(State* state, std::ostream& out);
    
    
    static void write_define(State* state, std::ostream& out);
    
    static void write(Nonterm::Rule* rule, std::ostream& out);

    static void write_declare(Nonterm::Rule* rule, std::ostream& out);

    static void write_proto(Nonterm::Rule* rule, std::ostream& out);

    
    static void write_action(Nonterm::Rule* rule, std::ostream& out);
    static void write_define(Nonterm::Rule* rule, std::ostream& out);
};

#endif
