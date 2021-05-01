/*******************************************************************************
 * Possible state while parsing an input string.  To build a parser, the program
 * solves for every possible state that could occur while reading a valid string
 * for the given grammar.
 */

#ifndef state_hpp
#define state_hpp

#include "symbols.hpp"
#include <map>

/*******************************************************************************
 * Each state contains a set of possible rules that could be matched after
 * having read the previous input symbols.
 */
class State
{
  public:
    State(size_t id);
    size_t id;
    
    /**
     * Possible location within a grammar rule during parsing.  At any given
     * time the parser must be within at least one rule while waiting for one of
     * several symbols that could be next based on the grammar.
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
    
    /** Returns the next state for a given input symbol. */
    std::unique_ptr<State> solve_next(Symbol* symbol, size_t id);
    void add_next(Symbol* symbol, State* next);
    
    /** Shift or reduce actions given the next symbol. */
    class Actions {
      public:
        std::map<Term*, State*> shift;
        std::map<Symbol*, Nonterm::Rule*> accept;
        std::map<Symbol*, Nonterm::Rule*> reduce;
    };
    std::unique_ptr<Actions> actions;
    void solve_actions(Item accept);
    
    /** Defines the next parse state after reduction of a rule. */
    std::map<Symbol*, State*> gotos;
    void solve_gotos();
    
    void print(std::ostream& out) const;
    void print_items(std::ostream& out) const;
        
    bool operator<(const State& other) const;
                
  private:
    std::set<Item> items;
    std::map<Symbol*, State*> nexts;

    static void firsts(const std::vector<Symbol*>& symbols,
                       std::set<Symbol*>* firsts);
};

#endif
