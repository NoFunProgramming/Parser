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

#include <string>
#include <map>
#include <memory>
#include <sstream>

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
    bool read_grammar(std::istream& in);
    
    /** After reading, solve for all possible parse states. */
    void solve();
    
    /** After solving, write the source code for the parser. */
    void write(std::ostream& out) const;
    
    void print_grammar(std::ostream& out) const;
    void print_states(std::ostream& out) const;

  //private:
    /**
     * While reading in a grammar, the parser builds a set of unique terminal
     * and nonterminal names.  The parser can then store production rules as
     * vectors of pointers to these terminal and nonterminal symbols.
     */
    std::map<std::string, std::unique_ptr<Term>> terms;
    std::map<std::string, std::unique_ptr<Nonterm>> nonterms;
    std::vector<std::unique_ptr<Accept>> accepts;
    std::vector<Nonterm*> all;
    Endmark endmark;
         
    /** Recursive decent parser for reading grammar rules. */
    bool read_term(std::istream& in);
    bool read_rules(std::istream& in);
    bool read_product(std::istream& in, std::vector<Symbol*>* syms);
    bool read_comment(std::istream& in);
    
    /** User defined includes to appear at the top of the generated file. */
    std::vector<std::string> includes;
    bool read_include(std::istream& in);

    /** Interns symbol names while reading production rules. */
    Term* intern_term(std::istream& in);
    Nonterm* intern_nonterm(std::istream& in);
        
    /** Reads a valid name for a symbol. */
    bool read_term_name(std::istream& in, std::string* name);
    bool read_nonterm_name(std::istream& in, std::string* name);
    
    /** Attributes of the grammar rules. */
    bool read_type(std::istream& in, std::string* type);
    bool read_regex(std::istream& in, std::string* regex);
    bool read_action(std::istream& in, std::string* action);
        
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
        bool operator() (const std::unique_ptr<State>& lhs,
                         const std::unique_ptr<State>& rhs) const {
            return *lhs < *rhs;
        }
    };
    std::set<std::unique_ptr<State>, Compare> states;
    State* start;
};

#endif
