#include "lexer.hpp"

#include <sstream>
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
Lexer::add_regex(Accept* accept, const std::string& regex)
{
    std::unique_ptr<Regex> expr = Regex::parse(regex, accept);
    if (!expr) {
        std::cerr << "Unable to parse expression.\n";
        return false;
    }
    
    exprs.push_back(std::move(expr));
    return true;
}

bool
Lexer::add_literal(Accept* accept, const std::string& series)
{
    std::unique_ptr<Literal> expr = Literal::build(series, accept);
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
    std::unique_ptr<State> first = std::make_unique<State>(states.size());
    for (auto& expr : exprs) {
        first->add_finite(expr->start);
    }
    for (auto& expr : literals) {
        first->add_finite(expr->start);
    }
    
    first->solve_closure();
    first->solve_accept();
    initial = first.get();
    states.insert(std::move(first));
    
    std::vector<State*> pending;
    pending.push_back(initial);
    
    /** While still finding new states. */
    while (pending.size() > 0) {
        State* current = pending.back();
        pending.pop_back();
        
        /** Check every character for a possible new set. */
        int c = 0;
        while (c <= CHAR_MAX) {
            std::set<Finite*> found;
            current->move(c, &found);
            
            int first = c;
            int last = c++;
            bool matches = true;
            
            /** Keep looking to check if the next char is the same set. */
            while (c <= CHAR_MAX && matches) {
                std::set<Finite*> next;
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

void
Lexer::reduce()
{
    std::set<Group> current = partition();
    std::set<Group> found = current;

    while (true) {
        for (auto group : current) {
            std::vector<Group> divided = group.divide(current);
            found.erase(group);
            copy(divided.begin(), divided.end(), inserter(found, found.end()));
        }
        if (current == found) {
            break;
        } else {
            current = found;
        }
    }

    std::map<State*, State*> replacement;
    for (auto group : current) {
        State* prime = group.represent(replacement, initial);
        primes.insert(prime);
    }
    for (State* prime : primes) {
        prime->replace(replacement);
        prime->reduce();
    }
}

std::set<Lexer::Group>
Lexer::partition()
{
    //std::map<Term*, Group> split;
    std::map<Accept*, Group> split;
    for (auto& state : states) {
        split[state->accept].insert(state.get());
    }

    std::set<Group> result;
    for (auto group : split) {
        result.insert(group.second);
    }
    return result;
}


/**
 * Writes the source code for a lexer.  The source code will define a structure
 * for each state in DFA. This structure contains a method that take a character
 * and returns either a new state in the DFA or a null pointer.  The null
 * indicates that the pattern matching is complete and the accept value, if any,
 * for the current state is the type of token identified.
 */
void
Lexer::write(std::ostream& out) const
{
    for (auto& state : states) {
        out << "extern Node node" << state->id << ";\n";
    }
    out << std::endl;
    for (auto& state : states) {
        state->write(out);
    }
    for (auto& state : states) {
        state->write_struct(out);
    }
    out << std::endl;
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
Lexer::State::add_finite(std::set<Finite*>& finites) {
    items.insert(finites.begin(), finites.end());
}

void
Lexer::State::add_next(int first, int last, State* next) {
    nexts[Range(first, last)] = next;
}

Lexer::State*
Lexer::State::get_next(int c)
{
    for (auto next : nexts) {
        if (c >= next.first.first && c <= next.first.last) {
            return next.second;
        }
    }
    return nullptr;
}

void
Lexer::State::move(char c, std::set<Finite*>* found) {
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
    std::vector<Finite*> stack;
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
    auto lowest = min_element(items.begin(), items.end(), Finite::lower_rank);
    if (lowest != items.end()) {
        accept = (*lowest)->accept;
    }
}

void
Lexer::State::replace(std::map<State*, State*> prime)
{
    for (auto& next : nexts) {
        if (prime.count(next.second) > 0) {
            next.second = prime[next.second];
        }
    }
}

void
Lexer::State::reduce()
{
    std::map<State::Range, State*> updated;

    auto itr = nexts.begin();
    while (itr != nexts.end()) {
        int first = itr->first.first;
        int last  = itr->first.last;
        State* next = itr->second;
        itr++;

        bool matches = true;
        while (itr != nexts.end() && matches) {
            matches = itr->second == next && (last + 1) == itr->first.first;
            if (matches) {
                last = itr->first.last;
                itr++;
            }
        }
        updated[State::Range(first, last)] = next;
    }
    nexts = updated;
}

bool
Lexer::State::lower(State* left, State* right)
{
    if (left->accept && right->accept) {
        return left->accept->rank < right->accept->rank;
    } else if (left->accept) {
        return true;
    } else {
        return false;
    }
}

/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
void
Lexer::State::write_proto(std::ostream& out) {
    out << "Node* scan_next" << id << "(int c);\n";
}

void
Lexer::State::write_struct(std::ostream& out) {
    out << "Node node" << id;
    out << " = {&scan" << id;
    if (accept) {
        out << ", &term" << accept->rank << "_accept";
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}

void
Lexer::State::write(std::ostream& out)
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
Lexer::State::Range::write(std::ostream& out) const
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
void
Lexer::Group::insert(State* state) {
    states.insert(state);
}

bool
Lexer::Group::belongs(State* state, const std::set<Group>& all) const
{
    if (states.size() < 1) {
        return false;
    }

    State* check = *states.begin();
    for (int c = 0; c <= CHAR_MAX; c++) {
        State* check_next = check->get_next(c);
        State* state_next = state->get_next(c);
        if (check_next || state_next) {
            if (!check_next || !state_next) {
                return false;
            } else if (!same_group(check_next, state_next, all)) {
                return false;
            }
        }
    }
    return true;
}

bool
Lexer::Group::same_group(State* s1, State* s2, const std::set<Group>& all)
{
    for (auto check : all) {
        if (check.states.count(s1) > 0) {
            if (check.states.count(s2) > 0) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

std::vector<Lexer::Group>
Lexer::Group::divide(const std::set<Group>& PI) const
{
    std::vector<Group> result;

    for (auto state : states) {
        bool found = false;
        for (auto& group : result) {
            if (group.belongs(state, PI)) {
                group.states.insert(state);
                found = true;
                break;
            }
        }
        if (!found) {
            Group added;
            added.states.insert(state);
            result.push_back(added);
        }
    }
    return result;
}

Lexer::State*
Lexer::Group::represent(std::map<State*, State*>& replace, State* start)
{
    State* result = nullptr;
    for (auto s : states) {
        if (s == start) {
            result = s;
            break;
        }
    }
    if (!result) {
        auto lowest = min_element(states.begin(), states.end(), State::lower);
        result = *lowest;
    }

    for (State* state : states) {
        replace[state] = result;
    }
    return result;
}

bool
Lexer::Group::operator<(const Group& other) const {
    return states < other.states;
}

bool
Lexer::Group::operator==(const Group& other) const {
    return states == other.states;
}

