#include "finite.hpp"

using std::make_unique;

/******************************************************************************/
Accept::Accept(const string& name, size_t rank):
    name(name),
    rank(rank){}

/******************************************************************************/
Finite::Finite():
    accept(nullptr){}

Finite::Finite(Accept* accept):
    accept(accept){}

Accept* Finite::get_accept() { return accept; }

Accept*
Finite::scan(std::istream* in)
{
    set<Finite*> current;
    set<Finite*> found;
    
    current.insert(this);
    closure(&current);
    
    while (in->peek() != EOF) {
        char c = in->peek();
        for (Finite* state : current) {
            state->move(c, &found);
        }
        closure(&found);
        
        if (found.size() > 0) {
            in->get();
            current = found;
            found.clear();
        } else {
            break;
        }
    }
    
    auto lowest = min_element(current.begin(), current.end(), lower);
    if (lowest != current.end()) {
        return (*lowest)->accept;
    } else {
        return nullptr;
    }
}

void
Finite::closure(set<Finite*>* states)
{
    vector<Finite*> stack;
    stack.insert(stack.end(), states->begin(), states->end());
    
    while (stack.size() > 0) {
        Finite* check = stack.back();
        stack.pop_back();
        check->closure(states, &stack);
    }
}

void
Finite::closure(set<Finite*>* states, vector<Finite*>* stack)
{
    for (unique_ptr<Out>& out: outs) {
        if (out->is_epsilon()) {
            Finite* next = out->next;
            if (next) {
                auto found = states->insert(next);
                if (found.second) {
                    stack->push_back(next);
                }
            }
        }
    }
}

void
Finite::move(char c, set<Finite*>* next)
{
    for (unique_ptr<Out>& out : outs) {
        if (out->in_range(c) && out->next) {
            next->insert(out->next);
        }
    }
}

bool
Finite::lower(Finite* left, Finite* right)
{
    if (left->accept && right->accept) {
        return left->accept->rank < right->accept->rank;
    } else if (left->accept) {
        return true;
    } else {
        return false;
    }
}

Finite::Out*
Finite::add_out(char c, Finite* next) {
    outs.emplace_back(make_unique<Out>(c, c, next));
    return outs.back().get();
}

Finite::Out*
Finite::add_out(char first, char last, Finite* next) {
    outs.emplace_back(make_unique<Out>(first, last, next));
    return outs.back().get();
}

Finite::Out*
Finite::add_not(char first, char last, Finite* next) {
    outs.emplace_back(make_unique<Out>(first, last, false, next));
    return outs.back().get();
}

Finite::Out*
Finite::add_epsilon(Finite* next) {
    outs.emplace_back(make_unique<Out>(next));
    return outs.back().get();
}

/******************************************************************************/
/** Constructs a new output. */
Finite::Out::Out(char first, char last, Finite* next):
    next    (next),
    epsilon (false),
    inside  (true),
    first   (first),
    last    (last){}

Finite::Out::Out(char first, char last, bool inside, Finite* next):
    next    (next),
    epsilon (false),
    inside  (inside),
    first   (first),
    last    (last){}

Finite::Out::Out(Finite* next):
    next    (next),
    epsilon (true),
    inside  (true),
    first   ('\0'),
    last    ('\0'){}

bool
Finite::Out::is_epsilon() {
    return epsilon;
}

bool
Finite::Out::in_range(char c) {
    if (epsilon) {
        return false;
    } else if (inside) {
        return (c >= first && c <= last);
    } else {
        return (c < first || c > last);
    }
}
