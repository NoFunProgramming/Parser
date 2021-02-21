#include "parser.hpp"

#include <iostream>

extern Node node0;
extern State state0;
extern Symbol endmark;

/******************************************************************************/
Parser::Parser() :
    node(&node0){}

void
Parser::init()
{
    stack.push_back(&state0);
    symbols.push_back(&endmark);
}

/******************************************************************************/
bool
Parser::scan(int c)
{
    if (c == EOF) {
        if (node->accept) {
            std::cout << "Found : " << node->accept << "\n";
            advance(node->accept);
            advance(&endmark);
        } else {
            std::cout << "Unexpected end of file." << "\n";
            return false;
        }
    }
    else if (c == ' ') {
        if (node->accept) {
            std::cout << "Found : " << node->accept << "\n";
            advance(node->accept);
            node = &node0;
            text.clear();
        } else if (node == &node0) {
            
        } else {
            std::cout << "Unexpected end of file." << "\n";
            return false;
        }
    }
    else {
        Node* next = node->scan(c);
        text.push_back(c);
        while (next == nullptr) {
            if (node->accept) {
                std::cout << "Found : " << node->accept << "\n";
                advance(node->accept);
                node = &node0;
                text.clear();
                next = node->scan(c);
                text.push_back(c);
            } else {
                std::cout << "Unexpected character" << "\n";
                return false;
            }
        }
        node = next;
    }
    return true;
}

/******************************************************************************/
bool
Parser::advance(Symbol* sym)
{
    while (true)
    {
        State* top = stack.back();
        
        State* next = top->find_shift(sym);
        if (next) {
            Value* result = nullptr;
            Term* term = dynamic_cast<Term*>(sym);
            if (term && term->scan) {
                result = term->scan(text);
            }
            push(next, sym, result);
            std::cout << "Shift\n";
            return true;
        }
        
        Rule* accept = top->find_accept(sym);
        if (accept) {
            Value* result = nullptr;
            if (accept->reduce) {
                result = accept->reduce(values);
            }
            pop(accept->length);
            top = stack.back();
            State* found = top->find_goto(accept->nonterm);
            push(found, accept->nonterm, result);
            std::cout << "Accept\n";
            return true;
        }

        Rule* rule = top->find_reduce(sym);
        if (rule) {
            Value* result = nullptr;
            if (rule->reduce) {
                result = rule->reduce(values);
            }
            pop(rule->length);
            top = stack.back();
            State* found = top->find_goto(rule->nonterm);
            push(found, rule->nonterm, result);
            std::cout << "Reduce\n";
            continue;
        }
        
        std::cout << "Error\n";
        return false;
    }
}

void
Parser::push(State* state, Symbol* sym, Value* val)
{
    stack.push_back(state);
    symbols.push_back(sym);
    values.push_back(val);
}

void
Parser::pop(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        stack.pop_back();
        symbols.pop_back();
        values.pop_back();
    }
}

/******************************************************************************/
State*
State::find_shift(Symbol* sym) {
    if (!shift)
        return nullptr;
    for (Shift* s = shift; s->sym; s++) {
        if (s->sym == sym) {
            return s->state;
        }
    }
    return nullptr;
}

Rule*
State::find_accept(Symbol* sym) {
    if (!accept)
        return nullptr;
    for (Accept* r = accept; r->sym; r++) {
        if (r->sym == sym) {
            return r->rule;
        }
    }
    return nullptr;
}

Rule*
State::find_reduce(Symbol* sym) {
    if (!reduce)
        return nullptr;
    for (Reduce* r = reduce; r->sym; r++) {
        if (r->sym == sym) {
            return r->rule;
        }
    }
    return nullptr;
}

State*
State::find_goto(Symbol* sym) {
    if (!next)
        return nullptr;
    for (Go* g = next; g->sym; g++) {
        if (g->sym == sym) {
            return g->state;
        }
    }
    return nullptr;
}
