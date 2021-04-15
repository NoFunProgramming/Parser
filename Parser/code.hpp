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
    
        
    static void write_state(State* state, std::ostream& out);
    static void write_define(State* state, std::ostream& out);
};

#endif