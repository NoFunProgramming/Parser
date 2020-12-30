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

/******************************************************************************/
class Accept {
  public:
    Accept(const string& name, size_t rank);
    string name;
    size_t rank;
};

/******************************************************************************/
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
