/*******************************************************************************
 * Calculator program using a precomputed parse table define by a seperate
 * grammar file.  To build the calculator, first call parse along with the
 * calculator's grammar file to write out a file that contains the parse table.
 * Then compile the calculator source along with the precomputed parse table to
 * build the final program.
 */
#ifndef calculator_hpp
#define calculator_hpp

#include <string>
#include <vector>

/*******************************************************************************
 * Classes for the terminals and nonterminals types.
 */
class Value {
  public:
    virtual ~Value() = default;
};

class Expr : public Value {
  public:
    Expr(int value);
    int value;
};

class Table {
};

/*******************************************************************************
 * Classes required to specify the parse table and to find the next action based
 * on the current parse state and next input symbol.
 */
struct Symbol {
    const char* name;
};

struct Accept {
    Symbol* term;
    Value* (*scan)(Table*, const std::string&);
};

struct N {
    int (*next)(int c);
    Symbol* term;
    Value* (*scan)(Table*, const std::string&);
};

extern N ns[];

struct Rule {
    Symbol* nonterm;
    Value* (*reduce)(Table*, std::vector<Value*>&);
    size_t length;
};

class Node;
extern Node* Start_Node;
Node*   next_node(Node* node, int c);
Accept* is_accept(Node* node);

class State;
extern State* Start_State;
extern Symbol* Endmark;

struct Rs {
    Symbol* nonterm;
    size_t length;
    Value* (*reduce)(Table*, std::vector<Value*>&);
};

extern Rs rs[];

//State* find_shift (State*, Symbol* sym);
//Rule*  find_reduce(State*, Symbol* sym);
//Rule*  find_accept(State*, Symbol* sym);
//State* find_goto  (State*, Symbol* sym);

char find_action(int state, Symbol* sym, int* next);

//int find_shift2 (int, Symbol* sym);
//int  find_reduce2(int, Symbol* sym);
//int  find_accept2(int, Symbol* sym);
int find_goto2  (int, Symbol* sym);

/*******************************************************************************
 * Example calculator program.  The program maintains a stack of parse states,
 * symbols and values while executing an input file.
 */
class Calculator {
  public:
    Calculator();
    
    void init();
    bool scan(Table* table, int c);
        
  private:
    //Node* node;
    int node2;
    std::string text;
    std::vector<State*>  states;
    std::vector<Symbol*> symbols;
    std::vector<Value*>  values;
    std::vector<int>  states2;
    
    bool advance(Table* table, Symbol* sym, Value* val);

    /** Utility methods for adding to the stack. */
    void push(State* state, int s, Symbol* sym, Value* val);
    void pop(size_t count);
};

#endif
