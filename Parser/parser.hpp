#ifndef parser_hpp
#define parser_hpp

#include "symbols.hpp"

#include <map>
#include <string>
#include <sstream>
#include <memory>
using std::map;
using std::string;
using std::istream;
using std::unique_ptr;

/*******************************************************************************
 * Builds a language parser.  The class reads in grammar rules and outputs
 * source code that parses a regular language.
 */
class Parser
{
  public:
    Parser();
    
    /**
     * Each nonterminal defines one or more rules seperated by vertical bars and
     * terminated with a semicolon.
     */
    bool read_grammar(istream& in);
    void print_grammar(ostream& out) const;
    
  private:
    /**
     * While reading in a grammar the parser a unique set of terminal and
     * nonterminal names.  The parser stores production rules as vectors of
     * pointers to these terminal and nonterminal symbols.
     */
    map<string, unique_ptr<Term>> terms;
    map<string, unique_ptr<Nonterm>> nonterms;
    Nonterm::Rule* first;
    
    /** Reads and then interns unique terminal and nonterminal names. */
    Term* read_term(istream& in);
    Nonterm* read_nonterm(istream& in);
     
    /** Recursive decent parser for reading grammar rules. */
    bool read_rules(istream& in);
    bool read_product(istream& in, Nonterm::Rule* rule);

    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in or follow every nonterminal.
     */
    void solve_first();
    void solve_follows();
};

#endif
