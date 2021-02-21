#ifndef parser_hpp
#define parser_hpp

#include <string>
#include <vector>
using std::vector;

/******************************************************************************/
class Value {
  public:
    virtual ~Value(){}
};

/******************************************************************************/
struct Symbol {
    virtual ~Symbol(){}
};

struct Term : public Symbol {
    Term(const char* name, Value* (*scan)(const std::string& s)):name(name), scan(scan){}
    const char* name;
    Value* (*scan)(const std::string& s);
};

struct Nonterm : public Symbol {
};

struct Node {
    Node* (*scan)(int c);
    Symbol* accept;
};


/******************************************************************************/
class State;

class Rule {
  public:
    int length;
    Symbol* nonterm;
    Value* (*reduce)(vector<Value*>&);
};

struct Shift {
    Symbol* sym;
    State*  state;
};

struct Accept {
    Symbol* sym;
    Rule*   rule;
};

struct Reduce {
    Symbol* sym;
    Rule*   rule;
};

struct Go {
    Symbol* sym;
    State*  state;
};

struct State {
    int     id;
    Shift*  shift;
    Accept* accept;
    Reduce* reduce;
    Go*     next;
    
    State* find_shift(Symbol* sym);
    Rule*  find_reduce(Symbol* sym);
    Rule*  find_accept(Symbol* sym);
    State* find_goto(Symbol* sym);
};

/******************************************************************************/
class Parser {
  public:
    Parser();
    Node* node;
    std::string text;
    
    void init();
    bool scan(int c);
    bool advance(Symbol* sym);
    
    
  private:
    vector<State*> stack;
    vector<Symbol*> symbols;
    vector<Value*> values;

    void push(State* state, Symbol* sym, Value* val);
    void pop(size_t count);
};

#endif
