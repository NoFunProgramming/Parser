#ifndef symbols_hpp
#define symbols_hpp

#include <set>
#include <vector>
#include <string>
#include <sstream>
using std::set;
using std::vector;
using std::string;
using std::ostream;
using std::unique_ptr;

/******************************************************************************/
class Symbol
{
  public:
    virtual void print(ostream& out) const = 0;
};

/******************************************************************************/
class Term : public Symbol
{
  public:
    Term(const string& name);
    
    virtual void print(ostream& out) const;
  
  private:
    string name;
};

/******************************************************************************/
class Nonterm : public Symbol
{
  public:
    Nonterm(const string& name);
        
    class Rule {
      public:
        void add(Symbol* sym);
        void print(ostream& out) const;
      private:
        vector<Symbol*> product;
    };
    
    Rule* add_rule();
    virtual void print(ostream& out) const;
    
  private:
    string name;
    vector<unique_ptr<Rule>> rules;
};

#endif
