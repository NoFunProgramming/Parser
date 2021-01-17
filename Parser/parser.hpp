#ifndef parser_hpp
#define parser_hpp

#include <sstream>
using std::istream;

/******************************************************************************/
class Parser
{
  public:
    Parser();
    
  private:
    bool read_grammar(istream& in);
    bool read_rules(istream& in);
    bool read_product(istream& in);

    bool read_term(istream& in);
    bool read_nonterm(istream& in);
};

#endif
