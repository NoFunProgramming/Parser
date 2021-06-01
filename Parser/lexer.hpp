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

#include <vector>
#include <set>
#include <map>
#include <iostream>

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
    
    void reduce();

    /** After solving for the DFA, call write to generate the source code. */
    void write(std::ostream& out) const;
    
  private:
    std::vector<std::unique_ptr<Regex>> exprs;
    std::vector<std::unique_ptr<Literal>> literals;

  public:
    /**
     * State of the deterministic finite automaton.  The DFA is built by finding
     * new states that are the possible sets of finite states of a NFA while
     * reading input characters.  Ranges within each state map an input
     * character to a single next state in the DFA.
     */
    class State {
      public:
        State(size_t id);
        size_t id;

        /** Adding finite states of the NFA to a single DFA state. */
        void add_finite(Finite* finite);
        void add_finite(std::set<Finite*>& finites);
        
        /** Map a range of characters to the next DFA state. */
        void add_next(int first, int last, State* next);
        State* get_next(int c);

        /** Solving for the next states in the DFA from this state. */
        void move(char c, std::set<Finite*>* next);
        void solve_closure();
        void solve_accept();
        
        void replace(std::map<State*, State*> prime);
        void reduce();
        
        /** Writes the source code for the lexer. */
        void write_proto(std::ostream& out);
        void write_struct(std::ostream& out);
        void write(std::ostream& out);

        struct is_same {
            bool operator() (const std::unique_ptr<State>& left,
                             const std::unique_ptr<State>& right) const {
                return left->items < right->items;
            }
        };
        
        static bool lower(State* left, State* right);
        
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
        std::map<Range, State*> nexts;
    };
    
    /** The DFA is defined by an initial state and unique sets of NFA states. */
    std::set<std::unique_ptr<State>, State::is_same> states;
    State* initial;
    
    /** Groups of states are for minimizing the number of DFA states. */
    class Group {
      public:
        void insert(State* state);

        bool belongs(State* state, const std::set<Group>& all) const;
        std::vector<Group> divide(const std::set<Group>& PI) const;
        
        /** Find a state that represents all states in the group. */
        State* represent(std::map<State*, State*>& replace, State* start);

        bool operator<(const Group& other) const;
        bool operator==(const Group& other) const;

      private:
        std::set<State*> states;
        static bool same_group(State* s1, State* s2, const std::set<Group>& all);
    };
    
    std::set<State*> primes;

    /** Initial partion of the states. */
    std::set<Group> partition();
};

#endif
