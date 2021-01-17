/*******************************************************************************
 * Finite automata for finding patterns in strings.  Outputs are added to each
 * finite state to define the next states to move to after reading an input
 * character.  After connecting the states, call scan from the start state to
 * read and return the accepted match from the input string.
 */

#ifndef finite_hpp
#define finite_hpp

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
using std::string;
using std::vector;
using std::set;
using std::unique_ptr;

/*******************************************************************************
 * Marks a state as matching a specific pattern.  The rank is required as
 * multiple final states are possible during reading and the state with the
 * lowest rank is selected as the match.
 */
class Accept {
  public:
    Accept(const string& name, size_t rank);
    string name;
    size_t rank;
};

/*******************************************************************************
 * State in the finite automata.  Each state contains an array of outputs that
 * determines the next states to move to after reading an input character.  The
 * presences of an accept object indicates a match when in this state.
 */
class Finite {
  public:
    Finite();
    Finite(Accept* accept);
    
    Accept* get_accept();

    /**
     * Checks an input stream for a pattern starting from this state.  Scan
     * continually reads from an input stream, following the outputs for each
     * character, until no new states are found.  At that point scan will return
     * of lowest ranked accept of the last found states.
     */
    Accept* scan(std::istream* in);
    
    /**
     * Each state contains an array of outputs that determine the next states
     * to move to after reading an input character.  Empty outputs, epsilon
     * transitions, are allowed and are useful for passing by optional states.
     */
    class Out {
      public:
        Out(char first, char last, Finite* next);
        Out(char first, char last, bool inside, Finite* next);
        Out(Finite* next);
        
        Finite* next;
        bool is_epsilon();
        bool in_range(char c);
        
      private:
        bool epsilon;
        bool inside;
        char first;
        char last;
    };
    
    /** Builds and returns new outputs, but retains ownership. */
    Out* add_out(char c, Finite* next);
    Out* add_out(char first, char last, Finite* next);
    Out* add_not(char first, char last, Finite* next);
    Out* add_epsilon(Finite* next);
        
    /** Finds output targets with the given character in its range. */
    void move(char c, set<Finite*>* next);
    
    /** Follows empty transitions until no new states are found. */
    static void closure(set<Finite*>* states);
    void closure(set<Finite*>* states, vector<Finite*>* stack);
    
    /** Compare states to find the lowest rank. */
    static bool lower(Finite* left, Finite* right);
    
  private:
    Accept* accept;
    vector<unique_ptr<Out>> outs;        
};

#endif
