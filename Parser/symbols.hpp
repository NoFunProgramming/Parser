/*******************************************************************************
 * The grammar of a regular language is defined by two types of symbols:
 * terminals and nonterminals.  Terminals are the smallest unit of the grammar
 * and often represent a specific pattern of characters such as a integer. The
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
 * Base class for all types of symbols such as terminals and nonterminals.
 */
class Symbol
{
  public:
    string type;
    virtual void print(ostream& out) const = 0;
    virtual void write(ostream& out) const = 0;
};

/*******************************************************************************
 * Terminals are the smallest unit of the grammar and often represent a specific
 * pattern of characters such as a integer.
 */
class Term : public Symbol
{
  public:
    Term(const string& name, size_t rank);
    string name;
    size_t rank;
    string action;
    
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
    virtual void write_declare(ostream& out) const;
    
    void write_proto(ostream& out) const;
    void write_define(ostream& out) const;
};

/*******************************************************************************
 * Endmark is a special terminal and indicates the end of an input string.
 */
class Endmark : public Symbol
{
  public:
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
};

/*******************************************************************************
 * Nonterminals of the grammar.  To form the language, every nonterminal of the
 * grammar represent a sequnce of symbols, either terminals or nonterminals.
 * These nonterminals have additional properties, such as the first and
 * following terminals which are used to solve for all possible parse states.
 */
class Nonterm : public Symbol
{
  public:
    Nonterm(const string& name);
    string name;
    size_t rank;
    
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
    void write_declare(ostream& out) const;
  
    /**
     * All nonterminals have one or more production rules, vectors of symbols,
     * that list the symbol sequence that defines a given nonterminal.  Each
     * rule can also have an action that occurs each time the parser matches a
     * rule in the input and reduces its sequence of symbols to a nonterminal.
     */
    class Rule {
      public:
        Rule(Nonterm* nonterm, const string& action);
        Nonterm* nonterm;
        vector<Symbol*> product;
        string action;
        size_t id;
        
        virtual void print(ostream& out) const;
        virtual void write(ostream& out) const;
        void write_declare(ostream& out) const;
        void write_proto(ostream& out) const;
        void write_action(ostream& out) const;
        void write_define(ostream& out) const;
    };
    
    vector<unique_ptr<Rule>> rules;
    void add_rule(const vector<Symbol*>& syms, const string& action);

    /**
     * To find all possible parse states, the first step is to solve for all
     * terminals that could be the first in the productions for each
     * nonterminal. A nonterminal can also have an empty production rule of no
     * symbols.
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
