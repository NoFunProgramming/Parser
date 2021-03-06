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
Lexer::add_regex(Term* accept, const std::string& regex)
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
Lexer::add_literal(Term* accept, const std::string& series)
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
    std::unique_ptr<Node> first = std::make_unique<Node>(nodes.size());
    for (auto& expr : exprs) {
        first->add_finite(expr->start);
    }
    for (auto& expr : literals) {
        first->add_finite(expr->start);
    }
    
    first->solve_closure();
    first->solve_accept();
    initial = first.get();
    nodes.insert(std::move(first));
    
    std::vector<Node*> pending;
    pending.push_back(initial);
    
    /** While still finding new states. */
    while (pending.size() > 0) {
        Node* current = pending.back();
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
                auto state = std::make_unique<Node>(nodes.size());
                state->add_finite(found);
                state->solve_closure();
                
                auto inserted = nodes.insert(std::move(state));
                Node* next = inserted.first->get();
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

    std::map<Node*, Node*> replacement;
    for (auto group : current) {
        Node* prime = group.represent(replacement, initial);
        primes.insert(prime);
    }
    for (Node* prime : primes) {
        prime->replace(replacement);
        prime->reduce();
    }
}

std::set<Lexer::Group>
Lexer::partition()
{
    //std::map<Term*, Group> split;
    std::map<Term*, Group> split;
    for (auto& state : nodes) {
        split[state->accept].insert(state.get());
    }

    std::set<Group> result;
    for (auto group : split) {
        result.insert(group.second);
    }
    return result;
}

/******************************************************************************/
void
Lexer::Group::insert(Node* state) {
    nodes.insert(state);
}

bool
Lexer::Group::belongs(Node* state, const std::set<Group>& all) const
{
    if (nodes.size() < 1) {
        return false;
    }

    Node* check = *nodes.begin();
    for (int c = 0; c <= CHAR_MAX; c++) {
        Node* check_next = check->get_next(c);
        Node* state_next = state->get_next(c);
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
Lexer::Group::same_group(Node* s1, Node* s2, const std::set<Group>& all)
{
    for (auto check : all) {
        if (check.nodes.count(s1) > 0) {
            if (check.nodes.count(s2) > 0) {
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

    for (auto state : nodes) {
        bool found = false;
        for (auto& group : result) {
            if (group.belongs(state, PI)) {
                group.nodes.insert(state);
                found = true;
                break;
            }
        }
        if (!found) {
            Group added;
            added.nodes.insert(state);
            result.push_back(added);
        }
    }
    return result;
}

Node*
Lexer::Group::represent(std::map<Node*, Node*>& replace, Node* start)
{
    Node* result = nullptr;
    for (auto s : nodes) {
        if (s == start) {
            result = s;
            break;
        }
    }
    if (!result) {
        auto lowest = min_element(nodes.begin(), nodes.end(), Node::lower);
        result = *lowest;
    }

    for (Node* state : nodes) {
        replace[state] = result;
    }
    return result;
}

bool
Lexer::Group::operator<(const Group& other) const {
    return nodes < other.nodes;
}

bool
Lexer::Group::operator==(const Group& other) const {
    return nodes == other.nodes;
}

