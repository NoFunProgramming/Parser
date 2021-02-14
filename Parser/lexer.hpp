/*******************************************************************************
 * Writes the source code for a lexer.  The lexer identifies tokens defined by
 * user provided regular expressions.  This class combines multiple regular
 * expressions into a single deterministic finite automaton (DFA).  After adding
 * all expressions, call solve and then write to generate the source code for
 * the lexer.
 *
 * The source code will define a structure for each state in DFA. This
 * structure contains a method that take a character and returns either a new
 * state in the DFA or a null pointer.  The null indicates that the pattern
 * matching is complete and the accept value, if any, for the current state is
 * the type of token identified.
 *
 *      Accept num("number", 0);
 *      Accept id("identifier", 1);
 *
 *      Lexer lexer;
 *      lexer.add(&num, "[0-9]+");
 *      lexer.add(&id, "[a-e]([a-e]|[0-9])*");
 *
 *      lexer.solve();
 *      lexer.write(std::cout);
 */

#ifndef lexer_hpp
#define lexer_hpp

#include "regex.hpp"

#include <set>
#include <map>
#include <vector>
#include <iostream>
using std::ostream;
using std::vector;
using std::unique_ptr;
using std::set;
using std::map;

/*******************************************************************************
 * Builds a lexer for identifying tokens in an input string.  The lexer
 * combines multiple regular expressions into a single deterministic finite
 * automaton (DFA).  This DFA can then be written to source code and later
 * compiled into another program to identify tokens in am input string.
 */
class Lexer
{
  public:
    Lexer();

    /**
     * Adds a user define regular expression pattern to the lexer.  The method
     * returns true if the provided expression is valid.
     */
    bool add(Accept* accept, const string& regex);
    bool add_series(Accept* accept, const string& series);
    
    /** After adding expressions, call solve to build to DFA. */
    void solve();

    /** After solving for the DFA, call write to generate the source code. */
    void write(ostream& out) const;
    
  private:
    vector<unique_ptr<Regex>> exprs;

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
        void add_finite(set<Finite*>& finites);
        
        /** Map a range of characters to the next DFA state. */
        void add_next(int first, int last, State* next);

        /** Solving for the next states in the DFA from this state. */
        void move(char c, set<Finite*>* next);
        void solve_closure();
        void solve_accept();
        
        /** Writes the source code for the lexer. */
        void write_proto(ostream& out);
        void write_struct(ostream& out);
        void write(ostream& out);

        /** Check if two states are really the same DFA state. */
        struct Compare {
            bool operator() (const unique_ptr<State>& left,
                             const unique_ptr<State>& right) const {
                return left->items < right->items;
            }
        };
        
        /** Character range for connecting states. */
        struct Range {
            Range(int first, int last);
            int first;
            int last;
            bool operator<(const Range& other) const;
            void write(ostream& out) const;
        };
        
      private:
        Accept* accept;
        set<Finite*> items;
        map<Range, State*> nexts;
    };
    
    /** The DFA is defined by an initial state and unique sets of NFA states. */
    State* initial;
    set<unique_ptr<State>, State::Compare> states;
    
    class Literal {
      public:
        static unique_ptr<Literal> parse(const string& in, Accept* accept);
        Literal();
        
        Finite* get_start();

      private:
        /** States of the NFA, owned by the literal object. */
        Finite* start;
        vector<unique_ptr<Finite>> states;
        Finite* add_state();
        Finite* add_state(Accept* accept);

        Finite* parse_term(istream& in, Accept* accept);
    };
    
    vector<unique_ptr<Literal>> literals;
};

#endif
