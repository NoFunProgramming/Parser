/*******************************************************************************
 * Writes the source code for a lexer which identifies tokens defined by user
 * provided regular expressions. After adding all expressions, call solve and
 * then write to generate the source code for the lexer.
 *
 * The source code defines a structure for each scan state. The code provides
 * a function for getting the next state based on the input character.  If the
 * function returns a null pointer the pattern matching is complete.  Check the
 * last known state for an accept object to find the matched pattern.  If the
 * last state is not accepting, then an unexpected character was found in the
 * input.
 */

#ifndef lexer_hpp
#define lexer_hpp

#include "literal.hpp"
#include "regex.hpp"
#include "node.hpp"

/*******************************************************************************
 * Builds a lexer for identifying tokens in an input string.  The lexer combines
 * multiple regular expressions into a single deterministic finite automaton
 * (DFA).  This DFA can then be written to source code and later compiled into
 * another program to identify tokens in an input string.
 */
class Lexer
{
  public:
    Lexer();

    /** Add patterns to match in the input string. */
    bool add_regex(Accept* accept, const std::string& regex);
    bool add_literal(Accept* accept, const std::string& series);
    
    /** After adding all expressions, call solve to build to DFA. */
    void solve();
    
    /** After building the DFA, call reduce to minimize the states. */
    void reduce();
    
  private:
    std::vector<std::unique_ptr<Regex>> exprs;
    std::vector<std::unique_ptr<Literal>> literals;

  public:    
    /** The DFA is defined by an initial state and unique sets of NFA states. */
    std::set<std::unique_ptr<Node>, Node::is_same> states;
    Node* initial;
    
    /** Groups of states are for minimizing the number of DFA states. */
    class Group {
      public:
        void insert(Node* state);

        bool belongs(Node* state, const std::set<Group>& all) const;
        std::vector<Group> divide(const std::set<Group>& PI) const;
        
        /** Find a state that represents all states in the group. */
        Node* represent(std::map<Node*, Node*>& replace, Node* start);

        bool operator<(const Group& other) const;
        bool operator==(const Group& other) const;

      private:
        std::set<Node*> states;
        static bool same_group(Node* s1, Node* s2, const std::set<Group>& all);
    };
    
    std::set<Node*> primes;

    /** Initial partion of the states. */
    std::set<Group> partition();
};

#endif
