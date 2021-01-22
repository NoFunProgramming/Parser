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
    static Symbol Endmark;
    
    virtual void print(ostream& out) const { out << "$"; }
};

/*******************************************************************************
 * Terminals of the grammar.
 */
class Term : public Symbol
{
  public:
    Term(const string& name);
    
    virtual void print(ostream& out) const;
  
  private:
    string name;
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
        void add(Symbol* sym);
        void print(ostream& out) const;
        Nonterm* nonterm;
        vector<Symbol*> product;
    };
    
    Rule* add_rule();
    
    /** Prints the grammar in BNF form. */
    virtual void print(ostream& out) const;

    /**
     * The first step to finding all possible parse states is finding all
     * terminals that could be first in or follow every nonterminal.
     */
    void solve_first(bool* found);
    void solve_follows(bool* found);
    void insert_follows(Symbol* symbol);

  private:
    string name;
    vector<unique_ptr<Rule>> rules;
    
    /**
     * Set of all terminals that could be the first one in or follow any
     * production of this nonterminal.  The following methods add all terminals
     * that could be first in part of a string of symbols.
     */
    set<Symbol*> firsts;
    set<Symbol*> follows;
    bool has_empty;
    
    void insert_firsts(vector<Symbol*>::iterator symbol,
                       vector<Symbol*>::iterator end);
    
    void insert_follows(vector<Symbol*>::iterator symbol,
                        vector<Symbol*>::iterator end,
                        bool* epsilon);
};

#endif
