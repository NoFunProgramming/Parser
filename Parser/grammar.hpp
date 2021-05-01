/*******************************************************************************
 * Represents a user defined grammar.  Provides a class to read a grammar in
 * Backus-Naur Form (BNF) and then solve for its parse table.
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
 * Grammar of a regular language.  The class reads in a user defined grammar and
 * and solves for all possible parse states of the language.  The class keeps
 * a set of all unique terminal and nonterminals found in the language.  Action
 * method names can be associated with every grammar rule and then stored in the
 * parse table.  Implemented action methods and the parse table can be combined
 * to build a parser or a compiler.
 */
class Grammar
{
  public:
    Grammar();
    
    /** Reads in the user defined grammar. */
    bool read_grammar(std::istream& in);
    
    /** After reading, solve for all of the possible parse states. */
    void solve_states();
            
    /** Unique terminals and nonterminals of the grammar. */
    std::map<std::string, std::unique_ptr<Term>> terms;
    std::map<std::string, std::unique_ptr<Nonterm>> nonterms;
    std::vector<Nonterm*> all;
    Endmark endmark;
    
    /**
     * While reading rules the grammar will also build a lexer for finding
     * terminals in an input string.  Each accept state corresponds to a
     * terminal of the same rank.
     */
    Lexer lexer;
    std::vector<std::unique_ptr<Accept>> accepts;

    /** Unique parse states of the grammar. */
    struct is_same {
        bool operator() (const std::unique_ptr<State>& lhs,
                         const std::unique_ptr<State>& rhs) const {
            return *lhs < *rhs;
        }
    };
    std::set<std::unique_ptr<State>, is_same> states;
    State* start;
    
    std::vector<std::string> includes;
    
    void print_grammar(std::ostream& out) const;
    void print_states(std::ostream& out) const;
    
  private:
    /** Recursive decent parser for reading grammar rules. */
    bool read_term(std::istream& in);
    bool read_rules(std::istream& in);
    bool read_product(std::istream& in, std::vector<Symbol*>* syms);
    bool read_comment(std::istream& in);
            
    bool read_term_name(std::istream& in, std::string* name);
    bool read_nonterm_name(std::istream& in, std::string* name);
    
    /** Reads attributes of the symbols. */
    bool read_type(std::istream& in, std::string* type);
    bool read_regex(std::istream& in, std::string* regex);
    bool read_action(std::istream& in, std::string* action);
    
    bool read_include(std::istream& in);
    
    /** Interns symbol names while reading production rules. */
    Term* intern_term(std::istream& in);
    Nonterm* intern_nonterm(std::istream& in);

    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in a rule or any terminal which could
     * follow a nonterminal.
     */
    void solve_first();
    void solve_follows(Symbol* endmark);
};

#endif
