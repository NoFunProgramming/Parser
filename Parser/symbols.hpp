/*******************************************************************************
 * The grammar of a regular language is defined by two types of symbols:
 * terminals and nonterminals.  Terminals are the smallest unit of the grammar
 * and often represent a specific pattern of characters, such as a integer. The
 * nonterminal themselves are defined by production rules, which are a sequences
 * of both terminals and nonterminals.
 */

#ifndef symbols_hpp
#define symbols_hpp

#include <set>
#include <vector>
#include <string>
#include <sstream>
using std::set;
using std::vector;
using std::string;
using std::ostream;
using std::unique_ptr;

/*******************************************************************************
 * Symbols for defining a regular language.  Every symbol has a name identifier
 * and an optional associated type.  The type defines the return value of the
 * action method of the symbol.  Endmark is a special terminal and indicates
 * the end of an input string.
 */
class Symbol
{
  public:
    Symbol(const string& name);
    string name;
    string type;
    size_t id;

    //static Symbol Endmark;
    
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
};

/*******************************************************************************
 * Nonterminals of the grammar.  To form the language, every nonterminals of the
 * grammar represent a sequnce of symbols, either terminals or nonterminals.
 * These nonterminals have additional properties, such as the first and
 * following terminals which are used to generate a parser.
 */
class Nonterm : public Symbol
{
  public:
    Nonterm(const string& name);
    
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
  
    /**
     * All nonterminals have one or more production rules, vectors of symbols,
     * that list the symbol sequence that defines a given nonterminal.  Each
     * rule can also have an action that is to occur each time the parser
     * matches a rule and reduces its string of symbols to a nonterminal.
     */
    class Rule {
      public:
        Rule(Nonterm* nonterm, const string& action);
        Nonterm* nonterm;
        vector<Symbol*> product;
        string action;
        size_t id;
        
        void write_action(ostream& out) const;
    };
    
    vector<unique_ptr<Rule>> rules;
    void add_rule(const vector<Symbol*>& syms, const string& action);

    /**
     * To find all possible parse states, the first step is to solve for all
     * terminals that could be the first in a productions for each nonterminal.
     * A nonterminal can also have an empty production rule of no symbols.
     */
    set<Symbol*> firsts;
    bool empty_first;
    void solve_first(bool* found);

    /**
     * After finding the firsts, solve for all terminals that could follow each
     * nonterminal.
     */
    set<Symbol*> follows;
    void solve_follows(bool* found);
    
    /** Prints the input grammar in BNF form. */
    void print_rules(ostream& out) const;
    void print_firsts(ostream& out) const;
    void print_follows(ostream& out) const;
    
  private:
    /**
     * Methods called by solve for adding symbols to the set of firsts and
     * follows for each nonterminal.  To solve for the firsts and follows of the
     * grammar, the program will keep calling solve as long as a new symbols are
     * being found and inserted into the set.
     */
    void insert_firsts(Rule* rule, bool* found);
    
    void insert_follows(const set<Symbol*>& syms, bool* found);
    void insert_follows(vector<Symbol*>::iterator symbol,
                        vector<Symbol*>::iterator end,
                        bool* epsilon, bool* found);
};

#endif
