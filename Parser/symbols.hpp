/*******************************************************************************
 * The grammar of a regular language is defined by two types of symbols:
 * terminals and nonterminals.  Terminals are the smallest unit of the grammar
 * and often represent a specific pattern of characters such as a integer. The
 * nonterminal themselves are defined by production rules, which are a sequences
 * of both terminals and nonterminals.
 */

#ifndef symbols_hpp
#define symbols_hpp

#include <string>
#include <vector>
#include <set>
#include <sstream>

/*******************************************************************************
 * Base class for all types of symbols such as terminals and nonterminals.
 */
class Symbol
{
  public:
    std::string type;
    virtual void print(std::ostream& out) const = 0;
    virtual void write(std::ostream& out) const = 0;
};

/*******************************************************************************
 * Terminals are the smallest unit of the grammar and often represent a specific
 * pattern of characters such as a integer.
 */
class Term : public Symbol
{
  public:
    Term(const std::string& name, size_t rank);
    std::string name;
    size_t rank;
    
    std::string action;
    
    virtual void print(std::ostream& out) const;
    virtual void write(std::ostream& out) const;
};

/*******************************************************************************
 * Endmark is a special terminal and indicates the end of an input string.
 */
class Endmark : public Symbol
{
  public:
    virtual void print(std::ostream& out) const;
    virtual void write(std::ostream& out) const;
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
    Nonterm(const std::string& name);
    std::string name;
    size_t rank;

    virtual void print(std::ostream& out) const;
    virtual void write(std::ostream& out) const;
      
    /**
     * All nonterminals have one or more production rules, vectors of symbols,
     * that list the symbol sequence that defines a given nonterminal.  Each
     * rule can also have an action that occurs each time the parser matches a
     * rule in the input and reduces its sequence of symbols to a nonterminal.
     */
    class Rule {
      public:
        Rule(Nonterm* nonterm, const std::string& action);
        Nonterm* nonterm;
        std::vector<Symbol*> product;
        std::string action;
        size_t id;
        
        virtual void print(std::ostream& out) const;
        virtual void write(std::ostream& out) const;
    };
    
    std::vector<std::unique_ptr<Rule>> rules;
    void add_rule(const std::vector<Symbol*>& syms, const std::string& action);

    /**
     * To find all possible parse states, the first step is to solve for all
     * terminals that could be the first in the productions for each
     * nonterminal. A nonterminal can also have an empty production rule of no
     * symbols.
     */
    std::set<Symbol*> firsts;
    bool empty_first;
    void solve_first(bool* found);

    /**
     * After finding the firsts, solve for all terminals that could follow each
     * nonterminal.
     */
    std::set<Symbol*> follows;
    void solve_follows(bool* found);
    
    /** Prints the input grammar in BNF form. */
    void print_rules(std::ostream& out) const;
    void print_firsts(std::ostream& out) const;
    void print_follows(std::ostream& out) const;
        
  private:
    /**
     * Methods called by solve for adding symbols to the set of firsts and
     * follows for each nonterminal.  To solve for the firsts and follows of the
     * grammar, the program will keep calling solve as long as a new symbols are
     * being found and inserted into the set.
     */
    void insert_firsts(Rule* rule, bool* found);
    
    void insert_follows(const std::set<Symbol*>& syms, bool* found);
    void insert_follows(std::vector<Symbol*>::iterator symbol,
                        std::vector<Symbol*>::iterator end,
                        bool* epsilon, bool* found);
};

#endif
