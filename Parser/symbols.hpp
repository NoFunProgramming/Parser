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
 * All grammar strings are comprised of symbols.
 */
class Symbol
{
  public:
    Symbol(const string& name);
    string name;
    string type;
    size_t id;

    static Symbol Endmark;
    
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
};

/*******************************************************************************
 * Nonterminals of the grammar.  
 */
class Nonterm : public Symbol
{
  public:
    Nonterm(const string& name);
  
    /**
     * All nonterminals have one or more production rules, vectors of symbols,
     * that list the symbols that define a given nonterminal.  Each rule can
     * also have an action that is to occur each time a rule reduces a string of
     * symbols to a nonterminal.
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
    void add_rule(const vector<Symbol*>& syms, const string& reduce);

    /**
     * To find all possible parse states, solve for all terminals that could be
     * the first terminal in a production for each nonterminal.  Nonterminals
     * could also have an empty production of no symbols.
     */
    set<Symbol*> firsts;
    bool empty_first;
    void solve_first(bool* found);

    /**
     * After find the firsts, finding all terminals that could follow every
     * nonterminal.
     */
    set<Symbol*> follows;
    void solve_follows(bool* found);
    
    /** Prints the grammar in BNF form. */
    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;

    void print_rules(ostream& out) const;
    void print_firsts(ostream& out) const;
    void print_follows(ostream& out) const;
        
    void write_actions(ostream& out) const;
    
  private:
    /**
     * After find the firsts, finding all terminals that could follow every
     * nonterminal.
     */
    void insert_firsts(Rule* rule, bool* found);
    
    void insert_follows(set<Symbol*>& syms, bool* found);
    void insert_follows(vector<Symbol*>::iterator symbol,
                        vector<Symbol*>::iterator end,
                        bool* epsilon, bool* found);

};

#endif
