#include "calculator.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
using std::unique_ptr;

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        std::cerr << "Expected a single input argument.\n";
        return 1;
    }
        
    std::stringstream in(argv[1]);
        
    Table table;
    
    Calculator parser;
    parser.init();

    do {
        bool ok = parser.scan(&table, in.get());
        if (!ok) {
            return 1;
        }
    } while (!in.eof());
    
    return 0;
}

/*******************************************************************************
 * Implementation of the actions specified for the grammar rules.
 */
Expr::Expr(int value):
    value(value){}

unique_ptr<Expr>
scan_num(Table* table, const std::string& text)
{
    return std::make_unique<Expr>(std::stoi(text));
}

unique_ptr<Expr>
scan_hex(Table* table, const std::string& text)
{
    std::stringstream stream;
    stream << std::hex << text;
    int num = 0;
    stream >> num;
    return std::make_unique<Expr>(num);
}

unique_ptr<Expr>
reduce_total(Table* table, unique_ptr<Expr>& E1)
{
    unique_ptr<Expr> result = std::move(E1);
    std::cout << result->value << "\n";
    return result;
}

unique_ptr<Expr>
reduce_add(Table* table, unique_ptr<Expr>& E1)
{
    return std::move(E1);
}

unique_ptr<Expr>
reduce_add_mul(Table* table, unique_ptr<Expr>& E1, unique_ptr<Expr>& E2)
{
    unique_ptr<Expr> result = std::move(E1);
    result->value += E2->value;
    return result;
}

unique_ptr<Expr>
reduce_mul(Table* table, unique_ptr<Expr>& E1)
{
    return std::move(E1);
}

unique_ptr<Expr>
reduce_mul_int(Table* table, unique_ptr<Expr>& E1, unique_ptr<Expr>& E2)
{
    unique_ptr<Expr> result = std::move(E1);
    result->value *= E2->value;
    return result;
}

unique_ptr<Expr>
reduce_paren(Table* table, unique_ptr<Expr>& E1)
{
    return std::move(E1);
}

unique_ptr<Expr>
reduce_num(Table* table, unique_ptr<Expr>& E1)
{
    return std::move(E1);
}

unique_ptr<Expr>
reduce_hex(Table* table, unique_ptr<Expr>& E1)
{
    return std::move(E1);
}


/******************************************************************************/
Calculator::Calculator(){}

void
Calculator::init()
{
    text.clear();
    node = 0;

    states.clear();
    symbols.clear();
    values.clear();
    
    // TODO Push the start symbol.
    states.push_back(0);
    symbols.push_back(Endmark);
    values.push_back(nullptr);
}

/*******************************************************************************
 */
bool
Calculator::scan(Table* table, int c)
{
    while (true)
    {
        if (c == EOF) {
            if (!nodes[node].accept && node != 0) {
                std::cout << "Unexpected end of file.\n";
                return false;
            }
            
            Value* value = nullptr;
            if (nodes[node].accept) {
                if (nodes[node].scan) {
                    value = nodes[node].scan(table, text);
                }
                if (!advance(table, nodes[node].accept, value)) {
                    return false;
                }
            }
            if (!advance(table, Endmark, nullptr)) {
                return false;
            }
            return true;
        }
        else if (node == 0 && isspace(c)) {
            return true;
        }
        else {
            int next = -1;
            if (nodes[node].next) {
                next = nodes[node].next(c);
            };
            if (next != -1) {
                text.push_back(c);
                node = next;
                return true;
            }
            else {
                if (nodes[node].accept == NULL) {
                    std::cout << "Unexpected character." << (char)c << "\n";
                    return false;
                }
                
                Value* value = nullptr;
                if (nodes[node].scan) {
                    value = nodes[node].scan(table, text);
                }
                if (!advance(table, nodes[node].accept, value)) {
                    return false;
                }
                node = 0;
                text.clear();
            }
        }
    }
}

/******************************************************************************/
bool
Calculator::advance(Table* table, Symbol* sym, Value* val)
{
    while (true)
    {
        int top = states.back();
                        
        int next = 0;
        char type = find_action(top, sym, &next);
        
        switch (type) {
            case 'S': {
                push(next, sym, val);
                return true;
            }
            case 'A': {
                Value* result = nullptr;
                if (rules[next].reduce) {
                    result = rules[next].reduce(table, values);
                }
                pop(rules[next].length);
                
                top = states.back();
                int found = find_goto(top, rules[next].nonterm);
                
                push(found, rules[next].nonterm, result);
                return true;
            }
            case 'R': {
                Value* result = nullptr;
                if (rules[next].reduce) {
                    result = rules[next].reduce(table, values);
                }
                pop(rules[next].length);
                
                top = states.back();
                int found = find_goto(top, rules[next].nonterm);
                
                push(found, rules[next].nonterm, result);
                break;
            }
            default: {
                std::cout << "Error, unexpected symbol ";
                std::cout << "'" << sym->name << "'.\n";
                return false;
            }
        }
    }
}

/******************************************************************************/
void
Calculator::push(int s, Symbol* sym, Value* val)
{
    states.push_back(s);
    symbols.push_back(sym);
    values.push_back(val);
}

void
Calculator::pop(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        states.pop_back();
        symbols.pop_back();
        values.pop_back();
    }
}

char
find_action(int state, Symbol* sym, int* next) {
    for (Act* s = states[state].act; s->sym; s++) {
        if (s->sym == sym) {
            *next = s->next;
            return s->type;
        }
    }
    return -1;
}

int
find_goto(int state, Symbol* sym) {
   if (!states[state].go)
        return -1;
   for (Go* g = states[state].go; g->sym; g++) {
       if (g->sym == sym) {
           return g->state;
       }
   }
   return -1;
}
