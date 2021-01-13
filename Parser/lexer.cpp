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
    unique_ptr<State> first = make_unique<State>(states.size());
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
                auto state = std::make_unique<State>(states.size());
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
    out << "class State {\n";
    out << "    State* (*next)(int c);\n";
    out << "    const char* accept;\n";
    out << "};\n\n";
    for (auto& state : states) {
        state->write_proto(out);
    }
    out << "\n";
    for (auto& state : states) {
        state->write_struct(out);
    }
    out << "\n";
    for (auto& state : states) {
        state->write(out);
    }
}

Lexer::State::State(size_t id):
    id(id),
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

void
Lexer::State::write_proto(ostream& out) {
    out << "State* next" << id << "(int c);\n";
}

void
Lexer::State::write_struct(ostream& out) {
    out << "State state" << id;
    out << "(&next" << id;
    if (accept) {
        out << ", \"" << accept->name << "\"";
    }
    out << ");\n";
}

void
Lexer::State::write(ostream& out)
{
    out << "State*\n";
    out << "next" << id << "(int c) {\n";
    for (auto next : nexts) {
        out << "    if (";
        next.first.write(out);
        out << ") {\n";
        out << "        return state" << next.second->id << ";\n";
        out << "    }\n";
    }
    out << "    return nullptr;\n";
    out << "}\n\n";
}

Lexer::State::Range::Range(int first, int last):
    first(first), last(last) {}

bool
Lexer::State::Range::operator<(const Range& other) const {
    return last < other.first;
}

void
Lexer::State::Range::write(ostream& out) const {
    out << "(c >= '" << (char)first << "')";
    out << " && ";
    out << "(c <= '" << (char)last << "')";
}
