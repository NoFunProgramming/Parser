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
reduce_mul_num(Table* table, unique_ptr<Expr>& E1, unique_ptr<Expr>& E2)
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

/******************************************************************************/
Calculator::Calculator() :
    node(nullptr){}

void
Calculator::init()
{
    text.clear();
    node = Start_Node;

    states.clear();
    symbols.clear();
    values.clear();
    
    // TODO Push the start symbol.
    states.push_back(Start_State);
    states2.push_back(0);
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
            Accept* accept = is_accept(node);
            if (!accept && node != Start_Node) {
                std::cout << "Unexpected end of file.\n";
                return false;
            }
            
            Value* value = nullptr;
            if (accept) {
                if (accept->scan) {
                    value = accept->scan(table, text);
                }
                if (!advance(table, accept->term, value)) {
                    return false;
                }
            }
            if (!advance(table, Endmark, nullptr)) {
                return false;
            }
            return true;
        }
        else if (node == Start_Node && isspace(c)) {
            return true;
        }
        else {
            Node* next = next_node(node, c);
            if (next) {
                text.push_back(c);
                node = next;
                return true;
            }
            else {
                Accept* accept = is_accept(node);
                if (!accept) {
                    std::cout << "Unexpected character." << (char)c << "\n";
                    return false;
                }
                
                Value* value = nullptr;
                if (accept->scan) {
                    value = accept->scan(table, text);
                }
                if (!advance(table, accept->term, value)) {
                    return false;
                }
                node = Start_Node;
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
        State* top = states.back();
        int top2 = states2.back();
                
        State* shift = find_shift(top, sym);
        int shift2 = find_shift2(top2, sym);
        //assert(shift2 != -1);
        if (shift) {
            push(shift, shift2, sym, val);
            return true;
        }
        
        Rule* accept = find_accept(top, sym);
        int accept2 = find_accept2(top2, sym);
        //assert(accept2 != -1);
        if (accept) {
            Value* result = nullptr;
            if (accept->reduce && rs[accept2].reduce) {
                //result = accept->reduce(table, values);
                result = rs[accept2].reduce(table, values);
            }
            pop(accept->length);
            top = states.back();
            top2 = states2.back();
            State* found = find_goto(top, accept->nonterm);
            int found2 = find_goto2(top2, accept->nonterm);
            push(found, found2, accept->nonterm, result);
            return true;
        }

        Rule* rule = find_reduce(top, sym);
        int rule2 = find_reduce2(top2, sym);
        //assert(rule2 != -1);
        if (rule) {
            Value* result = nullptr;
            if (rule->reduce && rs[rule2].reduce) {
                //result = rule->reduce(table, values);
                result = rs[rule2].reduce(table, values);
            }
            pop(rule->length);
            top = states.back();
            top2 = states2.back();
            State* found = find_goto(top, rule->nonterm);
            int found2 = find_goto2(top2, rule->nonterm);
            push(found, found2, rule->nonterm, result);
        }
        else {
            std::cout << "Error, unexpected symbol ";
            std::cout << "'" << sym->name << "'.\n";
            return false;
        }
    }
}

/******************************************************************************/
void
Calculator::push(State* state, int s, Symbol* sym, Value* val)
{
    states.push_back(state);
    states2.push_back(s);
    symbols.push_back(sym);
    values.push_back(val);
}

void
Calculator::pop(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        states.pop_back();
        states2.pop_back();
        symbols.pop_back();
        values.pop_back();
    }
}
