#include "lexer.hpp"

using std::make_unique;

/******************************************************************************/
Lexer::Lexer():
    initial(nullptr) {}

/**
 * Builds a NFA for a user defined regular expression.  The method returns true
 * if the provided expression is valid.  Solve then combines all of these NFAs
 * into a single DFA.
 */
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

/**
 * Converts the multiple non-deterministic finite automaton (NFA) defined by
 * regular expressions into a single deterministic finite automaton (DFA).
 * The lexer DFA is built by finding new states that are the possible sets of
 * finite states of a NFA while reading input characters.  Starting with the
 * initial set of finite states as the first DFA state, solve will follow
 * character ranges to new sets of states.  Each new found set of states will
 * define a new DFA state.  This searching will continue until no new sets of
 * states are found.
 */
void
Lexer::solve()
{
    /** Build the first state from the start state of all expressions. */
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
    
    /** While still finding new states. */
    while (pending.size() > 0) {
        State* current = pending.back();
        pending.pop_back();
        
        /** Check every character for a possible new set. */
        int c = 0;
        while (c <= CHAR_MAX) {
            set<Finite*> found;
            current->move(c, &found);
            
            int first = c;
            int last = c++;
            bool matches = true;
            
            /** Keep looking to check if the next char is the same set. */
            while (c <= CHAR_MAX && matches) {
                set<Finite*> next;
                current->move(c, &next);
                matches = found == next;
                if (matches) {
                    last = c++;
                }
            }
            
            /** After searching check to see if the state was already found. */
            if (found.size() > 0) {
                auto state = std::make_unique<State>(states.size());
                state->add_finite(found);
                state->solve_closure();
                
                auto inserted = states.insert(std::move(state));
                State* next = inserted.first->get();
                current->add_next(first, last, next);
                
                /** Check newly found state for other possible DFA states. */
                if (inserted.second) {
                    next->solve_accept();
                    pending.push_back(next);
                }
            }
        }
    }
}

/**
 * Writes the source code for a lexer.  The source code will define a structure
 * for each state in DFA. This structure contains a method that take a character
 * and returns either a new state in the DFA or a null pointer.  The null
 * indicates that the pattern matching is complete and the accept value, if any,
 * for the current state is the type of token identified.
 */
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

/******************************************************************************/
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
Lexer::State::move(char c, set<Finite*>* found) {
    for (Finite* item : items) {
        item->move(c, found);
    }
}

/**
 * After folliwng outputs that contain the input character, add the targets of
 * empty transitions to the newly found set of states.
 */
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

/**
 * Since the DFA states contain multiple finite states, determine the NFA state
 * with the lowest ranked accept to represent the pattern matched by the
 * current DFA state.
 */
void
Lexer::State::solve_accept()
{
    auto lowest = min_element(items.begin(), items.end(), Finite::lower);
    if (lowest != items.end()) {
        accept = (*lowest)->get_accept();
    }
}

/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
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

/******************************************************************************/
Lexer::State::Range::Range(int first, int last):
    first(first), last(last) {}

bool
Lexer::State::Range::operator<(const Range& other) const {
    return last < other.first;
}

void
Lexer::State::Range::write(ostream& out) const
{
    // TODO Check for printable characters.
    out << "(c >= '" << (char)first << "')";
    out << " && ";
    out << "(c <= '" << (char)last << "')";
}
