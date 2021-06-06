#ifndef node_hpp
#define node_hpp

#include "finite.hpp"

#include <vector>
#include <set>
#include <map>
#include <iostream>

/**
 * State of the deterministic finite automaton.  The DFA is built by finding
 * new states that are the possible sets of finite states of a NFA while
 * reading input characters.  Ranges within each state map an input
 * character to a single next state in the DFA.
 */
class Node {
  public:
    Node(size_t id);
    size_t id;

    /** Adding finite states of the NFA to a single DFA state. */
    void add_finite(Finite* finite);
    void add_finite(std::set<Finite*>& finites);
    
    /** Map a range of characters to the next DFA state. */
    void add_next(int first, int last, Node* next);
    Node* get_next(int c);

    /** Solving for the next states in the DFA from this state. */
    void move(char c, std::set<Finite*>* next);
    void solve_closure();
    void solve_accept();
    
    void replace(std::map<Node*, Node*> prime);
    void reduce();
    
    struct is_same {
        bool operator() (const std::unique_ptr<Node>& left,
                         const std::unique_ptr<Node>& right) const {
            return left->items < right->items;
        }
    };
    
    static bool lower(Node* left, Node* right);
    
    /** Character range for connecting states. */
    struct Range {
        Range(int first, int last);
        int first;
        int last;
        bool operator<(const Range& other) const;
        void write(std::ostream& out) const;
    };
    
    Accept* accept;
    std::set<Finite*> items;
    std::map<Range, Node*> nexts;
};

#endif
