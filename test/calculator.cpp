#include "calculator.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
using std::unique_ptr;

/******************************************************************************/
int
main(int argc, const char * argv[])
{
    std::stringstream in("(5 + 5) * 5");
    
    Table table;
    
    Parser parser;
    parser.init();

    do {
        bool ok = parser.scan(&table, in.get());
        if (!ok) {
            return 1;
        }
    } while (!in.eof());
    
    return 0;
}

/******************************************************************************/
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
Parser::Parser() :
    node(nullptr){}

void
Parser::init()
{
    text.clear();
    node = node_start();

    states.clear();
    symbols.clear();
    values.clear();
    
    states.push_back(state_start());
    symbols.push_back(symbol_end());
    values.push_back(nullptr);
}

/******************************************************************************/
bool
Parser::scan(Table* table, int c)
{
    while (true)
    {
        if (c == EOF) {
            Accept* accept = find_term(node);
            if (accept) {
                Value* value = nullptr;
                if (accept->scan) {
                    value = accept->scan(table, text);
                }
                bool ok = advance(table, accept->term, value);
                if (!ok) {
                    return false;
                }
                ok = advance(table, symbol_end(), nullptr);
                if (!ok) {
                    return false;
                }
                return true;
            }
            else {
                std::cout << "Unexpected end of file.\n";
                return false;
            }
        }
        else if (node == node_start() && c == ' ') {
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
                Accept* accept = find_term(node);
                if (accept) {
                    Value* value = nullptr;
                    if (accept->scan) {
                        value = accept->scan(table, text);
                    }
                    bool ok = advance(table, accept->term, value);
                    if (!ok) {
                        return false;
                    }
                    node = node_start();
                    text.clear();
                }
                else {
                    std::cout << "Unexpected character." << (char)c << "\n";
                    return false;
                }
            }
        }
    }
}

/******************************************************************************/
bool
Parser::advance(Table* table, Symbol* sym, Value* val)
{
    while (true)
    {
        State* top = states.back();
        
        State* shift = find_shift(top, sym);
        if (shift) {
            push(shift, sym, val);
            return true;
        }
        
        Rule* accept = find_accept(top, sym);
        if (accept) {
            Value* result = nullptr;
            if (accept->reduce) {
                result = accept->reduce(table, values);
            }
            pop(accept->length);
            top = states.back();
            State* found = find_goto(top, accept->nonterm);
            push(found, accept->nonterm, result);
            return true;
        }

        Rule* rule = find_reduce(top, sym);
        if (rule) {
            Value* result = nullptr;
            if (rule->reduce) {
                result = rule->reduce(table, values);
            }
            pop(rule->length);
            top = states.back();
            State* found = find_goto(top, rule->nonterm);
            push(found, rule->nonterm, result);
        }
        else {
            std::cout << "Error, unexpected symbol ";
            std::cout << "'" << symbol_name(sym) << "'.\n";
            return false;
        }
    }
}

/******************************************************************************/
void
Parser::push(State* state, Symbol* sym, Value* val)
{
    states.push_back(state);
    symbols.push_back(sym);
    values.push_back(val);
}

void
Parser::pop(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        states.pop_back();
        symbols.pop_back();
        values.pop_back();
    }
}
