#ifndef calc_hpp
#define calc_hpp

#include "calculator.hpp"

#include <string>
#include <vector>

/******************************************************************************/
class Value;
class Table;
class Symbol;
class Node;
class State;

/******************************************************************************/
struct Accept {
    Symbol* term;
    Value* (*scan)(Table*, const std::string&);
};

struct Rule {
    Symbol* nonterm;
    Value* (*reduce)(Table*, std::vector<Value*>&);
    size_t length;
};

Node*   node_start();
Node*   next_node(Node* node, int c);
Accept* find_term(Node* node);

State*  state_start();
Symbol* symbol_end();
const char* symbol_name(Symbol*);

State* find_shift(State*, Symbol* sym);
Rule*  find_reduce(State*, Symbol* sym);
Rule*  find_accept(State*, Symbol* sym);
State* find_goto(State*, Symbol* sym);

/******************************************************************************/
class Parser {
  public:
    Parser();
    
    void init();
    bool scan(Table*, int c);
        
  private:
    Node* node;
    std::string text;
    std::vector<State*>  states;
    std::vector<Symbol*> symbols;
    std::vector<Value*>  values;
    
    bool advance(Table*, Symbol* sym, Value* val);

    void push(State* state, Symbol* sym, Value* val);
    void pop(size_t count);
};

class Value {
  public:
    virtual ~Value(){}
};


class Table {
    
};

class Expr : public Value {
  public:
    Expr(int value):value(value){}
    int value;
};

#endif
