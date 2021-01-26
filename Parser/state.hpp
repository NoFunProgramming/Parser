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
    const string& get_ident() const;
    
    /**
     * Each state is a set of possible parse states.
     */
    class Item {
      public:
        Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead);
        Nonterm::Rule* rule;
        size_t mark;
        Symbol* ahead;
        
        Item advance();
        vector<Symbol*> rest();
                
        bool is_end();
        bool is_next(Symbol* symbol);
        
        Term* next_term();
        Nonterm* next_nonterm();
        
        bool operator==(const Item& other) const;
        bool operator<(const Item& other) const;
    };
    
    void add(Item item);
    void closure();

    /**
     * Actions for shift or reducing based on the state and next symbol.
     */
    class Actions {
      public:
        // TODO Change shift to terminals.
        map<Symbol*, State*> shift;
        map<Symbol*, Nonterm::Rule*> accept;
        map<Symbol*, Nonterm::Rule*> reduce;
    };
        
    unique_ptr<State> solve_next(Symbol* symbol);
    void add_next(Symbol* symbol, State* next);
    
    void solve_actions(Item accept);
    void solve_gotos();
    
    void write(ostream& out) const;
    void write_declare(ostream& out) const;
    void write_shift(ostream& out) const;
    void write_accept(ostream& out) const;
    void write_reduce(ostream& out) const;
    
    const string& get_name() { return name; }
    
    bool operator<(const State& other) const;
    
  private:
    string name;
    set<Item> items;
    map<Symbol*, State*> nexts;
    map<Symbol*, State*> gotos;
    //State::Actions* actions;
    unique_ptr<Actions> actions;
    
    static void firsts(const vector<Symbol*>& symbols, set<Symbol*>* firsts);
    
};

#endif
