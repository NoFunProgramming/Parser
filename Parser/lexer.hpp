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

/*******************************************************************************
 * One application of finite states machines is the matching patterns within an
 * input string.  To match patterns, multiple states are connected by outputs to
 * each other.  Outputs from a state are associated with characters and
 * determine the next active states based on the next character read from the input
 * string.  To match a specific pattern, define the connections between states
 * and then scan the input from the start state, moving between states while
 * reading the input.
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
    bool add_regex(Term* accept, const std::string& regex);
    bool add_literal(Term* accept, const std::string& series);
    
    /** After adding all expressions, call solve to build to DFA. */
    void solve();
    
    /** After building the DFA, call reduce to minimize the states. */
    void reduce();
    
  private:
    std::vector<std::unique_ptr<Regex>> exprs;
    std::vector<std::unique_ptr<Literal>> literals;

  public:    
    /** The DFA is defined by an initial state and unique sets of NFA states. */
    std::set<std::unique_ptr<Node>, Node::is_same> nodes;
    std::set<Node*> primes;
    Node* initial;
 
  private:
    /** Groups of states for minimizing the number of DFA states. */
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
        std::set<Node*> nodes;
        static bool same_group(Node* s1, Node* s2, const std::set<Group>& all);
    };
    

    /** Initial partion of the states. */
    std::set<Group> partition();
};

#endif
