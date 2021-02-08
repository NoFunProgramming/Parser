/*******************************************************************************
 * Main class for building a language parser.
 */

#ifndef parser_hpp
#define parser_hpp

#include "symbols.hpp"
#include "lexer.hpp"
#include "state.hpp"

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
 * source code that parses a regular language. Each nonterminal has one
 * or more rules seperated by vertical bars and terminated with a semicolon.
 */
class Parser
{
  public:
    Parser();
    
    /** Read in the grammar that defines the parser. */
    bool read_grammar(istream& in);
    
    /** After reading, solve for all possible parse states. */
    void solve();
    
    /** After solving, write the source code for the parser. */
    void write(ostream& out) const;
    void print_grammar(ostream& out) const;

  private:
    /**
     * While reading in a grammar, the parser builds the a set of unique
     * terminal and nonterminal names.  The parser stores production rules as
     * vectors of pointers to these terminal and nonterminal symbols.
     */
    map<string, unique_ptr<Symbol>> terms;
    map<string, unique_ptr<Nonterm>> nonterms;
    Nonterm* first;
         
    /** Recursive decent parser for reading grammar rules. */
    bool read_term(istream& in);
    bool read_rules(istream& in);
    bool read_product(istream& in, vector<Symbol*>* syms);
    
    /** Reads and then interns unique terminal and nonterminal names. */
    Symbol* intern_term(istream& in);
    Nonterm* intern_nonterm(istream& in);
        
    /** Recursive decent parser for reading grammar rules. */
    bool read_term_name(istream& in, string* name);
    bool read_nonterm_name(istream& in, string* type);
    
    /** Attributes of the grammar rules. */
    bool read_type(istream& in, string* type);
    bool read_regex(istream& in, string* regex);
    bool read_action(istream& in, string* action);
        
    /** Lexer to scan an input for terminals. */
    Lexer lexer;

    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in a rule or follows nonterminal.
     */
    void solve_first();
    void solve_follows();

    /**
     * Set of all possible parse states.  The compare method is for the set to
     * determines if the next parse state for a given symbol has already been
     * found.
     */
    struct Compare {
        bool operator() (const unique_ptr<State>& lhs,
                         const unique_ptr<State>& rhs) const {
            return *lhs < *rhs;
        }
    };
    set<unique_ptr<State>, Compare> states;
    State* start;
};

#endif
