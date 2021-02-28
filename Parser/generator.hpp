/*******************************************************************************
 * Generates source code for parsing a regular language.  The class reads in a
 * user defined grammar and outputs source code that can be compiled into
 * another program to parse an input string.  Action methods can be associated
 * with every rule of the grammar and are called when that pattern is found
 * within the input string.  User provided action methods and the generated
 * source code for the parser can be combined to build programs such as a
 * compiler.
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
 * Writes a language parser.  The parser reads in grammar rules and outputs
 * source code that parses a regular language.  The grammar is defined by two
 * types of symbols: terminals and nonterminals.  The nonterminal themselves
 * are define as a sequence of symbols, known as a production rule. Terminals
 * are shown in quotes and are defined by a pattern of input characters.
 */
class Generator
{
  public:
    Generator();
    
    /** Reads in the grammar that defines the parser. */
    bool read_grammar(istream& in);
    
    /** After reading, solve for all possible parse states. */
    void solve();
    
    /** After solving, write the source code for the parser. */
    void write(ostream& out) const;
    void print_grammar(ostream& out) const;
    void print_states(ostream& out) const;

  private:
    /**
     * While reading in a grammar, the parser builds a set of unique terminal
     * and nonterminal names.  The parser can then store production rules as
     * vectors of pointers to these terminal and nonterminal symbols.
     */
    map<string, unique_ptr<Term>> terms;
    map<string, unique_ptr<Nonterm>> nonterms;
    vector<unique_ptr<Accept>> accepts;
    vector<Nonterm*> all;
    Endmark endmark;
         
    /** Recursive decent parser for reading grammar rules. */
    bool read_term(istream& in);
    bool read_rules(istream& in);
    bool read_product(istream& in, vector<Symbol*>* syms);
    bool read_comment(istream& in);
    
    /** Interns symbol names while reading production rules. */
    Term* intern_term(istream& in);
    Nonterm* intern_nonterm(istream& in);
        
    /** Reads a valid name for a symbol. */
    bool read_term_name(istream& in, string* name);
    bool read_nonterm_name(istream& in, string* name);
    
    /** Attributes of the grammar rules. */
    bool read_type(istream& in, string* type);
    bool read_regex(istream& in, string* regex);
    bool read_action(istream& in, string* action);
        
    /** Lexer to scan an input for terminals. */
    Lexer lexer;

    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in a rule or any terminal which could
     * follow a nonterminal.
     */
    void solve_first();
    void solve_follows(Symbol* endmark);

    /**
     * Set of all possible unique parse states.  The compare method determines
     * if the next parse state for a given symbol has already been found.
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
