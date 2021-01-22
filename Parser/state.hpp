#ifndef state_hpp
#define state_hpp

#include "symbols.hpp"

#include <map>
using std::map;

/******************************************************************************/
class State
{
  public:
    class Item {
      public:
        Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead);
        
        bool operator<(const Item& other) const;
        
        Term* next_term();
        Nonterm* next_nonterm();
        
      private:
        Nonterm::Rule* rule;
        size_t mark;
        Symbol* ahead;
    };
    
    void add(Item item);
    void add_next(Symbol* symbol, State* next);
    
    void closure();
    unique_ptr<State> solve_next(Symbol* symbol);
    
    struct Compare {
        bool operator() (const unique_ptr<State>& lhs,
                         const unique_ptr<State>& rhs) const {
            return false;
            //return *lhs < *rhs;
        }
    };
        
  private:
    set<Item> items;
    map<Symbol*, State*> nexts;
};

#endif
