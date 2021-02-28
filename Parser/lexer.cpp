#include "lexer.hpp"

#include <sstream>
using std::make_unique;
using std::istringstream;
using std::cerr;

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

bool
Lexer::add_series(Accept* accept, const string& series)
{
    unique_ptr<Literal> expr = Literal::parse(series, accept);
    if (!expr) {
        std::cerr << "Unable to parse character series.\n";
        return false;
    }
    
    literals.push_back(std::move(expr));
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
    for (auto& expr : literals) {
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
Lexer::write(ostream& out) const
{
    for (auto& state : states) {
        out << "extern Node node" << state->id << ";\n";
    }
    out << "\n";
//    for (auto& state : states) {
//        state->write_proto(out);
//    }
//    out << "\n";
    for (auto& state : states) {
        state->write(out);
    }
    for (auto& state : states) {
        state->write_struct(out);
    }
    out << "\n";
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
    if (items.size() > 1) {
        std::cerr << "Checking\n";
    }
    auto lowest = min_element(items.begin(), items.end(), Finite::lower);
    if (lowest != items.end()) {
        accept = (*lowest)->get_accept();
        if (items.size() > 1 && accept) {
            std::cerr << "Return " << id << " " << accept->name << "\n";
        }
    }
}

/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
void
Lexer::State::write_proto(ostream& out) {
    out << "Node* scan_next" << id << "(int c);\n";
}

void
Lexer::State::write_struct(ostream& out) {
    out << "Node node" << id;
    out << " = {&scan" << id;
    if (accept) {
        //out << ", \"" << accept->name << "\"";
        out << ", &term" << accept->rank;
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}

void
Lexer::State::write(ostream& out)
{
    out << "Node*\n";
    out << "scan" << id << "(int c) {\n";
    for (auto next : nexts) {
        out << "    if (";
        next.first.write(out);
        out << ") {\n";
        out << "        return &node" << next.second->id << ";\n";
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
    if (first == last) {
        if (isprint(first) && first != '\'') {
            out << "c == '" << (char)first << "'";
        } else {
            out << "c == " << first << "";
        }
    } else {
        if (isprint(first) && isprint(last)
                && first != '\'' && last != '\'') {
            out << "(c >= '" << (char)first << "')";
            out << " && ";
            out << "(c <= '" << (char)last << "')";
        } else {
            out << "(c >= " << first << ")";
            out << " && ";
            out << "(c <= " << last << ")";
        }
    }
}

/******************************************************************************/
Lexer::Literal::Literal():
    start(nullptr) {}

Finite* Lexer::Literal::get_start() { return start; }

unique_ptr<Lexer::Literal>
Lexer::Literal::parse(const string& in, Accept* accept)
{
    unique_ptr<Literal> result(make_unique<Literal>());
    
    istringstream input(in);
    
    result->start = result->parse_term(input, accept);
    
    if (!result->start) {
        std::cerr << "Unable to parse expression '" << in << "'.\n";
        result.reset();
    }

    return result;
}

/**
 * Builds a new state and retains ownership.  No memory leaks occur if any
 * exceptions or errors occur during subset construction, as the states vector
 * contains all fragments of the automaton.
 */
Finite*
Lexer::Literal::add_state() {
    states.emplace_back(make_unique<Finite>());
    return states.back().get();
}

Finite*
Lexer::Literal::add_state(Accept* accept) {
    states.emplace_back(make_unique<Finite>(accept));
    return states.back().get();
}


Finite*
Lexer::Literal::parse_term(istream& in, Accept* accept)
{
    Finite* fact = add_state();
    Finite* term = fact;
    
    while (true)
    {
        int c = in.get();
        if (!isprint(c)) {
            cerr << "Expected a printable character.\n";
            return nullptr;
        }
        
        if (c == '\\') {
            c = in.get();
            switch (c) {
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'a': c = '\a'; break;
                case 'b': c = '\b'; break;
                case 'e': c = '\e'; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                case '\'': c = '\''; break;
                case '"':  c = '"' ; break;
                case '?':  c = '?' ; break;
                default: {
                    cerr << "Unknown escape sequence.\n";
                    return nullptr;
                }
            }
        }
        
        if (in.peek() == EOF) {
            Finite* next = add_state(accept);
            term->add_out(c, next);
            break;
        }
        else {
            Finite* next = add_state();
            term->add_out(c, next);
            term = next;
        }
    }
    
    return fact;
}
