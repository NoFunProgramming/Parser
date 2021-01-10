#include "lexer.hpp"

using std::make_unique;

/******************************************************************************/
Lexer::Lexer() {}

bool
Lexer::add(Accept* accept, const string& regex)
{
    unique_ptr<Regex> expr = Regex::parse(regex, accept);
    if (!expr) {
        std::cerr << "Unable to parse expression.\n";
        return false;
    }
    
    exprs.push_back(std::move(expr));
    return true;
}

void
Lexer::solve()
{
    unique_ptr<State> first = make_unique<State>();
    for (auto& expr : exprs) {
        first->add_finite(expr->get_start());
    }
    first->solve_closure();
    first->solve_accept();
    initial = first.get();
    states.insert(std::move(first));
    
    vector<State*> pending;
    pending.push_back(initial);
    
    while (pending.size() > 0) {
        State* current = pending.back();
        pending.pop_back();
        
        int c = 0;
        while (c <= CHAR_MAX) {
            set<Finite*> found;
            current->move(c, &found);
            
            int first = c;
            int last = c++;
            bool matches = true;
            
            while (c <= CHAR_MAX && matches) {
                set<Finite*> next;
                current->move(c, &next);
                matches = found == next;
                if (matches) {
                    last = c++;
                }
            }
            
            if (found.size() > 0) {
                auto state = std::make_unique<State>();
                state->add_finite(found);
                state->solve_closure();
                
                auto inserted = states.insert(std::move(state));
                State* next = inserted.first->get();
                current->add_next(first, last, next);
                if (inserted.second) {
                    next->solve_accept();
                    pending.push_back(next);
                }
            }
        }
    }
}

void
Lexer::write(ostream& out)
{
    
}

Lexer::State::State():
    accept(nullptr) {}

void
Lexer::State::add_finite(Finite* finite) {
    items.insert(finite);
}

void
Lexer::State::add_finite(set<Finite*>& finites) {
    items.insert(finites.begin(), finites.end());
}

void
Lexer::State::add_next(int first, int last, State* next) {
    nexts[Range(first, last)] = next;
}

void
Lexer::State::move(char c, set<Finite*>* found)
{
    for (Finite* item : items) {
        item->move(c, found);
    }
}

void
Lexer::State::solve_closure()
{
    vector<Finite*> stack;
    stack.insert(stack.end(), items.begin(), items.end());
    
    while (stack.size() > 0) {
        Finite* check = stack.back();
        stack.pop_back();
        check->closure(&items, &stack);
    }
}

void
Lexer::State::solve_accept()
{
    auto lowest = min_element(items.begin(), items.end(), Finite::lower);
    if (lowest != items.end()) {
        accept = (*lowest)->get_accept();
    }
}

Lexer::State::Range::Range(int first, int last):
    first(first), last(last) {}

bool
Lexer::State::Range::operator<(const Range& other) const {
    return last < other.first;
}
