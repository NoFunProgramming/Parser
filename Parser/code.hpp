/*******************************************************************************
 * Generates source code for parsing a regular language.  The class reads in a
 * user defined grammar and outputs source code that can be compiled into
 * another program to parse an input string.  Action methods can be associated
 * with every rule of the grammar and are called when that pattern is found
 * within the input string.  User provided action methods and the generated
 * source code for the parser can be combined to build programs such as a
 * compiler.
 */

#ifndef code_hpp
#define code_hpp

#include "grammar.hpp"
#include <iostream>

/*******************************************************************************
 * Writes the source code to a parser.  
 * The parser reads in grammar rules and outputs
 * source code that parses a regular language.  The grammar is defined by two
 * types of symbols: terminals and nonterminals.  The nonterminal themselves
 * are define as a sequence of symbols, known as a production rule. Terminals
 * are shown in quotes and are defined by a pattern of input characters.
 */
class Code
{
  public:
    /**************************************************************************/
    static void write(Grammar& grammar, std::ostream& out);

    /**************************************************************************/
    static void write_struct(std::ostream& out);
        
    /**************************************************************************/
    static void write_declare(Term* term, std::ostream& out);
    static void write_define (Term* term, std::ostream& out);
    
    /**************************************************************************/
    static void write_declare(Nonterm* nonterm, std::ostream& out);
    static void write_declare(Nonterm::Rule* rule, std::ostream& out);
    static void write_proto  (Nonterm::Rule* rule, std::ostream& out);
    
    /**************************************************************************/
    static void write_action(Nonterm::Rule* rule, std::ostream& out);
    static void write_define(Nonterm::Rule* rule, std::ostream& out);
    
    /**************************************************************************/
    static void write_shift (State* state, std::ostream& out);
    static void write_accept(State* state, std::ostream& out);
    static void write_reduce(State* state, std::ostream& out);
    static void write_goto  (State* state, std::ostream& out);
    static void write_define(State* state, std::ostream& out);

    /**************************************************************************/
    static void write_functions(std::ostream& out);
};

#endif
