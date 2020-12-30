/**
 * State in the finite automata.  Each state contains an array of outputs that
 * determine the next states to move to after reading an input character.  After
 * connecting the states, call scan from the start state to read and return the
 * accepted match state from the stream.
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

/**
 * Marks a state as matching a specific pattern.  In addition to its name the
 * class has a rank.  The rank is required as multiple final states are possible
 * during reading and the state with the lowest rank is selected as the match.
 */
class Accept {
  public:
    Accept(const string& name, size_t rank);
    string name;
    size_t rank;
};

/**
 * State in the finite automata.  Each state contains an array of outputs that
 * determine the next states to move to after reading an input character.  Empty
 * outputs, epsilon transitions, are allowed and are useful for passing by
 * optional states.
 */
class Finite {
  public:
    Finite();
    Finite(Accept* accept);

    Accept* scan(std::istream* in);
    
    class Out {
      public:
        Out(char first, char last, Finite* next);
        Out(Finite* next);
        
        Finite* next;
        bool is_epsilon();
        bool in_range(char c);
        
      private:
        bool epsilon;
        char first;
        char last;
    };
    
    Out* add_out(char c, Finite* next);
    Out* add_out(char first, char last, Finite* next);
    Out* add_epsilon(Finite* next);
    
  private:
    Accept* accept;
    vector<unique_ptr<Out>> outs;
    
    static void closure(set<Finite*>* states);
    void closure(set<Finite*>* states, vector<Finite*>* stack);
    
    void move(char c, set<Finite*>* next);
    
    static bool lower(Finite* left, Finite* right);
};

#endif
