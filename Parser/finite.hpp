/*******************************************************************************
 * Finite automata for finding patterns in strings.  Outputs are added to each
 * finite state to define the next states to move to after reading an input
 * character.  After connecting the states, call scan from the start state to
 * find and return the accepted match for the input string.
 */

#ifndef finite_hpp
#define finite_hpp

#include <set>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

/*******************************************************************************
 * Base class for all types of symbols such as terminals and nonterminals.
 */
class Symbol
{
  public:
    std::string type;
    virtual void print(std::ostream& out) const = 0;
    virtual void write(std::ostream& out) const = 0;
};

/*******************************************************************************
 * Terminals are the smallest unit of the grammar and often represent a specific
 * pattern of characters such as a integer.
 */
class Term : public Symbol
{
  public:
    Term(const std::string& name, size_t rank);
    std::string name;
    size_t rank;
    
    std::string action;
    
    virtual void print(std::ostream& out) const;
    virtual void write(std::ostream& out) const;
};

/*******************************************************************************
 * Endmark is a special terminal and indicates the end of an input string.
 */
class Endmark : public Symbol
{
  public:
    virtual void print(std::ostream& out) const;
    virtual void write(std::ostream& out) const;
};

/*******************************************************************************
 * Marks a state as matching a specific pattern.  The rank is required as
 * multiple final states are possible during reading and the state with the
 * lowest rank is selected as the match.
 */
class Accept {
  public:
    Accept(const std::string& name, size_t rank);
    std::string name;
    std::string scan;
    size_t rank;
};

/*******************************************************************************
 * State in the finite automata.  Each state contains an array of outputs that
 * determines the next states to move to after reading an input character.  The
 * presences of an accept pointer indicates a match when in this state.
 */
class Finite {
  public:
    Finite();
    Finite(Term* accept);
    Term* accept;

    /** Checks an input stream for a pattern starting from this state. */
    Term* scan(std::istream* in);
    
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
    void move(char c, std::set<Finite*>* next) const;
    
    /** Follows empty transitions until no new states are found. */
    static void closure(std::set<Finite*>* states);
    void closure(std::set<Finite*>* states, std::vector<Finite*>* stack) const;
    
    static bool lower_rank(const Finite* left, const Finite* right);
    
  private:
    std::vector<std::unique_ptr<Out>> outs;
};

#endif
