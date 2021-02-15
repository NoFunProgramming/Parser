/*******************************************************************************
 * State of the parser.  To build a parser, the program will solve for every
 * possible state that could occur while reading a valid input for the given
 * language.
 */

#ifndef state_hpp
#define state_hpp

#include "symbols.hpp"

#include <map>
using std::map;

/******************************************************************************/
class State
{
  public:
    State(size_t id);
    size_t id;
    
    void print(ostream& out) const;
    void print_items(ostream& out) const;
    
    void write(ostream& out) const;
    
    bool operator<(const State& other) const;
    
    /**
     * Each state is a set of possible parse states. At any given time while
     * parsing a valid input, the parser must be within one rule, looking for
     * one a several symbols that could be next based on the possible rules.
     */
    class Item {
      public:
        Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead);
        
        Nonterm::Rule* rule;
        size_t mark;
        Symbol* ahead;
        
        Item advance();
        vector<Symbol*> rest();
                
        Symbol* next();
        Nonterm* next_nonterm();
        bool operator==(const Item& other) const;
        bool operator<(const Item& other) const;
        void print(ostream& out) const;
    };
    
    void add(Item item);
    void closure();
    
    unique_ptr<State> solve_next(Symbol* symbol);
    void add_next(Symbol* symbol, State* next);
    
    /** Shift or reduce based on the state and next symbol. */
    class Actions {
      public:
        map<Term*, State*> shift;
        map<Symbol*, Nonterm::Rule*> accept;
        map<Symbol*, Nonterm::Rule*> reduce;
    };

    void solve_actions(Item accept);
    void solve_gotos();
    
    /** Write the states source code. */
    void write_declare(ostream& out) const;
    void write_define(ostream& out) const;
    void write_shift(ostream& out) const;
    void write_accept(ostream& out) const;
    void write_reduce(ostream& out) const;
    void write_goto(ostream& out) const;
            
  private:
    set<Item> items;
    map<Symbol*, State*> nexts;
    map<Symbol*, State*> gotos;
    unique_ptr<Actions> actions;
    
    static void firsts(const vector<Symbol*>& symbols, set<Symbol*>* firsts);
};

#endif
