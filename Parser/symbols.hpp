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
     * All nonterminals have one or more production rules which define the
     * symbols that reduce to a given nonterminal.
     */
    class Rule {
      public:
        Rule(Nonterm* nonterm);
        Nonterm* nonterm;
        vector<Symbol*> product;
        string reduce;
        size_t id;
        void write_rule(ostream& out) const;
    };
    
    void add_rule(const vector<Symbol*>& syms, const string& reduce);
    vector<unique_ptr<Rule>> rules;
    
    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in or follow every nonterminal.
     */
    bool has_empty;
    set<Symbol*> firsts;
    set<Symbol*> follows;
    
    void solve_first(bool* found);
    void solve_follows(bool* found);
    
    void insert_firsts(vector<Symbol*>::iterator symbol,
                       vector<Symbol*>::iterator end);
    
    void insert_follows(vector<Symbol*>::iterator symbol,
                        vector<Symbol*>::iterator end,
                        bool* epsilon);
    
    /** Prints the grammar in BNF form. */
    virtual void print(ostream& out) const;
    
    void print_rules(ostream& out) const;
    void print_firsts(ostream& out) const;
    void print_follows(ostream& out) const;
    
    virtual void write(ostream& out) const;
    
    void write_rules(ostream& out) const;
};

#endif
