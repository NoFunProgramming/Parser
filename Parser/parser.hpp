#ifndef parser_hpp
#define parser_hpp

#include "symbols.hpp"

#include <map>
#include <string>
#include <sstream>
#include <memory>
using std::map;
using std::string;
using std::istream;
using std::unique_ptr;

/******************************************************************************/
class Parser
{
  public:
    Parser();
    
    bool read_grammar(istream& in);
    
    void print(ostream& out) const;
    
  private:
    bool read_rules(istream& in);
    bool read_product(istream& in, Nonterm::Rule* rule);

    Term* read_term(istream& in);
    Nonterm* read_nonterm(istream& in);
    
    map<string, unique_ptr<Term>> terms;
    map<string, unique_ptr<Nonterm>> nonterms;
};

#endif
