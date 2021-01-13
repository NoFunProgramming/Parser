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

class Lexer
{
  public:
    Lexer();

    bool add(Accept* accept, const string& regex);
    void solve();

    void write(ostream& out);
    
  private:
    vector<unique_ptr<Regex>> exprs;
    
    class State {
      public:
        State(size_t id);
        size_t id;
        
        void add_finite(Finite* finite);
        void add_finite(set<Finite*>& finites);
        void add_next(int first, int last, State* next);
        
        void move(char c, set<Finite*>* next);
        void solve_closure();
        void solve_accept();
        
        void write_proto(ostream& out);
        void write_struct(ostream& out);
        void write(ostream& out);

        struct Compare {
            bool operator() (const unique_ptr<State>& left,
                             const unique_ptr<State>& right) const {
                return left->items < right->items;
            }
        };
        
        struct Range {
            Range(int first, int last);
            int first;
            int last;
            bool operator<(const Range& other) const;
            void write(ostream& out) const;
        };
        
      private:
        set<Finite*> items;
        Accept* accept;
        
        map<Range, State*> nexts;
    };
    
    State* initial;
    set<unique_ptr<State>, State::Compare> states;
};

#endif
