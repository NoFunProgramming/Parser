/*******************************************************************************
 * State of the parser.  To build a parser, the program will solve for every
 * possible state that could occur while reading a valid input for the given
 * language.
 */

#ifndef state_hpp
#define state_hpp

#include "symbols.hpp"

#include <map>

/******************************************************************************/
class State
{
  public:
    State(size_t id);
    
    /**
     * Each state is a set of possible parse rules.  At any given time while
     * parsing a valid input, the parser must be within at least one rule while
     * waiting for one of several symbols that could be next based on the
     * grammar.
     */
    class Item {
      public:
        Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead);
        
        Nonterm::Rule* rule;
        size_t mark;
        Symbol* ahead;
        
        Item advance();
        std::vector<Symbol*> rest();
                
        Symbol* next();
        Nonterm* next_nonterm();
        bool operator==(const Item& other) const;
        bool operator<(const Item& other) const;
        void print(std::ostream& out) const;
    };
    
    void add(Item item);
    void closure();
    
    std::unique_ptr<State> solve_next(Symbol* symbol, size_t id);
    void add_next(Symbol* symbol, State* next);
    
    /** Shift or reduce based on the state and next symbol. */
    class Actions {
      public:
        std::map<Term*, State*> shift;
        std::map<Symbol*, Nonterm::Rule*> accept;
        std::map<Symbol*, Nonterm::Rule*> reduce;
    };

    void solve_actions(Item accept);
    void solve_gotos();
    
    void print(std::ostream& out) const;
    void print_items(std::ostream& out) const;
        
    bool operator<(const State& other) const;
            
  //private:
    size_t id;
    std::set<Item> items;
    std::map<Symbol*, State*> nexts;
    std::map<Symbol*, State*> gotos;
    std::unique_ptr<Actions> actions;
    
    static void firsts(const std::vector<Symbol*>& symbols,
                       std::set<Symbol*>* firsts);
};

#endif
