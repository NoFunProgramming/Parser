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
 *
 * The input rules are written as a nonterminal followed by zero or more
 * symbols.  If there is more than one rule associated the same nonterminal,
 * they are separated by a vertical bar.  A semicolon indicates the end of the
 * rules for a given nonterminal.
 *
 *      add: mul | add '+' mul;
 *
 * At the end of each rule can be optional user defined action, indicated by an
 * ampersand.  The action method is called by the generated source code when
 * that rule's sequence of symbols is found.  Each nonterminal can also have a
 * type which is defined within angle brackets.  The generated source code calls
 * the action method with a input argument for each rule's symbol that has a
 * type.
 *
 *      add<int64> add '+' mul &reduce_add_mul;
 *
 * Terminals are defined by regular expressions.  When running the generated
 * source code, the parser will identify these patterns in the input string and
 * add the matching terminal to the stack of the parser.  The parser will then
 * check this stack for sequences that match one of the rules.  If a match is
 * found, that rule's nonterminal will replace those symbols on the stack.
 *
 *      'num' [0-9]+;
 *
 * Similar to nonterminals, the terminals can also have a user defined action
 * and a type. When a pattern is match in the input string, the source calls the
 * with the matching string and expects the method to return a value of the
 * provided type.
 *
 *    'num'<int> [0-9]+ &read_int;
 */
class Parser
{
  public:
    Parser();
    
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
    vector<Nonterm*> all;
    Nonterm* first;
    Endmark endmark;
         
    /** Recursive decent parser for reading grammar rules. */
    bool read_term(istream& in);
    bool read_rules(istream& in);
    bool read_product(istream& in, vector<Symbol*>* syms);
    
    /** Reads and add to, or finds in, the set of symbol names. */
    Term* intern_term(istream& in);
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
